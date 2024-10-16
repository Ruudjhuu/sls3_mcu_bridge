#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/system/detail/errc.hpp>
#include <boost/system/detail/error_code.hpp>
#include <cstddef>
#include <memory>
#include <string>

const size_t MAX_BUFFER_SIZE = 1500;

namespace sls3mcubridge {
class Client : public std::enable_shared_from_this<Client> {
public:
  Client(boost::asio::io_context &svc) : io_context(svc), socket(io_context) {}
  void connect(std::string const &host, int const &port);
  void write(std::vector<std::byte> &message);
  void start_reading();

private:
  void read_handler(const boost::system::error_code &error,
                    std::size_t bytes_transferred);
  boost::asio::io_context &io_context;
  boost::asio::ip::tcp::socket socket;
  std::byte buffer[MAX_BUFFER_SIZE];
};
} // namespace sls3mcubridge