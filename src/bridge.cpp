#include "bridge.hpp"
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <memory>
#include <vector>

#include "client.hpp"
#include "rtmidi/RtMidi.h"

namespace sls3mcubridge {
Bridge::Bridge(boost::asio::io_context &io_context, std::string ip, int port)
    : tcp_client(std::make_shared<Client>(io_context)),
      midi_main(RtMidi::Api::LINUX_ALSA, "MAIN") {
  tcp_client->connect(ip, port);
  send_init_messages();

  midi_main.openVirtualPort("MAIN");
}

void Bridge::start() {
  tcp_client->start_reading(std::bind(&Bridge::handle_read, shared_from_this(),
                                      std::placeholders::_1));
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

void Bridge::handle_read(Package package) {
  std::cout << "Bridge handle read" << std::endl;
  auto message = package.body.midi_content.message;
  std::vector<unsigned char> char_mesage;
  for (auto &it : message) {
    char_mesage.push_back((unsigned char)it);
  }

  midi_main.sendMessage(&char_mesage);
}

} // namespace sls3mcubridge