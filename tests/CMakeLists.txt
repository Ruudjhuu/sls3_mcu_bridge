enable_testing()

include(GoogleTest)
include(CTest)
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

include_directories(${COMMON_INCLUDES})

add_executable(unit_tests 
  test_unit_package.cpp)
target_link_libraries(unit_tests PRIVATE ${CMAKE_PROJECT_NAME}_lib GTest::GTest)
gtest_discover_tests(unit_tests)
set_property(TARGET unit_tests PROPERTY COMPILE_WARNING_AS_ERROR ON)

