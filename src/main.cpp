#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/system/detail/errc.hpp>
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

  std::vector<unsigned char> data = {0x55, 0x43, 0x00, 0x01, 0x0a, 0x00,
                                     0x45, 0x51, 0x64, 0x00, 0x65, 0x00,
                                     0x4d, 0x49, 0x44, 0x49};
  client->send(data);

  // TODO try to get number of midi devices from response

  data = {0x55, 0x43, 0x00, 0x01, 0x0e, 0x00, 0x42, 0x4f, 0x6c, 0x00,
          0x67, 0x00, 0x4d, 0x69, 0x64, 0x63, 0x00, 0x00, 0x00, 0x00,
          0x55, 0x43, 0x00, 0x01, 0x0e, 0x00, 0x42, 0x4f, 0x6d, 0x00,
          0x68, 0x00, 0x4d, 0x69, 0x64, 0x63, 0x00, 0x00, 0x00, 0x00,
          0x55, 0x43, 0x00, 0x01, 0x0e, 0x00, 0x42, 0x4f, 0x6e, 0x00,
          0x69, 0x00, 0x4d, 0x69, 0x64, 0x63, 0x00, 0x00, 0x00, 0x00};

  client->send(data);

  client->start_reading();

  io_context.run();
}