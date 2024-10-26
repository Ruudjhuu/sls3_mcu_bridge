
#include <functional>
#include <memory>

#include "libremidi/libremidi.hpp"
#include "libremidi/message.hpp"
#include "spdlog/spdlog.h"

#include "mididevice.hpp"

namespace sls3mcubridge {

MidiDevice::MidiDevice(const std::string &name) : m_name(name), m_out({}) {
  m_out.open_virtual_port(m_name);
}

MidiDevice::~MidiDevice() {
  m_out.close_port();
  m_in->close_port();
}

void MidiDevice::start_reading(
    const std::function<void(int, const libremidi::message &)> &callback) {

  m_in = std::make_shared<libremidi::midi_in>(libremidi::input_configuration{
      .on_message =
          [callback](const libremidi::message &message) {
            callback(0, message);
            auto test = message.bytes;
          },
      .ignore_sysex = 0});
  m_in->open_virtual_port(m_name);
}

void MidiDevice::send_message(const libremidi::message &message) {
  m_out.send_message(message);
}

} // namespace sls3mcubridge