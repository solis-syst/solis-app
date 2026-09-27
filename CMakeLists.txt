cmake_minimum_required(VERSION 3.20)

set(SOLIS_VERSION "1.0.0" CACHE STRING "Solis application version")
project(solis VERSION ${SOLIS_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(solis
    src/main.cpp
    src/app/Application.cpp
    src/analysis/Analyzer.cpp
    src/browser/BrowserManager.cpp
    src/core/EventBus.cpp
    src/core/Version.cpp
    src/engines/EngineManager.cpp
    src/settings/Settings.cpp
    src/sites/SiteManager.cpp
    src/updater/GitHubReleaseProvider.cpp
    src/updater/Updater.cpp
    src/updater/WindowsInstaller.cpp
)

target_include_directories(solis PRIVATE src)
target_compile_definitions(solis PRIVATE SOLIS_VERSION="${PROJECT_VERSION}")

if(WIN32)
    target_link_libraries(solis PRIVATE winhttp)
endif()

if(MSVC)
    target_compile_options(solis PRIVATE /W4 /permissive-)
else()
    target_compile_options(solis PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CPack)

set(CPACK_PACKAGE_NAME "Solis")
set(CPACK_PACKAGE_VENDOR "solis-syst")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Solis Chess Analyzer")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Solis")
set(CPACK_GENERATOR "NSIS")

install(TARGETS solis RUNTIME DESTINATION .)
