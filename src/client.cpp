
#include <cstddef>
#include <exception>
#include <format>
#include <functional>
#include <spdlog/spdlog.h>
#include <string>

#include "asio.hpp" // IWYU pragma: export
#include "client.hpp"
#include "package.hpp"

namespace sls3mcubridge {
void Client::connect(std::string const &host, int const &port) {
  spdlog::info("Connecting to " + host + ":" + std::to_string(port));
  asio::ip::tcp::resolver resolver(io_context);
  asio::ip::tcp::resolver::iterator endpoint = resolver.resolve(
      asio::ip::tcp::resolver::query(host, std::to_string(port)));
  asio::connect(this->socket, endpoint);
  spdlog::info("Connected succesfully");
}

void Client::write(std::vector<std::byte> &message) {
  try {
    socket.send(asio::buffer(message));
  } catch (const std::exception &exc) {
    spdlog::warn("Failed to send tcp message: " + std::string(exc.what()));
  }
}

void Client::start_reading(std::function<void(Package &)> callback) {
  read_callback = callback;
  socket.async_read_some(asio::buffer(buffer),
                         std::bind(&Client::read_handler, shared_from_this(),
                                   asio::placeholders::error,
                                   asio::placeholders::bytes_transferred));
}

void Client::read_handler(const asio::error_code &error,
                          std::size_t bytes_transferred) {
  if (!error) {
    spdlog::info("handle message");
    try {
      int bytes_read = 0;
      while (bytes_read < bytes_transferred) {
        auto package = Package(buffer, bytes_read);
        read_callback(package);
      }
    } catch (const std::exception &exc) {
      spdlog::warn(std::format("TCP read parse failure: {}", exc.what()));
    }
  } else {
    spdlog::error("failed to read incomming TCP message");
  }
  start_reading(read_callback);
}
} // namespace sls3mcubridge
