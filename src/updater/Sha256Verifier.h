#pragma once

#include <filesystem>

namespace solis {

class Sha256Verifier {
public:
    static bool verify(const std::filesystem::path& file,
                       const std::filesystem::path& checksumFile);
};

}
