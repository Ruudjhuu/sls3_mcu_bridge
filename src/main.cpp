#include <memory>
#include <string_view>

#include "asio/io_context.hpp"

#include "bridge.hpp"

const int PORT = 53000;
const std::string_view HOST = "10.3.141.56";

int main() {

  asio::io_context io_context;

  auto bridge = std::make_shared<sls3mcubridge::Bridge>(
      io_context, std::string(HOST), PORT);
  bridge->start();

  io_context.run();
}