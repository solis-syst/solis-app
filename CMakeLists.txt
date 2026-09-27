cmake_minimum_required(VERSION 3.20)
project(solis LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_executable(solis
 src/main.cpp
 src/app/Application.cpp
 src/analysis/Analyzer.cpp
 src/browser/BrowserManager.cpp
 src/core/EventBus.cpp
 src/engines/EngineManager.cpp
 src/settings/Settings.cpp
 src/sites/SiteManager.cpp
)
target_include_directories(solis PRIVATE src)
if(MSVC)
 target_compile_options(solis PRIVATE /W4 /permissive-)
else()
 target_compile_options(solis PRIVATE -Wall -Wextra -Wpedantic)
endif()
