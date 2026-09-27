#include "browser/ManagedBrowserBootstrapper.h"

#include "browser/ManagedBrowserDownloader.h"

#ifdef _WIN32

#include <windows.h>
#include <winhttp.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace solis {

namespace {

constexpr char kAvailabilityUrl[] =
    "https://googlechromelabs.github.io/chrome-for-testing/last-known-good-versions-with-downloads.json";

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

bool getText(
    const std::string& url,
    std::string& body) {
    std::wstring host;
    std::wstring path;

    if (!parseUrl(url, host, path)) {
        return false;
    }

    HINTERNET session = WinHttpOpen(
        L"Solis-Bootstrapper/1.0",
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);

    if (!session) {
        return false;
    }

    HINTERNET connection = WinHttpConnect(
        session,
        host.c_str(),
        INTERNET_DEFAULT_HTTPS_PORT,
        0);

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
        WINHTTP_FLAG_SECURE);

    if (!request) {
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    const wchar_t* headers =
        L"Accept: application/json\r\n"
        L"User-Agent: Solis-Bootstrapper\r\n";

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

    if (success) {
        std::vector<char> buffer(16 * 1024);
        DWORD bytesRead = 0;

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
                body.append(buffer.data(), bytesRead);
            }
        } while (bytesRead > 0);
    }

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);

    return success;
}

std::string extractString(
    const std::string& json,
    const std::string& key,
    std::size_t start) {
    const std::string marker = "\"" + key + "\"";
    const auto keyPos = json.find(marker, start);

    if (keyPos == std::string::npos) {
        return {};
    }

    const auto colon = json.find(':', keyPos + marker.size());

    if (colon == std::string::npos) {
        return {};
    }

    const auto quote = json.find('"', colon + 1);

    if (quote == std::string::npos) {
        return {};
    }

    std::string result;

    for (std::size_t i = quote + 1; i < json.size(); ++i) {
        if (json[i] == '\\' && i + 1 < json.size()) {
            result.push_back(json[++i]);
            continue;
        }

        if (json[i] == '"') {
            break;
        }

        result.push_back(json[i]);
    }

    return result;
}

std::wstring quotePowerShell(
    const std::filesystem::path& value) {
    std::wstring result = L"'";
    const auto text = value.wstring();

    for (const wchar_t ch : text) {
        if (ch == L'\'') {
            result += L"''";
        } else {
            result += ch;
        }
    }

    result += L"'";
    return result;
}

bool runPowerShell(
    const std::wstring& command) {
    std::wstring commandLine =
        L"powershell.exe -NoLogo -NoProfile -NonInteractive "
        L"-ExecutionPolicy Bypass -Command \"" +
        command +
        L"\"";

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};

    std::vector<wchar_t> buffer(
        commandLine.begin(),
        commandLine.end());

    buffer.push_back(L'\0');

    const BOOL created = CreateProcessW(
        nullptr,
        buffer.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &startup,
        &process);

    if (!created) {
        return false;
    }

    WaitForSingleObject(process.hProcess, INFINITE);

    DWORD exitCode = 1;
    GetExitCodeProcess(
        process.hProcess,
        &exitCode);

    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);

    return exitCode == 0;
}

bool writeVersion(
    const std::filesystem::path& path,
    const std::string& version) {
    std::ofstream output(path, std::ios::trunc);

    if (!output) {
        return false;
    }

    output << version;
    return static_cast<bool>(output);
}

bool readVersion(
    const std::filesystem::path& path,
    std::string& version) {
    std::ifstream input(path);

    if (!input) {
        return false;
    }

    std::getline(input, version);

    return !version.empty();
}

}

std::filesystem::path ManagedBrowserBootstrapper::browserRoot() const {
    wchar_t localAppData[32768]{};

    const DWORD length = GetEnvironmentVariableW(
        L"LOCALAPPDATA",
        localAppData,
        static_cast<DWORD>(
            sizeof(localAppData) /
            sizeof(localAppData[0])));

    if (length == 0 ||
        length >= sizeof(localAppData) /
                      sizeof(localAppData[0])) {
        return {};
    }

    return std::filesystem::path(localAppData) /
           L"Solis" /
           L"ManagedBrowser";
}

