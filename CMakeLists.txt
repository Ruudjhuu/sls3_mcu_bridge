cmake_minimum_required(VERSION 3.5.0)
project(sls3_mcu_bridge VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/src)

enable_testing()

add_custom_target(analyze COMMAND CodeChecker analyze --config ${PROJECT_SOURCE_DIR}/.codechecker.yaml -i ${PROJECT_SOURCE_DIR}/.codechecker_skipfile.txt -o ${PROJECT_BINARY_DIR}/codechecker/reports ${PROJECT_BINARY_DIR}/compile_commands.json)
add_custom_target(analyze_report COMMAND CodeChecker parse -e html ${PROJECT_BINARY_DIR}/codechecker/reports --output ${PROJECT_BINARY_DIR}/codechecker_html_report)

# Program
add_subdirectory(src)

# Testing
include(GoogleTest)

add_subdirectory(tests)