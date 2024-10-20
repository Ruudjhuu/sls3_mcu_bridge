#include "mididevice.hpp"
#include <cstddef>
#include <functional>
#include <spdlog/spdlog.h>
#include <vector>

namespace sls3mcubridge {

MidiDevice::MidiDevice(std::string name, RtMidi::Api api)
    : m_name(name), m_in(api, name), m_out(api, name) {
  m_out.openVirtualPort(m_name);

  // don't igore sysex
  m_in.ignoreTypes(false, true, true);
}

void MidiDevice::start_reading(
    std::function<void(int, std::vector<std::byte>)> callback) {
  m_read_callback = callback;

  void *context_obj = this;
  m_in.setCallback(&MidiDevice::handle_midi_read_static, context_obj);

  m_in.openVirtualPort(m_name);
}

void MidiDevice::handle_midi_read_static(double timeStamp,
                                         std::vector<unsigned char> *message,
                                         void *userData) {
  auto object = static_cast<MidiDevice *>(userData);
  object->handle_midi_read(timeStamp, message);
}

void MidiDevice::handle_midi_read(double timeStamp,
                                  std::vector<unsigned char> *message) {
  std::vector<std::byte> byte_message;
  for (auto &it : *message) {
    byte_message.push_back(std::byte(it));
  }
  m_read_callback(0, byte_message);
}

void MidiDevice::send_message(std::vector<std::byte> message) {
  std::vector<unsigned char> char_mesage;
  for (auto &it : message) {
    char_mesage.push_back((unsigned char)it);
  }

  m_out.sendMessage(&char_mesage);
}

} // namespace sls3mcubridge