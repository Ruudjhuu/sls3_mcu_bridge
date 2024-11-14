#pragma once

#include "asio/io_context.hpp"
#include "libremidi/message.hpp"

#include <memory>
#include <string>
#include <vector>

namespace sls3mcubridge {
class MidiDevice;
class Client;
namespace tcp {
class Package;
} // namespace tcp

class Bridge : public std::enable_shared_from_this<Bridge> {
public:
  Bridge(asio::io_context &io_context, const std::string &ip_address, int port);
  void start();

private:
  void send_init_messages();
  void handle_tcp_read(tcp::Package &package);
  void handle_midi_read(int device_index, const libremidi::message &message);
  std::shared_ptr<Client> tcp_client;
  std::vector<std::shared_ptr<MidiDevice>> midi_devices;
}; // namespace sls3mcubridge

} // namespace sls3mcubridge