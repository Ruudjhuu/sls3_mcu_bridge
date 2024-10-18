#include <memory>
#include <string>

#include "spdlog/spdlog.h"

#include "bridge.hpp"

using namespace sls3mcubridge;

const int PORT = 53000;
const std::string HOST = "10.3.141.56";

int main() {
  spdlog::info("Starting sls3_mcu_bridge");

  boost::asio::io_context io_context;

  auto bridge = std::make_shared<Bridge>(io_context, HOST, PORT);
  bridge->start();

  io_context.run();
}