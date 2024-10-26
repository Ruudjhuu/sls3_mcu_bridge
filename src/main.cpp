#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>

#include "asio/io_context.hpp"
#include "cxxopts.hpp"
#include "spdlog/spdlog.h"

#include "bridge.hpp"

const int PORT = 53000;

int main(int argc, char **argv) {

  cxxopts::Options options(
      "sls3_mcu_bridge",
      "Bridge between Presonus Studio Live Series 3 mixer and DAW");

  options.add_options()("host",
                        "hostname or ip-address of the mixer to connect to.",
                        cxxopts::value<std::string>())(
      "v,verbose", "info level logging.", cxxopts::value<bool>());

  cxxopts::ParseResult parse_result;
  try {
    options.parse_positional({"host"});
    options.positional_help("host");
    parse_result = options.parse(argc, argv);
  } catch (const std::exception &exc) {
    std::cout << exc.what() << std::endl << std::endl;
    std::cout << options.help() << std::endl;
    return -1;
  }

  if (parse_result["host"].count() == 0) {
    std::cout << "host is mandetory." << std::endl << std::endl;
    std::cout << options.help() << std::endl;
    return -1;
  }

  spdlog::set_level(spdlog::level::info);
  if (parse_result["verbose"].count() > 0) {
    if (parse_result["verbose"].as<bool>()) {
      spdlog::set_level(spdlog::level::debug);
    }
  }
  asio::io_context io_context;

  // TODO(ruud): remove the use of bind and shared_from_this in brdige so it
  // does not have to be a shared pointer
  auto bridge = std::make_shared<sls3mcubridge::Bridge>(
      io_context, parse_result["host"].as<std::string>(), PORT);
  bridge->start();

  io_context.run();
}