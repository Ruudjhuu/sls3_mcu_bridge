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

  cxxopts::ParseResult parse_result;

  try {
    options.add_options()("host",
                          "hostname or ip-address of the mixer to connect to.",
                          cxxopts::value<std::string>())(
        "v,verbose", "info level logging.", cxxopts::value<bool>());
    options.parse_positional({"host"});
    options.positional_help("host");
    parse_result = options.parse(argc, argv);
  } catch (const std::exception &exc) {
    std::cout << exc.what() << "\n" << "\n";
    std::cout << options.help() << "\n";
    return -1;
  }

  if (parse_result["host"].count() == 0) {
    std::cout << "host is mandetory." << "\n" << "\n";
    std::cout << options.help() << "\n";
    return -1;
  }

  spdlog::set_level(spdlog::level::info);
  if (parse_result["verbose"].count() > 0) {
    if (parse_result["verbose"].as<bool>()) {
      spdlog::set_level(spdlog::level::debug);
    }
  }

  asio::io_context io_context;

  try {
    // TODO(ruud): remove the use of shared pointer if possible. Currently it is
    // needed to support shared_from_this inside the Bridge class
    auto bridge = std::make_shared<sls3mcubridge::Bridge>(
        io_context, parse_result["host"].as<std::string>(), PORT);
    bridge->start();
  } catch (std::exception &exc) {
    spdlog::error("Failed to start bridge, exiting: " +
                  std::string(exc.what()));
    return -1;
  }

  try {
    io_context.run();
  } catch (std::exception &exc) {
    spdlog::error("Issue occured with async runner: " +
                  std::string(exc.what()));
  }
}