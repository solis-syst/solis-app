#include "updater/Updater.h"

#include "core/Version.h"
#include "updater/GitHubReleaseProvider.h"
#include "updater/Sha256Verifier.h"
#include "updater/WindowsInstaller.h"

#include <filesystem>
#include <string>
#include <utility>

#ifdef _WIN32

#include <windows.h>

namespace solis {

namespace {

void log(const std::wstring& message) {
    OutputDebugStringW((L"[Solis Updater] " + message + L"\n").c_str());
}

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

std::filesystem::path temporaryPath(
    const std::string& version,
    const std::wstring& suffix) {
    wchar_t buffer[MAX_PATH]{};
    const DWORD length = GetTempPathW(MAX_PATH, buffer);

    if (length == 0 || length >= MAX_PATH) {
        return {};
    }

    return std::filesystem::path(buffer) /
           (L"Solis-Update-" + toWide(version) + suffix);
}

std::filesystem::path currentExecutablePath() {
    wchar_t buffer[32768]{};
    const DWORD length = GetModuleFileNameW(
        nullptr,
        buffer,
        static_cast<DWORD>(std::size(buffer)));

    if (length == 0 || length >= std::size(buffer)) {
        return {};
    }

    return std::filesystem::path(
        std::wstring(buffer, length));
}

void downloadProgress(
    const wchar_t* label,
    std::size_t downloaded,
    std::size_t total) {
    if (total == 0) {
        return;
    }

    const auto percent =
        static_cast<unsigned long long>(downloaded) * 100ULL /
        static_cast<unsigned long long>(total);

    OutputDebugStringW(
        (L"[Solis Updater] " +
         std::wstring(label) +
         L": " +
         std::to_wstring(percent) +
         L"%\n").c_str());
}

}

Updater::Updater(std::string owner, std::string repository)
    : owner_(std::move(owner)), repository_(std::move(repository)) {
}

bool Updater::initialize() {
    return true;
}

void Updater::shutdown() {
}

bool Updater::checkForUpdates(bool promptUser) {
    GitHubReleaseProvider provider(owner_, repository_);

    UpdateInfo update;
    if (!provider.fetchLatest(update)) {
        log(L"Unable to fetch latest release.");
        return false;
    }

    if (Version::compare(Version::current(), update.version) >= 0) {
        log(L"No newer release found.");
        return true;
    }

    const auto message =
        L"Solis " + toWide(update.version) +
        L" is available.\n\nCurrent version: " +
        toWide(Version::current()) +
        L"\n\nDownload and install it now?";

    if (promptUser) {
        const int answer = MessageBoxW(
            nullptr,
            message.c_str(),
            L"Solis Update Available",
            MB_ICONINFORMATION | MB_YESNO | MB_DEFBUTTON1);

        if (answer != IDYES) {
            log(L"User postponed update.");
            return true;
        }
    }

    const bool canUseDelta =
        update.deltaFromVersion == Version::current() &&
        !update.deltaUrl.empty() &&
        !update.deltaChecksumUrl.empty() &&
        !update.applicationChecksumUrl.empty();

    if (canUseDelta) {
        const auto deltaPath = temporaryPath(update.version, L".delta");
        const auto deltaChecksumPath = temporaryPath(
            update.version,
            L".delta.sha256");
        const auto applicationChecksumPath = temporaryPath(
            update.version,
            L".application.sha256");

        if (!deltaPath.empty() &&
            !deltaChecksumPath.empty() &&
            !applicationChecksumPath.empty()) {
            log(L"Downloading differential update.");

            if (provider.downloadDelta(
                    update,
                    deltaPath,
                    [](std::size_t downloaded, std::size_t total) {
                        downloadProgress(L"Delta download", downloaded, total);
                    }) &&
                provider.downloadDeltaChecksum(update, deltaChecksumPath) &&
                Sha256Verifier::verify(deltaPath, deltaChecksumPath) &&
                provider.downloadApplicationChecksum(
                    update,
                    applicationChecksumPath)) {
                const auto currentExecutable = currentExecutablePath();

                const auto readyMessage =
                    L"The differential Solis update is ready.\n\n" +
                    toWide(update.version) +
                    L" will be applied after Solis closes.\n\nRestart and install now?";

                if (!currentExecutable.empty() &&
                    MessageBoxW(
                        nullptr,
                        readyMessage.c_str(),
                        L"Solis Update Ready",
                        MB_ICONINFORMATION | MB_YESNO | MB_DEFBUTTON1) == IDYES) {
                    if (WindowsInstaller::launchDeltaUpdate(
                            deltaPath,
                            applicationChecksumPath,
                            currentExecutable)) {
                        restartRequested_ = true;
                        return true;
                    }

                    log(L"Differential update launch failed.");
                }
            } else {
                log(L"Differential update download or verification failed.");
            }

            std::filesystem::remove(deltaPath);
            std::filesystem::remove(deltaChecksumPath);
            std::filesystem::remove(applicationChecksumPath);
        }
    }

    const auto destination = temporaryPath(update.version, L".exe");
    if (destination.empty()) {
        log(L"Unable to create temporary installer path.");
        return false;
    }

    log(L"Downloading full installer.");

    if (!provider.downloadInstaller(
            update,
            destination,
            [](std::size_t downloaded, std::size_t total) {
                downloadProgress(
                    L"Installer download",
                    downloaded,
                    total);
            })) {
        log(L"Update download failed.");
        std::filesystem::remove(destination);
        return false;
    }

    const auto checksumPath =
        std::filesystem::path(destination.string() + ".sha256");

    log(L"Downloading full installer checksum.");

    if (!provider.downloadChecksum(update, checksumPath)) {
        log(L"Checksum download failed.");
        std::filesystem::remove(destination);
        return false;
    }

    if (!Sha256Verifier::verify(destination, checksumPath)) {
        log(L"Installer checksum verification failed.");
        std::filesystem::remove(destination);
        std::filesystem::remove(checksumPath);
        return false;
    }

    std::filesystem::remove(checksumPath);

    const auto readyMessage =
        L"The Solis update has been downloaded.\n\n" +
        toWide(update.version) +
        L" will be installed when you restart Solis.\n\nRestart and install now?";

    if (MessageBoxW(
            nullptr,
            readyMessage.c_str(),
            L"Solis Update Ready",
            MB_ICONINFORMATION | MB_YESNO | MB_DEFBUTTON1) != IDYES) {
        log(L"User postponed installation.");
        return true;
    }

    if (!WindowsInstaller::launch(destination)) {
        log(L"Unable to launch installer.");
        return false;
    }

    restartRequested_ = true;
    return true;
}

bool Updater::restartRequested() const {
    return restartRequested_;
}

}

#else

namespace solis {

Updater::Updater(std::string owner, std::string repository)
    : owner_(std::move(owner)), repository_(std::move(repository)) {
}

bool Updater::initialize() {
    return true;
}

void Updater::shutdown() {
}

bool Updater::checkForUpdates(bool) {
    return true;
}

bool Updater::restartRequested() const {
    return false;
}

}

#endif
