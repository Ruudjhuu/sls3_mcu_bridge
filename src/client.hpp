#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <string>

#include "asio/buffer.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"

#include "package.hpp"

const size_t MAX_BUFFER_SIZE = 1500;

namespace sls3mcubridge {
class Client : public std::enable_shared_from_this<Client> {
public:
  explicit Client(asio::io_context &io_context) : m_socket(io_context) {}
  void connect(std::string const &host, int const &port);
  void write(const asio::const_buffer &message);
  void start_reading(const std::function<void(tcp::Package &)> &callback);

private:
  void read_handler(const asio::error_code &error,
                    std::size_t bytes_transferred);
  asio::ip::tcp::socket m_socket;
  std::function<void(tcp::Package &)> m_read_callback;
  std::byte m_buffer[MAX_BUFFER_SIZE]{};
};
} // namespace sls3mcubridge