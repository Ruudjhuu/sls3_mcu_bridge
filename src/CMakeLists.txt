
# Dependencies

# libremidi
include(FetchContent)

FetchContent_Declare(
  libremidi
  GIT_REPOSITORY https://github.com/celtera/libremidi.git
  GIT_TAG        v4.5.0
)
FetchContent_MakeAvailable(libremidi)

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

add_library(sls3_mcu_bridge_lib 
  package.cpp package.hpp
  client.cpp client.hpp
  bridge.cpp bridge.hpp
  mididevice.cpp mididevice.hpp)

target_link_libraries(sls3_mcu_bridge_lib PUBLIC asio libremidi fmt)

# Executable
add_executable(sls3_mcu_bridge)
target_sources(sls3_mcu_bridge PRIVATE main.cpp)
target_link_libraries(sls3_mcu_bridge PRIVATE sls3_mcu_bridge_lib)