#pragma once

#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/system/detail/errc.hpp>
#include <cstddef>
#include <memory>
#include <string>

#include "parser.hpp"

namespace sls3mcubridge {
class Client : public std::enable_shared_from_this<Client> {
public:
  Client(boost::asio::io_context &svc) : io_context(svc), socket(io_context) {}

  void connect(std::string const &host, int const &port);

  void send(std::vector<unsigned char> &message);
  void start_reading();

private:
  void read_handler(const boost::system::error_code &error,
                    std::size_t bytes_transferred);
  boost::asio::io_context &io_context;
  boost::asio::ip::tcp::socket socket;
  Parser parser;
  unsigned char buffer[128];
};
} // namespace sls3mcubridge