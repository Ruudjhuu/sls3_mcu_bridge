#pragma once

#include "asio.hpp"
#include "package.hpp"
#include <cstddef>
#include <functional>
#include <memory>
#include <string>

const size_t MAX_BUFFER_SIZE = 1500;

namespace sls3mcubridge {
class Client : public std::enable_shared_from_this<Client> {
public:
  Client(asio::io_context &io_context)
      : io_context(io_context), socket(io_context) {}
  void connect(std::string const &host, int const &port);
  void write(std::vector<std::byte> &message);
  void start_reading(std::function<void(Package &)>);

private:
  void read_handler(const asio::error_code &error,
                    std::size_t bytes_transferred);
  asio::io_context &io_context;
  asio::ip::tcp::socket socket;
  std::function<void(Package &)> read_callback;
  std::byte buffer[MAX_BUFFER_SIZE];
};
} // namespace sls3mcubridge