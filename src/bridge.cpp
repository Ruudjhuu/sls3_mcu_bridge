#include "bridge.hpp"
#include <boost/asio.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "client.hpp"
#include "package.hpp"
#include "rtmidi/RtMidi.h"
#include "spdlog/spdlog.h"

namespace sls3mcubridge {

Bridge::Bridge(boost::asio::io_context &io_context, std::string ip, int port)
    : io_context(io_context), tcp_client(std::make_shared<Client>(io_context)) {
  tcp_client->connect(ip, port);
  send_init_messages();

  midi_devices.push_back(
      std::make_shared<MidiDevice>("MAIN", RtMidi::Api::LINUX_ALSA));
}

void Bridge::start() {
  tcp_client->start_reading(std::bind(
      &Bridge::handle_tcp_read, shared_from_this(), std::placeholders::_1));

  for (auto &it : midi_devices)
    it->start_reading(std::bind(&Bridge::handle_midi_read, shared_from_this(),
                                0, std::placeholders::_2));
}

void Bridge::send_init_messages() {
  std::vector<std::byte> data = {
      std::byte{0x55}, std::byte{0x43}, std::byte{0x00}, std::byte{0x01},
      std::byte{0x0a}, std::byte{0x00}, std::byte{0x45}, std::byte{0x51},
      std::byte{0x64}, std::byte{0x00}, std::byte{0x65}, std::byte{0x00},
      std::byte{0x4d}, std::byte{0x49}, std::byte{0x44}, std::byte{0x49}};
  tcp_client->write(data);

  // TODO try to get number of midi devices from response

  data = {std::byte{0x55}, std::byte{0x43}, std::byte{0x00}, std::byte{0x01},
          std::byte{0x0e}, std::byte{0x00}, std::byte{0x42}, std::byte{0x4f},
          std::byte{0x6c}, std::byte{0x00}, std::byte{0x67}, std::byte{0x00},
          std::byte{0x4d}, std::byte{0x69}, std::byte{0x64}, std::byte{0x63},
          std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
          std::byte{0x55}, std::byte{0x43}, std::byte{0x00}, std::byte{0x01},
          std::byte{0x0e}, std::byte{0x00}, std::byte{0x42}, std::byte{0x4f},
          std::byte{0x6d}, std::byte{0x00}, std::byte{0x68}, std::byte{0x00},
          std::byte{0x4d}, std::byte{0x69}, std::byte{0x64}, std::byte{0x63},
          std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
          std::byte{0x55}, std::byte{0x43}, std::byte{0x00}, std::byte{0x01},
          std::byte{0x0e}, std::byte{0x00}, std::byte{0x42}, std::byte{0x4f},
          std::byte{0x6e}, std::byte{0x00}, std::byte{0x69}, std::byte{0x00},
          std::byte{0x4d}, std::byte{0x69}, std::byte{0x64}, std::byte{0x63},
          std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}};

  tcp_client->write(data);
}

void Bridge::handle_tcp_read(Package &package) {
  std::cout << "Bridge handle read" << std::endl;
  switch (package.get_body()->get_type()) {
  case Body::Type::Midi: {
    auto midi_body = std::dynamic_pointer_cast<MidiBody>(package.get_body());
    midi_devices.at(midi_body->get_device_index())
        ->send_message(midi_body->get_message());
    break;
  }
  case Body::Type::Unkown: {
    spdlog::warn("Ignored unkown package");
  }
  }
}

void Bridge::handle_midi_read(int device_index,
                              std::vector<std::byte> message) {
  spdlog::info("midi handler");
}

} // namespace sls3mcubridge