include(FetchContent)

FetchContent_Declare(
  libremidi
  GIT_REPOSITORY https://github.com/celtera/libremidi.git
  GIT_TAG        v4.5.0
)
FetchContent_MakeAvailable(libremidi)

add_library(sls3_mcu_bridge_lib 
 package.cpp package.hpp
 client.cpp client.hpp
 bridge.cpp bridge.hpp
 mididevice.cpp mididevice.hpp)


 
target_link_libraries(sls3_mcu_bridge_lib PRIVATE fmt libremidi)

add_executable(sls3_mcu_bridge)
target_sources(sls3_mcu_bridge PRIVATE main.cpp)

target_link_libraries(sls3_mcu_bridge PRIVATE sls3_mcu_bridge_lib spdlog fmt libremidi)