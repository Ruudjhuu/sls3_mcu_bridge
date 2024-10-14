add_executable(sls3_mcu_bridge)
target_sources(sls3_mcu_bridge PRIVATE main.cpp parser.cpp parser.hpp client.cpp client.hpp)

target_link_libraries(sls3_mcu_bridge PRIVATE rtmidi spdlog fmt)