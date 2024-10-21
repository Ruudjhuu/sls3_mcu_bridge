#include "mididevice.hpp"
#include "libremidi/input_configuration.hpp"
#include "libremidi/libremidi.hpp"
#include <cstddef>
#include <functional>
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>

namespace sls3mcubridge {

MidiDevice::MidiDevice(std::string name) : m_name(name), m_out() {
  m_out.open_virtual_port(m_name);
}

void MidiDevice::start_reading(
    std::function<void(int, const libremidi::message &)> callback) {

  m_in = std::make_shared<libremidi::midi_in>(libremidi::input_configuration{
      .on_message =
          [callback](const libremidi::message &message) {
            callback(0, message);
            auto test = message.bytes;
          },
      .ignore_sysex = false});
  m_in->open_virtual_port(m_name);
}

void MidiDevice::send_message(std::vector<std::byte> message) {
  std::vector<unsigned char> char_mesage;
  for (auto &it : message) {
    char_mesage.push_back((unsigned char)it);
  }
  m_out.send_message(char_mesage);
}

} // namespace sls3mcubridge