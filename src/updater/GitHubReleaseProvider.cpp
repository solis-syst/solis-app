#include "updater/GitHubReleaseProvider.h"

#ifdef _WIN32

#include <windows.h>
#include <winhttp.h>

#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <utility>

namespace solis {

namespace {

std::wstring toWide(const std::string& value) {
    if (value.empty()) {
        return {};
    }

    const int size = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
    if (size <= 0) {
        return {};
    }

    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size);
    return result;
}

std::string toUtf8(const std::wstring& value) {
    if (value.empty()) {
        return {};
    }

    const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return {};
    }

    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size, nullptr, nullptr);
    return result;
}

bool parseUrl(const std::string& url, std::wstring& host, std::wstring& path) {
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

    if (!WinHttpCrackUrl(wideUrl.c_str(), static_cast<DWORD>(wideUrl.size()), 0, &components)) {
        return false;
    }

    if (components.nScheme != INTERNET_SCHEME_HTTPS) {
        return false;
    }

    host.assign(components.lpszHostName, components.dwHostNameLength);

    path.assign(components.lpszUrlPath, components.dwUrlPathLength);
    if (components.dwExtraInfoLength > 0) {
        path.append(components.lpszExtraInfo, components.dwExtraInfoLength);
    }

    return true;
}

bool httpGet(const std::string& url, std::string& body) {
    std::wstring host;
    std::wstring path;

    if (!parseUrl(url, host, path)) {
        return false;
    }

    HINTERNET session = WinHttpOpen(
        L"Solis-Updater/1.0",
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );

    if (!session) {
        return false;
    }

    HINTERNET connection = WinHttpConnect(
        session,
        host.c_str(),
        INTERNET_DEFAULT_HTTPS_PORT,
        0
    );

    if (!connection) {
        WinHttpCloseHandle(session);
        return false;
    }

    HINTERNET request = WinHttpOpenRequest(
        connection,
        L"GET",
        path.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE
    );

    if (!request) {
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    const wchar_t* headers =
        L"Accept: application/vnd.github+json\r\n"
        L"User-Agent: Solis-Updater\r\n"
        L"X-GitHub-Api-Version: 2022-11-28\r\n";

    bool success = WinHttpAddRequestHeaders(
        request,
        headers,
        static_cast<DWORD>(-1L),
        WINHTTP_ADDREQ_FLAG_ADD
    ) &&
    WinHttpSendRequest(
        request,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        0,
        0
    ) &&
    WinHttpReceiveResponse(request, nullptr);

    if (!success) {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);

    success = WinHttpQueryHeaders(
        request,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &statusCode,
        &statusSize,
        WINHTTP_NO_HEADER_INDEX
    ) && statusCode == 200;

    if (success) {
        std::vector<char> buffer(16 * 1024);
        DWORD bytesRead = 0;

        do {
            if (!WinHttpReadData(request, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead)) {
                success = false;
                break;
            }

            body.append(buffer.data(), bytesRead);
        } while (bytesRead > 0);
    }

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);

    return success;
}

bool httpDownload(const std::string& url,
                  const std::filesystem::path& destination,
                  GitHubReleaseProvider::ProgressCallback progress) {
    std::wstring host;
    std::wstring path;

    if (!parseUrl(url, host, path)) {
        return false;
    }

    HINTERNET session = WinHttpOpen(
        L"Solis-Updater/1.0",
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );

    if (!session) {
        return false;
    }

    HINTERNET connection = WinHttpConnect(
        session,
        host.c_str(),
        INTERNET_DEFAULT_HTTPS_PORT,
        0
    );

    if (!connection) {
        WinHttpCloseHandle(session);
        return false;
    }

    HINTERNET request = WinHttpOpenRequest(
        connection,
        L"GET",
        path.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE
    );

    if (!request) {
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    const wchar_t* headers =
        L"Accept: application/octet-stream\r\n"
        L"User-Agent: Solis-Updater\r\n";

    bool success = WinHttpAddRequestHeaders(
        request,
        headers,
        static_cast<DWORD>(-1L),
        WINHTTP_ADDREQ_FLAG_ADD
    ) &&
    WinHttpSendRequest(
        request,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        0,
        0
    ) &&
    WinHttpReceiveResponse(request, nullptr);

    if (!success) {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);

    success = WinHttpQueryHeaders(
        request,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &statusCode,
        &statusSize,
        WINHTTP_NO_HEADER_INDEX
    ) && statusCode == 200;

    DWORD totalSize = 0;
    DWORD totalSizeBytes = sizeof(totalSize);

    WinHttpQueryHeaders(
        request,
        WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &totalSize,
        &totalSizeBytes,
        WINHTTP_NO_HEADER_INDEX
    );

    if (success) {
        std::ofstream output(destination, std::ios::binary | std::ios::trunc);

        if (!output) {
            success = false;
        } else {
            std::vector<char> buffer(64 * 1024);
            DWORD bytesRead = 0;
            std::size_t downloaded = 0;

            do {
                if (!WinHttpReadData(request, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead)) {
                    success = false;
                    break;
                }

                if (bytesRead > 0) {
                    output.write(buffer.data(), bytesRead);
                    downloaded += bytesRead;

                    if (progress) {
                        progress(downloaded, totalSize);
                    }
                }
            } while (bytesRead > 0);
        }
    }

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);

    return success;
}

