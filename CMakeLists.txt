cmake_minimum_required(VERSION 3.5.0)
project(sls3_reverse_eng VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/src)

enable_testing()

# Program
add_subdirectory(src)

# Testing
include(GoogleTest)

add_subdirectory(tests)