#include "updater/Sha256Verifier.h"

#ifdef _WIN32

#include <windows.h>
#include <msdelta.h>

#include <filesystem>
#include <string>

namespace {

int createDelta(const std::filesystem::path& source,
                const std::filesystem::path& target,
                const std::filesystem::path& delta) {
    std::filesystem::remove(delta);

    solis::Delta_input_placeholder;
    return 0;
}

}

int wmain(int argc, wchar_t* argv[]) {
    return 0;
}

#else

int main() {
    return 1;
}

#endif
