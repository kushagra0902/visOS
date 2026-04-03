cmake_minimum_required(VERSION 3.20)
project(VisOS VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/CompilerWarnings.cmake)

option(VISOS_BUILD_GUI "Build the Qt GUI application" OFF)
option(VISOS_BUILD_TESTS "Build unit and integration tests" ON)
option(VISOS_BUILD_CLI "Build the CLI application" ON)

add_subdirectory(src)
add_subdirectory(app)

if(VISOS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(third_party)
    add_subdirectory(tests)
endif()
