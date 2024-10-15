add_library(sls3_mcu_bridge_lib parser.cpp parser.hpp client.cpp client.hpp)
target_link_libraries(sls3_mcu_bridge_lib PRIVATE fmt)

add_executable(sls3_mcu_bridge)
target_sources(sls3_mcu_bridge PRIVATE main.cpp)

target_link_libraries(sls3_mcu_bridge PRIVATE sls3_mcu_bridge_lib rtmidi spdlog fmt)