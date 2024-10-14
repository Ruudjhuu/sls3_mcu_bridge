#include "rtmidi/RtMidi.h"
#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/system/detail/errc.hpp>
#include <cstddef>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

const int PORT = 53000;
const std::string HOST = "10.3.141.56";

class Client : public std::enable_shared_from_this<Client> {
public:
  Client(boost::asio::io_context &svc) : io_context(svc), socket(io_context) {}

  void connect(std::string const &host, int const &port) {
    boost::asio::ip::tcp::resolver resolver(io_context);
    boost::asio::ip::tcp::resolver::iterator endpoint = resolver.resolve(
        boost::asio::ip::tcp::resolver::query(host, std::to_string(port)));
    boost::asio::connect(this->socket, endpoint);
  }

  void send(std::vector<unsigned char> &message) {
    socket.send(boost::asio::buffer(message));
  }

  void start_reading() {
    socket.async_read_some(
        boost::asio::buffer(buffer),
        std::bind(&Client::read_handler, shared_from_this(),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
  }

  boost::asio::io_context &io_context;
  boost::asio::ip::tcp::socket socket;
  unsigned char buffer[128];

  void read_handler(const boost::system::error_code &error,
                    std::size_t bytes_transferred) {
    if (error.value() == boost::system::errc::success) {
      for (int i = 0; i < bytes_transferred; i++) {
        std::cout << std::hex << (int)buffer[i] << " ";
      }
      std::cout << std::endl;
    } else {
      std::cout << "Error reading some, ignore and go on" << std::endl;
    }
    start_reading();
  }
};

int main() {
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