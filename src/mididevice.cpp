
#include <functional>
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>

#include "libremidi/libremidi.hpp"
#include "libremidi/message.hpp"

#include "mididevice.hpp"

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

void MidiDevice::send_message(libremidi::message message) {
  m_out.send_message(message);
}

} // namespace sls3mcubridge