#include "bridge.hpp"

#include <chrono>
#include <cstddef>
#include <ios>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "libremidi/message.hpp"
#include "spdlog/spdlog.h"

#include "package.hpp"

namespace sls3mcubridge {

Bridge::Bridge(asio::io_context &io_context, std::string ip, int port)
    : io_context(io_context), tcp_client(std::make_shared<Client>(io_context)) {
  tcp_client->connect(ip, port);
  send_init_messages();

  midi_devices.push_back(std::make_shared<MidiDevice>("MAIN"));
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  midi_devices.push_back(std::make_shared<MidiDevice>("EXT1"));
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  midi_devices.push_back(std::make_shared<MidiDevice>("EXT2"));
}

void Bridge::start() {
  tcp_client->start_reading(std::bind(
      &Bridge::handle_tcp_read, shared_from_this(), std::placeholders::_1));

  for (int i = 0; i < midi_devices.size(); i++) {
    midi_devices.at(i)->start_reading(std::bind(&Bridge::handle_midi_read,
                                                shared_from_this(), i,
                                                std::placeholders::_2));
  }
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

void Bridge::handle_tcp_read(tcp::Package &package) {
  std::cout << "Bridge handle read" << std::endl;
  switch (package.get_body()->get_type()) {
  case tcp::Body::Type::IncommingMidi: {
    auto midi_body =
        std::dynamic_pointer_cast<tcp::IncommingMidiBody>(package.get_body());
    midi_devices.at(midi_body->get_device_index())
        ->send_message(midi_body->get_message());
    break;
  }
  case tcp::Body::Type::Unkown:
  default: {
    spdlog::warn("Ignored unkown package");
  }
  }
}

void Bridge::handle_midi_read(int device_index,
                              const libremidi::message &message) {
  std::cout << device_index << std::endl;
  std::stringstream substring;
  substring << "type: " << std::hex << std::setw(2) << std::setfill('0')
            << (int)message.get_message_type();

  for (auto &it : message) {
    substring << " " << std::setw(2) << std::setfill('0') << (int)it;
  }

  spdlog::info("midi handler. message.size: " + std::to_string(message.size()) +
               ", " + ": " + substring.str());

  auto tmp_device = std::byte(0);
  switch (device_index) {
  case 0:
    tmp_device = std::byte(0x67);
    break;
  case 1:
    tmp_device = std::byte(0x68);
    break;
  case 2:
    tmp_device = std::byte(0x69);
    break;
  default:
    spdlog::warn("Bridge midi read tries to handle unsupported device index");
    break;
  }

  std::shared_ptr<tcp::Body> body;
  switch (message.get_message_type()) {
  case libremidi::message_type::NOTE_OFF:
  case libremidi::message_type::NOTE_ON:
  case libremidi::message_type::POLY_PRESSURE:
  case libremidi::message_type::PROGRAM_CHANGE:
  case libremidi::message_type::CONTROL_CHANGE:
  case libremidi::message_type::AFTERTOUCH:
  case libremidi::message_type::PITCH_BEND: {
    auto tmp_list = {message};
    body = std::make_shared<tcp::OutgoingMidiBody>(tmp_device, tmp_list);
    break;
  }
  case libremidi::message_type::SYSTEM_EXCLUSIVE:
    body = std::make_shared<tcp::SysExMidiBody>(tmp_device, message);
    break;
  default:
    spdlog::warn("Recieved unkown midi message");
    break;
  }

  tcp::Package tcp_message(body);
  auto bytes = tcp_message.serialize();
  tcp_client->write(bytes);
}

} // namespace sls3mcubridge