#include <memory>
#include <string>

#include "rtmidi/RtMidi.h"
#include "spdlog/spdlog.h"

#include "client.hpp"

using namespace sls3mcubridge;

const int PORT = 53000;
const std::string HOST = "10.3.141.56";

int main() {
  spdlog::info("test logging");
  boost::asio::io_context io_context;

  RtMidiOut *midiout_main = new RtMidiOut(RtMidi::Api::LINUX_ALSA, "MAIN");
  midiout_main->openVirtualPort("MAIN");

  RtMidiIn *midiin_main = new RtMidiIn(RtMidi::Api::LINUX_ALSA, "MAIN");
  midiin_main->openVirtualPort("MAIN");

  auto client = std::make_shared<Client>(io_context);
  client->connect(HOST, PORT);

  std::vector<std::byte> data = {
      std::byte{0x55}, std::byte{0x43}, std::byte{0x00}, std::byte{0x01},
      std::byte{0x0a}, std::byte{0x00}, std::byte{0x45}, std::byte{0x51},
      std::byte{0x64}, std::byte{0x00}, std::byte{0x65}, std::byte{0x00},
      std::byte{0x4d}, std::byte{0x49}, std::byte{0x44}, std::byte{0x49}};
  client->write(data);

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

  client->write(data);

  client->start_reading();

  io_context.run();
}