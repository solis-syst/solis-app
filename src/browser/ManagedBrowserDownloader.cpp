#include "browser/ManagedBrowserDownloader.h"

#ifdef _WIN32

#include <windows.h>
#include <commctrl.h>
#include <winhttp.h>

#include <fstream>
#include <string>
#include <vector>

namespace solis {

namespace {

std::wstring toWide(const std::string& value) {
    if (value.empty()) {
        return {};
    }

    const int size = MultiByteToWideChar(
        CP_UTF8,
        0,
        value.data(),
        static_cast<int>(value.size()),
        nullptr,
        0);

    if (size <= 0) {
        return {};
    }

    std::wstring result(size, L'\0');
    MultiByteToWideChar(
        CP_UTF8,
        0,
        value.data(),
        static_cast<int>(value.size()),
        result.data(),
        size);

    return result;
}

bool parseUrl(
    const std::string& url,
    std::wstring& host,
    std::wstring& path) {
    const auto wideUrl = toWide(url);

    if (wideUrl.empty()) {
        return false;
    }

    URL_COMPONENTS components{};
    components.dwStructSize = sizeof(components);
    components.dwSchemeLength = static_cast<DWORD>(-1);
    components.dwHostNameLength = static_cast<DWORD>(-1);
    components.dwUrlPathLength = static_cast<DWORD>(-1);
    components.dwExtraInfoLength = static_cast<DWORD>(-1);

    if (!WinHttpCrackUrl(
            wideUrl.c_str(),
            static_cast<DWORD>(wideUrl.size()),
            0,
            &components)) {
        return false;
    }

    if (components.nScheme != INTERNET_SCHEME_HTTPS) {
        return false;
    }

    host.assign(
        components.lpszHostName,
        components.dwHostNameLength);

    path.assign(
        components.lpszUrlPath,
        components.dwUrlPathLength);

    if (components.dwExtraInfoLength > 0) {
        path.append(
            components.lpszExtraInfo,
            components.dwExtraInfoLength);
    }

    return true;
}

class ProgressWindow {
public:
    bool create(const std::wstring& title) {
        INITCOMMONCONTROLSEX controls{};
        controls.dwSize = sizeof(controls);
        controls.dwICC = ICC_PROGRESS_CLASS;

        if (!InitCommonControlsEx(&controls)) {
            return false;
        }

        WNDCLASSW windowClass{};
        windowClass.lpfnWndProc = windowProc;
        windowClass.hInstance = GetModuleHandleW(nullptr);
        windowClass.lpszClassName = L"SolisBrowserDownload";

        if (!GetClassInfoW(
                windowClass.hInstance,
                windowClass.lpszClassName,
                &windowClass) &&
            !RegisterClassW(&windowClass)) {
            return false;
        }

        window_ = CreateWindowExW(
            WS_EX_DLGMODALFRAME,
            windowClass.lpszClassName,
            title.c_str(),
            WS_CAPTION | WS_SYSMENU,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            460,
            145,
            nullptr,
            nullptr,
            windowClass.hInstance,
            this);

        if (!window_) {
            return false;
        }

        ShowWindow(window_, SW_SHOWNORMAL);
        UpdateWindow(window_);
        return true;
    }

