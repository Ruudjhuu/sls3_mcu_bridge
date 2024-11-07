
# Dependencies

# cxxopts
include(FetchContent)

FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
  GIT_TAG        v3.2.1
)
FetchContent_MakeAvailable(cxxopts)

# libremidi
include(FetchContent)

FetchContent_Declare(
  libremidi
  GIT_REPOSITORY https://github.com/celtera/libremidi.git
  GIT_TAG        v4.5.0
)
FetchContent_MakeAvailable(libremidi)

# spdlog
include(FetchContent)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.14.1
)
FetchContent_MakeAvailable(spdlog)


# asio
FetchContent_Declare(asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG asio-1-31-0
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  )

FetchContent_MakeAvailable(asio)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
find_package(Threads)
target_link_libraries(asio INTERFACE Threads::Threads)

# Project library

add_library(${CMAKE_PROJECT_NAME}_lib 
  package.cpp package.hpp
  client.cpp client.hpp
  bridge.cpp bridge.hpp
  mididevice.cpp mididevice.hpp)
target_link_libraries(${CMAKE_PROJECT_NAME}_lib PUBLIC libremidi asio spdlog gcov)
set_property(TARGET ${CMAKE_PROJECT_NAME}_lib  PROPERTY COMPILE_WARNING_AS_ERROR ON)

target_compile_options(${CMAKE_PROJECT_NAME}_lib PUBLIC 
"--coverage")
target_link_options(${CMAKE_PROJECT_NAME}_lib PUBLIC "-coverage")

# Executable
add_executable(${CMAKE_PROJECT_NAME})
target_sources(${CMAKE_PROJECT_NAME} PRIVATE main.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_PROJECT_NAME}_lib cxxopts)
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)
