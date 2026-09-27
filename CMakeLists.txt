cmake_minimum_required(VERSION 3.20)

set(SOLIS_VERSION "1.0.0" CACHE STRING "Solis application version")
project(solis VERSION ${SOLIS_VERSION} LANGUAGES CXX)

find_package(Qt6 6.11 REQUIRED COMPONENTS Gui Qml Quick QuickControls2)
qt_standard_project_setup(REQUIRES 6.8)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

qt_add_executable(solis
    src/main.cpp
    src/app/Application.cpp
    src/analysis/Analyzer.cpp
    src/browser/BrowserManager.cpp
    src/browser/ManagedBrowserDownloader.cpp
    src/browser/ManagedBrowserBootstrapper.cpp
    src/core/EventBus.cpp
    src/core/Version.cpp
    src/engines/EngineManager.cpp
    src/settings/Settings.cpp
    src/sites/SiteManager.cpp
    src/updater/GitHubReleaseProvider.cpp
    src/updater/Sha256Verifier.cpp
    src/updater/Updater.cpp
    src/updater/WindowsInstaller.cpp
)

target_include_directories(solis PRIVATE src)
target_compile_definitions(solis PRIVATE SOLIS_VERSION="${PROJECT_VERSION}")

qt_add_qml_module(solis
    URI Solis
    VERSION 1.0
    QML_FILES
        src/ui/Main.qml
)

target_link_libraries(solis PRIVATE
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

if(WIN32)
    add_executable(solis-updater
        src/updater/DeltaTool.cpp
        src/updater/Sha256Verifier.cpp
    )

    target_include_directories(solis-updater PRIVATE src)
    target_link_libraries(solis-updater PRIVATE msdelta bcrypt shell32)
endif()

if(WIN32)
    target_link_libraries(solis PRIVATE winhttp bcrypt comctl32 shell32)
endif()

if(MSVC)
    set_property(TARGET solis PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_compile_options(solis PRIVATE /W4 /permissive-)
else()
    target_compile_options(solis PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(MSVC AND WIN32)
    target_compile_options(solis-updater PRIVATE /W4 /permissive-)
elseif(WIN32)
    target_compile_options(solis-updater PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32)
    set(CPACK_PACKAGE_NAME "Solis")
    set(CPACK_PACKAGE_VENDOR "solis-syst")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Solis Chess Analyzer")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Solis")
    set(CPACK_GENERATOR "NSIS")

    install(TARGETS solis solis-updater RUNTIME DESTINATION .)

    include(CPack)
endif()