    void update(
        std::size_t downloaded,
        std::size_t total) {
        if (!window_) {
            return;
        }

        MSG message{};

        while (PeekMessageW(
                   &message,
                   nullptr,
                   0,
                   0,
                   PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        if (total > 0) {
            const auto percent =
                static_cast<int>(
                    static_cast<unsigned long long>(
                        downloaded) *
                    100ULL /
                    static_cast<unsigned long long>(
                        total));

            SendMessageW(
                progressBar_,
                PBM_SETPOS,
                percent,
                0);

            const auto text =
                std::to_wstring(percent) + L"%";

            SetWindowTextW(
                statusText_,
                text.c_str());
        }
    }

    void close() {
        if (window_) {
            DestroyWindow(window_);
            window_ = nullptr;
        }
    }

private:
    static LRESULT CALLBACK windowProc(
        HWND window,
        UINT message,
        WPARAM wParam,
        LPARAM lParam) {
        auto* self =
            reinterpret_cast<ProgressWindow*>(
                GetWindowLongPtrW(
                    window,
                    GWLP_USERDATA));

        if (message == WM_NCCREATE) {
            auto* create =
                reinterpret_cast<CREATESTRUCTW*>(
                    lParam);

            self =
                static_cast<ProgressWindow*>(
                    create->lpCreateParams);

            SetWindowLongPtrW(
                window,
                GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(self));

            self->window_ = window;
        }

        if (!self) {
            return DefWindowProcW(
                window,
                message,
                wParam,
                lParam);
        }

        if (message == WM_CREATE) {
            self->statusText_ = CreateWindowW(
                L"STATIC",
                L"0%",
                WS_CHILD | WS_VISIBLE,
                20,
                20,
                400,
                22,
                window,
                nullptr,
                GetModuleHandleW(nullptr),
                nullptr);

            self->progressBar_ = CreateWindowExW(
                0,
                PROGRESS_CLASSW,
                nullptr,
                WS_CHILD | WS_VISIBLE,
                20,
                52,
                400,
                24,
                window,
                nullptr,
                GetModuleHandleW(nullptr),
                nullptr);

            SendMessageW(
                self->progressBar_,
                PBM_SETRANGE,
                0,
                MAKELPARAM(0, 100));

            return 0;
        }

        if (message == WM_CLOSE) {
            EnableWindow(window, FALSE);
            return 0;
        }

        return DefWindowProcW(
            window,
            message,
            wParam,
            lParam);
    }

    HWND window_ = nullptr;
    HWND statusText_ = nullptr;
    HWND progressBar_ = nullptr;
};

}

bool ManagedBrowserDownloader::download(
    const std::string& url,
    const std::filesystem::path& destination,
    ProgressCallback progress) const {
    std::wstring host;
    std::wstring path;

    if (!parseUrl(url, host, path)) {
        return false;
    }

    ProgressWindow window;
    window.create(L"Solis - Downloading Managed Browser");

    HINTERNET session = WinHttpOpen(
        L"Solis-Browser/1.0",
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);

    if (!session) {
        window.close();
        return false;
    }

    HINTERNET connection = WinHttpConnect(
        session,
        host.c_str(),
        INTERNET_DEFAULT_HTTPS_PORT,
        0);

    if (!connection) {
        WinHttpCloseHandle(session);
        window.close();
        return false;
    }

    HINTERNET request = WinHttpOpenRequest(
        connection,
        L"GET",
        path.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);

    if (!request) {
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        window.close();
        return false;
    }

    const wchar_t* headers =
        L"Accept: application/octet-stream\r\n"
        L"User-Agent: Solis-Browser\r\n";

    bool success =
        WinHttpAddRequestHeaders(
            request,
            headers,
            static_cast<DWORD>(-1L),
            WINHTTP_ADDREQ_FLAG_ADD) &&
        WinHttpSendRequest(
            request,
            WINHTTP_NO_ADDITIONAL_HEADERS,
            0,
            WINHTTP_NO_REQUEST_DATA,
            0,
            0,
            0) &&
        WinHttpReceiveResponse(request, nullptr);

    if (!success) {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        window.close();
        return false;
    }

    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);

    success =
        WinHttpQueryHeaders(
            request,
            WINHTTP_QUERY_STATUS_CODE |
                WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,
            &statusCode,
            &statusSize,
            WINHTTP_NO_HEADER_INDEX) &&
        statusCode == 200;

    DWORD totalSize = 0;
    DWORD totalSizeBytes = sizeof(totalSize);

    WinHttpQueryHeaders(
        request,
        WINHTTP_QUERY_CONTENT_LENGTH |
            WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &totalSize,
        &totalSizeBytes,
        WINHTTP_NO_HEADER_INDEX);

    if (success) {
        std::ofstream output(
            destination,
            std::ios::binary | std::ios::trunc);

        if (!output) {
            success = false;
        } else {
            std::vector<char> buffer(64 * 1024);
            DWORD bytesRead = 0;
            std::size_t downloaded = 0;

            do {
                if (!WinHttpReadData(
                        request,
                        buffer.data(),
                        static_cast<DWORD>(buffer.size()),
                        &bytesRead)) {
                    success = false;
                    break;
                }

                if (bytesRead > 0) {
                    output.write(buffer.data(), bytesRead);
                    downloaded += bytesRead;

                    window.update(
                        downloaded,
                        totalSize);

                    if (progress) {
                        progress(
                            downloaded,
                            totalSize);
                    }
                }
            } while (bytesRead > 0);
        }
    }

    if (success) {
        window.update(
            totalSize,
            totalSize);
    }

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);
    window.close();

    return success;
}

}

#else

namespace solis {

bool ManagedBrowserDownloader::download(
    const std::string&,
    const std::filesystem::path&,
    ProgressCallback) const {
    return false;
}

}

#endif