std::string extractString(const std::string& json, const std::string& key, std::size_t start = 0) {
    const std::string marker = "\"" + key + "\"";
    const auto keyPos = json.find(marker, start);

    if (keyPos == std::string::npos) {
        return {};
    }

    const auto colon = json.find(':', keyPos + marker.size());
    if (colon == std::string::npos) {
        return {};
    }

    const auto firstQuote = json.find('\"', colon + 1);
    if (firstQuote == std::string::npos) {
        return {};
    }

    std::string result;

    for (std::size_t i = firstQuote + 1; i < json.size(); ++i) {
        if (json[i] == '\\' && i + 1 < json.size()) {
            result.push_back(json[++i]);
            continue;
        }

        if (json[i] == '\"') {
            break;
        }

        result.push_back(json[i]);
    }

    return result;
}

bool extractInstaller(const std::string& json, std::string& name, std::string& url) {
    std::size_t search = json.find("\"assets\"");

    while (search != std::string::npos) {
        const auto namePos = json.find("\"name\"", search);
        const auto nextAssets = json.find("\"assets\"", search + 1);

        if (namePos == std::string::npos || (nextAssets != std::string::npos && namePos > nextAssets)) {
            search = nextAssets;
            continue;
        }

        const auto assetName = extractString(json, "name", namePos);
        const auto urlPos = json.find("\"browser_download_url\"", namePos);

        if (urlPos != std::string::npos &&
            (nextAssets == std::string::npos || urlPos < nextAssets) &&
            assetName.size() >= 4 &&
            assetName.starts_with("Solis") &&
            assetName.ends_with(".exe")) {
            name = assetName;
            url = extractString(json, "browser_download_url", urlPos);
            return !url.empty();
        }

        search = nextAssets;
    }

    return false;
}

}

GitHubReleaseProvider::GitHubReleaseProvider(std::string owner, std::string repository)
    : owner_(std::move(owner)), repository_(std::move(repository)) {
}

bool GitHubReleaseProvider::fetchLatest(UpdateInfo& update) const {
    std::string body;

    const auto url =
        "https://api.github.com/repos/" + owner_ + "/" + repository_ + "/releases/latest";

    if (!httpGet(url, body)) {
        return false;
    }

    update.tag = extractString(body, "tag_name");
    update.version = update.tag;

    if (!update.version.empty() && update.version.front() == 'v') {
        update.version.erase(update.version.begin());
    }

    update.releaseUrl = extractString(body, "html_url");

    std::string installerName;
    if (!extractInstaller(body, installerName, update.installerUrl)) {
        return false;
    }

    update.checksumUrl = update.installerUrl + ".sha256";

    return !update.version.empty() &&
           !update.installerUrl.empty() &&
           !update.checksumUrl.empty();
}

bool GitHubReleaseProvider::downloadInstaller(
    const UpdateInfo& update,
    const std::filesystem::path& destination,
    ProgressCallback progress) const {
    if (update.installerUrl.empty()) {
        return false;
    }

    return httpDownload(update.installerUrl, destination, std::move(progress));
}

bool GitHubReleaseProvider::downloadChecksum(
    const UpdateInfo& update,
    const std::filesystem::path& destination) const {
    if (update.checksumUrl.empty()) {
        return false;
    }

    return httpDownload(update.checksumUrl, destination, {});
}

}

#else

#include <utility>

namespace solis {

GitHubReleaseProvider::GitHubReleaseProvider(std::string owner, std::string repository)
    : owner_(std::move(owner)), repository_(std::move(repository)) {
}

bool GitHubReleaseProvider::fetchLatest(UpdateInfo&) const {
    return false;
}

bool GitHubReleaseProvider::downloadInstaller(
    const UpdateInfo&,
    const std::filesystem::path&,
    ProgressCallback) const {
    return false;
}

bool GitHubReleaseProvider::downloadChecksum(
    const UpdateInfo&,
    const std::filesystem::path&) const {
    return false;
}

}

#endif
