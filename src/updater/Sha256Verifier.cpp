#include "updater/Sha256Verifier.h"

#ifdef _WIN32

#include <bcrypt.h>

#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

namespace solis {

namespace {

std::string normalize(std::string value) {
    for (char& ch : value) {
        if (ch >= 'A' && ch <= 'F') {
            ch = static_cast<char>(ch - 'A' + 'a');
        }
    }

    return value;
}

}

bool Sha256Verifier::verify(const std::filesystem::path& file,
                            const std::filesystem::path& checksumFile) {
    std::ifstream checksumInput(checksumFile);
    std::string expected;

    if (!checksumInput || !(checksumInput >> expected) || expected.size() != 64) {
        return false;
    }

    expected = normalize(expected);

    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;

    DWORD objectSize = 0;
    DWORD objectSizeBytes = sizeof(objectSize);
    DWORD hashSize = 0;
    DWORD hashSizeBytes = sizeof(hashSize);

    if (BCryptOpenAlgorithmProvider(
            &algorithm,
            BCRYPT_SHA256_ALGORITHM,
            nullptr,
            0) < 0) {
        return false;
    }

    if (BCryptGetProperty(
            algorithm,
            BCRYPT_OBJECT_LENGTH,
            reinterpret_cast<PUCHAR>(&objectSize),
            sizeof(objectSize),
            &objectSizeBytes,
            0) < 0 ||
        BCryptGetProperty(
            algorithm,
            BCRYPT_HASH_LENGTH,
            reinterpret_cast<PUCHAR>(&hashSize),
            sizeof(hashSize),
            &hashSizeBytes,
            0) < 0) {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return false;
    }

    std::vector<UCHAR> object(objectSize);

    if (BCryptCreateHash(
            algorithm,
            &hash,
            object.data(),
            static_cast<ULONG>(object.size()),
            nullptr,
            0,
            0) < 0) {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return false;
    }

    std::ifstream input(file, std::ios::binary);
    std::vector<char> buffer(64 * 1024);
    bool success = static_cast<bool>(input);

    while (success && input) {
        input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const auto count = input.gcount();

        if (count > 0 &&
            BCryptHashData(
                hash,
                reinterpret_cast<PUCHAR>(buffer.data()),
                static_cast<ULONG>(count),
                0) < 0) {
            success = false;
        }
    }

    std::vector<UCHAR> digest(hashSize);

    if (success &&
        BCryptFinishHash(
            hash,
            digest.data(),
            static_cast<ULONG>(digest.size()),
            0) < 0) {
        success = false;
    }

    BCryptDestroyHash(hash);
    BCryptCloseAlgorithmProvider(algorithm, 0);

    if (!success) {
        return false;
    }

    static constexpr char hex[] = "0123456789abcdef";
    std::string actual;
    actual.reserve(digest.size() * 2);

    for (const auto byte : digest) {
        actual.push_back(hex[(byte >> 4) & 0x0F]);
        actual.push_back(hex[byte & 0x0F]);
    }

    return actual == expected;
}

}

#else

namespace solis {

bool Sha256Verifier::verify(const std::filesystem::path&,
                            const std::filesystem::path&) {
    return false;
}

}

#endif
