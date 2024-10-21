#pragma once

#include "client.hpp"
#include "mididevice.hpp"
#include <boost/asio/io_context.hpp>
#include <memory>
#include <vector>

namespace sls3mcubridge {

class Bridge : public std::enable_shared_from_this<Bridge> {
public:
  Bridge(boost::asio::io_context &io_context, std::string ip, int port);
  void start();

private:
  void send_init_messages();
  void handle_tcp_read(Package &package);
  void handle_midi_read(int device_index, const libremidi::message &message);
  boost::asio::io_context &io_context;
  std::shared_ptr<Client> tcp_client;
  std::vector<std::shared_ptr<MidiDevice>> midi_devices;
}; // namespace sls3mcubridge

} // namespace sls3mcubridge