bool ManagedBrowserBootstrapper::fetchStableRelease(
    std::string& version,
    std::string& downloadUrl) const {
    std::string body;

    if (!getText(kAvailabilityUrl, body)) {
        return false;
    }

    const auto stablePos =
        body.find("\"Stable\"");

    if (stablePos == std::string::npos) {
        return false;
    }

    version =
        extractString(
            body,
            "version",
            stablePos);

    const auto chromePos =
        body.find("\"chrome\"", stablePos);

    if (chromePos == std::string::npos) {
        return false;
    }

    const auto win64Pos =
        body.find("\"platform\": \"win64\"", chromePos);

    if (win64Pos == std::string::npos) {
        return false;
    }

    downloadUrl =
        extractString(
            body,
            "url",
            win64Pos);

    return !version.empty() &&
           downloadUrl.starts_with(
               "https://storage.googleapis.com/chrome-for-testing-public/") &&
           downloadUrl.ends_with(
               "/win64/chrome-win64.zip");
}

bool ManagedBrowserBootstrapper::extractArchive(
    const std::filesystem::path& archive,
    const std::filesystem::path& destination) const {
    std::filesystem::remove_all(destination);

    if (!std::filesystem::create_directories(destination)) {
        return false;
    }

    const auto command =
        L"Expand-Archive -LiteralPath " +
        quotePowerShell(archive) +
        L" -DestinationPath " +
        quotePowerShell(destination) +
        L" -Force";

    return runPowerShell(command);
}

bool ManagedBrowserBootstrapper::ensureInstalled(
    std::filesystem::path& executable) const {
    const auto root = browserRoot();

    if (root.empty()) {
        return false;
    }

    std::string version;
    std::string downloadUrl;

    if (!fetchStableRelease(version, downloadUrl)) {
        return false;
    }

    const auto versionDirectory = root / toWide(version);
    const auto candidate =
        versionDirectory /
        L"chrome-win64" /
        L"chrome.exe";
    const auto marker = versionDirectory / L"version.txt";

    std::string installedVersion;

    if (std::filesystem::exists(candidate) &&
        readVersion(marker, installedVersion) &&
        installedVersion == version) {
        executable = candidate;
        return true;
    }

    std::error_code error;
    std::filesystem::create_directories(root, error);

    if (error) {
        return false;
    }

    const auto temporaryRoot =
        root / (L".staging-" + toWide(version));
    const auto archive =
        temporaryRoot / L"chrome.zip";
    const auto extracted =
        temporaryRoot / L"extracted";

    std::filesystem::remove_all(temporaryRoot, error);

    if (!std::filesystem::create_directories(
            temporaryRoot,
            error) ||
        error) {
        return false;
    }

    ManagedBrowserDownloader downloader;

    if (!downloader.download(
            downloadUrl,
            archive)) {
        std::filesystem::remove_all(
            temporaryRoot,
            error);
        return false;
    }

    if (!extractArchive(
            archive,
            extracted)) {
        std::filesystem::remove_all(
            temporaryRoot,
            error);
        return false;
    }

    const auto extractedExecutable =
        extracted /
        L"chrome-win64" /
        L"chrome.exe";

    if (!std::filesystem::exists(
            extractedExecutable)) {
        std::filesystem::remove_all(
            temporaryRoot,
            error);
        return false;
    }

    std::filesystem::remove_all(
        versionDirectory,
        error);

    if (error) {
        std::filesystem::remove_all(
            temporaryRoot,
            error);
        return false;
    }

    std::filesystem::create_directories(
        versionDirectory,
        error);

    if (error) {
        std::filesystem::remove_all(
            temporaryRoot,
            error);
        return false;
    }

    const auto finalBrowserDirectory =
        versionDirectory /
        L"chrome-win64";

    std::filesystem::rename(
        extracted / L"chrome-win64",
        finalBrowserDirectory,
        error);

    if (error) {
        std::filesystem::remove_all(
            temporaryRoot,
            error);
        return false;
    }

    if (!writeVersion(marker, version)) {
        std::filesystem::remove_all(
            versionDirectory,
            error);
        std::filesystem::remove_all(
            temporaryRoot,
            error);
        return false;
    }

    std::filesystem::remove_all(
        temporaryRoot,
        error);

    executable =
        versionDirectory /
        L"chrome-win64" /
        L"chrome.exe";

    return std::filesystem::exists(executable);
}

}

#else

namespace solis {

bool ManagedBrowserBootstrapper::ensureInstalled(
    std::filesystem::path&) const {
    return false;
}

}

#endif
