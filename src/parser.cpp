#include "parser.hpp"
#include <cstddef>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vector>

namespace sls3mcubridge {

const int DEVICE_BYTE_LOCATION = 10;

const std::byte INPUT_DEVICE_ONE_BYTE = std::byte{0x6c};
const std::byte INPUT_DEVICE_TWO_BYTE = std::byte{0x6d};
const std::byte INPUT_DEVICE_THREE_BYTE = std::byte{0x6e};

const std::byte DELIMITER = std::byte(0x0);

const int TCP_MIDI_HEADER_SIZE = 12;
const int TCP_MIN_MESSAGE_SIZE = 14;

std::vector<Package> Parser::serialize(std::byte input[], size_t nr_bytes) {
  int step = 0;
  int bytes_read = 0;
  bool error = false;

  std::vector<Package> packages;
  Package tmp_package;

  for (int i = 0; i < nr_bytes; i++) {
    const auto byte = input[i];
    bytes_read++;

    switch (step) {
    // Header UC
    case 0:
      if (byte != std::byte{'U'}) {
        error = true;
        continue;
      }
      step++;
      break;
    case 1:
      if (byte != std::byte{'C'}) {
        error = true;
        continue;
      }
      step++;
      break;

    // Delimiter
    case 2:
      if (byte != DELIMITER) {
        error = true;
        continue;
      }
      step++;
      break;
    // unkown
    case 3 ... 9:
      step++;
      break;

    // midi device
    case 10:
      switch (byte) {
      case INPUT_DEVICE_ONE_BYTE:
        tmp_package.midi_dev = 1;
        break;

      case INPUT_DEVICE_TWO_BYTE:
        tmp_package.midi_dev = 2;
        break;

      case INPUT_DEVICE_THREE_BYTE:
        tmp_package.midi_dev = 3;
        break;

      default:
        spdlog::error("Unkown midi device");
        throw std::runtime_error(
            "Unkown midi device number while parcing TCP message");
        break;
      }
      step++;
      break;

    case 11:
      if (byte != DELIMITER) {
        error = true;
        continue;
      }
      step++;
      break;

    // get body
    case 12:
      if (byte != DELIMITER) {
        tmp_package.body.push_back(byte);
        continue;
      }
      packages.push_back(tmp_package);
      break;
    }
  }
  return packages;
}

} // namespace sls3mcubridge