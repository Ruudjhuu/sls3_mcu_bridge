
#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include "libremidi/error.hpp"
#include "libremidi/libremidi.hpp"
#include "libremidi/message.hpp"
#include "spdlog/spdlog.h"

#include "mididevice.hpp"

namespace sls3mcubridge {

MidiDevice::MidiDevice(std::string name) : m_name(std::move(name)) {
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
          [callback, this](const libremidi::message &message) {
            if (!this->m_received_first_message) {
              spdlog::info(this->m_name + " accepted connection.");
              this->m_received_first_message = true;
            }
            callback(0, message);
            auto test = message.bytes;
          },
      .on_error =
          [](libremidi::midi_error error, std::string_view str) {
            spdlog::error("Midi error: " + std::to_string(error) + ": " +
                          std::string(str));
          },
      .on_warning =
          [](libremidi::midi_error error, std::string_view str) {
            spdlog::warn("Midi warning: " + std::to_string(error) + ": " +
                         std::string(str));
          },
      .ignore_sysex = 0} // namespace sls3mcubridge
  );
  m_in->open_virtual_port(m_name);
}

void MidiDevice::send_message(const libremidi::message &message) {
  m_out.send_message(message);
}

} // namespace sls3mcubridge