#include <boost/asio/connect.hpp>
#include <cstddef>
#include <functional>
#include <spdlog/spdlog.h>

#include "client.hpp"
#include "new_package.hpp"

namespace sls3mcubridge {
void Client::connect(std::string const &host, int const &port) {
  boost::asio::ip::tcp::resolver resolver(io_context);
  boost::asio::ip::tcp::resolver::iterator endpoint = resolver.resolve(
      boost::asio::ip::tcp::resolver::query(host, std::to_string(port)));
  boost::asio::connect(this->socket, endpoint);
}

void Client::write(std::vector<std::byte> &message) {
  socket.send(boost::asio::buffer(message));
}

void Client::start_reading(std::function<void(Package &)> callback) {
  read_callback = callback;
  socket.async_read_some(
      boost::asio::buffer(buffer),
      std::bind(&Client::read_handler, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
}

void Client::read_handler(const boost::system::error_code &error,
                          std::size_t bytes_transferred) {
  if (error.value() == boost::system::errc::success) {
    spdlog::info("handle message");
    try {
      int bytes_read = 0;
      while (bytes_read < bytes_transferred) {
        auto package = Package(buffer, bytes_read);
        read_callback(package);
      }
    } catch (...) {
      // ignore error and move on to next message.
      spdlog::warn("ignore parse failure.");
    }
  } else {
    spdlog::error("failed to read incomming TCP message");
  }
  start_reading(read_callback);
}
} // namespace sls3mcubridge
