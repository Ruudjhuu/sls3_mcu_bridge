
#include "client.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <format>
#include <functional>
#include <spdlog/spdlog.h>
#include <string>
#include <sys/types.h>

#include "asio/buffer.hpp"
#include "asio/connect.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/placeholders.hpp"

#include "package.hpp"

namespace sls3mcubridge {
void Client::connect(const std::string &host, int const &port) {
  spdlog::info("Connecting to " + host + ":" + std::to_string(port));
  asio::ip::tcp::resolver resolver(m_socket.get_executor());
  asio::ip::tcp::resolver::iterator endpoint = resolver.resolve(
      asio::ip::tcp::resolver::query(host, std::to_string(port)));
  asio::connect(this->m_socket, endpoint);
  spdlog::info("Connected succesfully");
}

void Client::write(const asio::const_buffer &message) {
  try {
    m_socket.send(asio::buffer(message));
  } catch (const std::exception &exc) {
    spdlog::warn("Failed to send tcp message: " + std::string(exc.what()));
  }
}

void Client::start_reading(
    const std::function<void(tcp::Package &)> &callback) {
  m_read_callback = callback;
  m_socket.async_read_some(asio::buffer(m_buffer2),
                           std::bind(&Client::read_handler, shared_from_this(),
                                     asio::placeholders::error,
                                     asio::placeholders::bytes_transferred));
}

void Client::read_handler(const asio::error_code &error,
                          size_t bytes_transferred) {
  if (!error) {
    spdlog::info("handle message");
    try {
      size_t bytes_read = 0;
      while (bytes_read < bytes_transferred) {
        auto package = tcp::Package(tcp::BufferView(
            m_buffer2.begin(), (m_buffer2.begin() + bytes_transferred)));

        m_read_callback(package);
        bytes_read += package.get_size();
      }
    } catch (const std::exception &exc) {
      spdlog::warn(std::format("TCP read parse failure: {}", exc.what()));
    }
  } else {
    spdlog::error("failed to read incomming TCP message");
  }
  start_reading(m_read_callback);
}
} // namespace sls3mcubridge
