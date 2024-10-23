#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <string>

#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"

#include "package.hpp"

const size_t MAX_BUFFER_SIZE = 1500;

namespace sls3mcubridge {
class Client : public std::enable_shared_from_this<Client> {
public:
  Client(asio::io_context &io_context)
      : io_context(io_context), socket(io_context) {}
  void connect(std::string const &host, int const &port);
  void write(std::vector<std::byte> &message);
  void start_reading(std::function<void(tcp::Package &)>);

private:
  void read_handler(const asio::error_code &error,
                    std::size_t bytes_transferred);
  asio::io_context &io_context;
  asio::ip::tcp::socket socket;
  std::function<void(tcp::Package &)> read_callback;
  std::byte buffer[MAX_BUFFER_SIZE];
};
} // namespace sls3mcubridge