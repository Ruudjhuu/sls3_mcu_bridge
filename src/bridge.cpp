#include "bridge.hpp"

#include <array>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "asio/buffer.hpp"
#include "libremidi/message.hpp"
#include "spdlog/spdlog.h"

#include "package.hpp"

const int DELAY_BETWEEN_MIDI_DEVICE_CREATION_MS = 100;

namespace sls3mcubridge {

const std::array<std::byte, 16> FIRST_INIT_MESSAGE = {
    std::byte{0x55}, std::byte{0x43}, std::byte{0x00}, std::byte{0x01},
    std::byte{0x0a}, std::byte{0x00}, std::byte{0x45}, std::byte{0x51},
    std::byte{0x64}, std::byte{0x00}, std::byte{0x65}, std::byte{0x00},
    std::byte{0x4d}, std::byte{0x49}, std::byte{0x44}, std::byte{0x49}};

const std::array<std::byte, 60> SECOND_INIT_MESSAGE = {
    std::byte{0x55}, std::byte{0x43}, std::byte{0x00}, std::byte{0x01},
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

Bridge::Bridge(asio::io_context &io_context, const std::string &ip_address,
               int port)
    : tcp_client(std::make_shared<Client>(io_context)) {
  tcp_client->connect(ip_address, port);
  send_init_messages();

  midi_devices.push_back(std::make_shared<MidiDevice>("StudioLive_MAIN"));
  std::this_thread::sleep_for(
      std::chrono::milliseconds(DELAY_BETWEEN_MIDI_DEVICE_CREATION_MS));
  midi_devices.push_back(std::make_shared<MidiDevice>("StudioLive_EXT1"));
  std::this_thread::sleep_for(
      std::chrono::milliseconds(DELAY_BETWEEN_MIDI_DEVICE_CREATION_MS));
  midi_devices.push_back(std::make_shared<MidiDevice>("StudioLive_EXT2"));
}

void Bridge::start() {
  tcp_client->start_reading(std::bind(
      &Bridge::handle_tcp_read, shared_from_this(), std::placeholders::_1));

  for (size_t i = 0; i < midi_devices.size(); i++) {
    midi_devices.at(i)->start_reading(std::bind(&Bridge::handle_midi_read,
                                                shared_from_this(), i,
                                                std::placeholders::_2));
  }
}

void Bridge::send_init_messages() {
  tcp_client->write(asio::buffer(FIRST_INIT_MESSAGE));

  // TODO(ruud): try to get number of midi devices from response

  tcp_client->write(asio::buffer(SECOND_INIT_MESSAGE));
}

void Bridge::handle_tcp_read(tcp::Package &package) {

  spdlog::debug("Bridge handle read");
  switch (package.get_body()->get_type()) {
  case tcp::Body::Type::IncommingMidi: {
    auto midi_body =
        std::dynamic_pointer_cast<tcp::IncommingMidiBody>(package.get_body());
    midi_devices.at(midi_body->get_device_index())
        ->send_message(midi_body->get_message());
    break;
  }
  case tcp::Body::Type::OutgoingMidi:
    throw std::invalid_argument("Received unexpected package of type: " +
                                std::to_string(package.get_body()->get_type()));
  case tcp::Body::Type::SysEx: {
    auto midi_body =
        std::dynamic_pointer_cast<tcp::SysExMidiBody>(package.get_body());
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
  std::stringstream substring;
  substring << "type: " << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(message.get_message_type());

  for (const auto &iter : message) {
    substring << " " << std::setw(2) << std::setfill('0')
              << static_cast<int>(iter);
  }

  spdlog::debug("midi handler. message.size: " +
                std::to_string(message.size()) + ", " + ": " + substring.str());

  auto device_byte = tcp::Package::index_to_midi_device_byte(device_index);
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
    body = std::make_shared<tcp::OutgoingMidiBody>(device_byte, tmp_list);
    break;
  }
  case libremidi::message_type::SYSTEM_EXCLUSIVE:
    body = std::make_shared<tcp::SysExMidiBody>(device_byte, message);
    break;

  case libremidi::message_type::TIME_CODE:
  case libremidi::message_type::SONG_POS_POINTER:
  case libremidi::message_type::SONG_SELECT:
  case libremidi::message_type::RESERVED1:
  case libremidi::message_type::RESERVED2:
  case libremidi::message_type::TUNE_REQUEST:
  case libremidi::message_type::EOX:
  case libremidi::message_type::TIME_CLOCK:
  case libremidi::message_type::RESERVED3:
  case libremidi::message_type::START:
  case libremidi::message_type::CONTINUE:
  case libremidi::message_type::STOP:
  case libremidi::message_type::RESERVED4:
  case libremidi::message_type::ACTIVE_SENSING:
  case libremidi::message_type::SYSTEM_RESET:
  case libremidi::message_type::INVALID:
  default:
    spdlog::warn("Recieved unsuported midi message");
    break;
  }

  tcp::Package tcp_message(body);
  auto bytes = tcp_message.serialize();
  tcp_client->write(asio::buffer(bytes));
}

} // namespace sls3mcubridge