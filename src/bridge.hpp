#pragma once

#include "client.hpp"
#include <boost/asio/io_context.hpp>
#include <memory>
#include <rtmidi/RtMidi.h>

namespace sls3mcubridge {
class Bridge : public std::enable_shared_from_this<Bridge> {
public:
  Bridge(boost::asio::io_context &io_context, std::string ip, int port);
  void start();

private:
  void send_init_messages();
  void handle_read(Package package);
  std::shared_ptr<Client> tcp_client;
  RtMidiOut midi_main;
};
} // namespace sls3mcubridge