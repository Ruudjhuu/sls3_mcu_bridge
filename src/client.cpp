#include <spdlog/spdlog.h>

#include "client.hpp"

namespace sls3mcubridge {
void Client::connect(std::string const &host, int const &port) {
  boost::asio::ip::tcp::resolver resolver(io_context);
  boost::asio::ip::tcp::resolver::iterator endpoint = resolver.resolve(
      boost::asio::ip::tcp::resolver::query(host, std::to_string(port)));
  boost::asio::connect(this->socket, endpoint);
}

void Client::send(std::vector<unsigned char> &message) {
  socket.send(boost::asio::buffer(message));
}

void Client::start_reading() {
  socket.async_read_some(
      boost::asio::buffer(buffer),
      std::bind(&Client::read_handler, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
}

void Client::read_handler(const boost::system::error_code &error,
                          std::size_t bytes_transferred) {
  if (error.value() == boost::system::errc::success) {
    parser.to_midi(buffer);
  } else {
    spdlog::error("failed to read incomming TCP message");
  }
  start_reading();
}
} // namespace sls3mcubridge
