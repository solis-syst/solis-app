#include "core/Version.h"

#include <charconv>
#include <system_error>
#include <string_view>
#include <vector>

#ifndef SOLIS_VERSION
#define SOLIS_VERSION "0.0.0"
#endif

namespace solis {

namespace {

std::vector<int> parse(const std::string& value) {
    std::vector<int> parts;
    std::string_view view(value);

    while (!view.empty()) {
        const auto dot = view.find('.');
        const auto token = view.substr(0, dot);

        int number = 0;
        const auto result = std::from_chars(token.data(), token.data() + token.size(), number);

        parts.push_back(result.ec == std::errc{} ? number : 0);

        if (dot == std::string_view::npos) {
            break;
        }

        view.remove_prefix(dot + 1);
    }

    while (parts.size() < 3) {
        parts.push_back(0);
    }

    return parts;
}

}

std::string Version::current() {
    return SOLIS_VERSION;
}

int Version::compare(const std::string& left, const std::string& right) {
    const auto a = parse(left);
    const auto b = parse(right);

    for (std::size_t i = 0; i < 3; ++i) {
        if (a[i] < b[i]) {
            return -1;
        }

        if (a[i] > b[i]) {
            return 1;
        }
    }

    return 0;
}

}
