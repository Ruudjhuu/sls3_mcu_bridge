#include "mididevice.hpp"
#include <cstddef>
#include <functional>
#include <spdlog/spdlog.h>
#include <vector>

static void handle_midi_read_test(double timeStamp,
                                  std::vector<unsigned char> *message,
                                  void *userData) {

  // somehow message is not valid here
  spdlog::info("midi_handledr");
}

namespace sls3mcubridge {

// This is a hacky solution to convert member function pointers to c function
// pointers
union class_member_to_func_ptr {
  decltype(&MidiDevice::handle_midi_read) member_func;
  void (*c_func)(double timeStamp, std::vector<unsigned char> *message,
                 void *userData);
};

MidiDevice::MidiDevice(std::string name, RtMidi::Api api)
    : m_name(name), m_in(api, name), m_out(api, name) {
  m_out.openVirtualPort(m_name);
}

void MidiDevice::start_reading(
    std::function<void(int, std::vector<std::byte>)> callback) {
  m_read_callback = callback;
  class_member_to_func_ptr cb_hack;
  cb_hack.member_func = &MidiDevice::handle_midi_read;
  m_in.setCallback(cb_hack.c_func);

  m_in.openVirtualPort(m_name);
}

void MidiDevice::handle_midi_read(double timeStamp,
                                  std::vector<unsigned char> *message,
                                  void *userData) {

  // somehow message is not valid here
  spdlog::info("midi_handledr");

  // std::vector<std::byte> byte_message;
  // if (message->size() < 0) {
  //   spdlog::warn("Received empty midi message");
  //   return;
  // }
  // for (auto &it : *message) {
  //   byte_message.push_back(std::byte(it));
  // }
  // m_read_callback(0, byte_message);
}

void MidiDevice::send_message(std::vector<std::byte> message) {
  std::vector<unsigned char> char_mesage;
  for (auto &it : message) {
    char_mesage.push_back((unsigned char)it);
  }

  m_out.sendMessage(&char_mesage);
}

} // namespace sls3mcubridge