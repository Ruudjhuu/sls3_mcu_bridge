#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <string>

#include "asio/buffer.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"

namespace sls3mcubridge {
namespace tcp {
class Package;
} // namespace tcp

const size_t MAX_BUFFER_SIZE = 1500;
class Client : public std::enable_shared_from_this<Client> {
public:
  explicit Client(asio::io_context &io_context) : m_socket(io_context) {}
  void connect(std::string const &host, int const &port);
  void write(const asio::const_buffer &message);
  size_t read_some(const asio::mutable_buffers_1 &buffer) {
    return m_socket.read_some(buffer);
  }
  void start_reading(const std::function<void(tcp::Package &)> &callback);

private:
  void read_handler(const asio::error_code &error,
                    std::size_t bytes_transferred);
  asio::ip::tcp::socket m_socket;
  std::function<void(tcp::Package &)> m_read_callback;
  std::array<std::byte, MAX_BUFFER_SIZE> m_buffer2{};
};
} // namespace sls3mcubridge