#include "parser.hpp"
#include <cstddef>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace sls3mcubridge {

const int DEVICE_BYTE_LOCATION = 10;

const std::byte INPUT_DEVICE_ONE_BYTE = std::byte{0x6c};
const std::byte INPUT_DEVICE_TWO_BYTE = std::byte{0x6d};
const std::byte INPUT_DEVICE_THREE_BYTE = std::byte{0x6e};

const std::byte DELIMITER = std::byte(0x0);

const int TCP_MIDI_HEADER_SIZE = 12;
const int TCP_MIN_MESSAGE_SIZE = 14;

Package Package::deserialize(std::vector<std::byte> input) {
  int step = 0;
  bool error = false;

  Package package;
  package.header = Header::deserialize(input);

  for (auto &byte : input) {

    switch (step) {
    // midi device
    case 10:
      package.midi_dev = byte;
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
        package.body.push_back(byte);
        continue;
      }
      return package;
      break;
    }
  }
  throw std::invalid_argument("Could not deserialize");
}

std::vector<std::byte> Package::serialize(Package input) {
  std::vector<std::byte> buffer = Header::serialize(input.header);
  buffer.push_back(DELIMITER);
  buffer.push_back(DELIMITER);
  buffer.push_back(input.midi_dev);
  buffer.push_back(std::byte(DELIMITER));
  buffer.insert(buffer.end(), input.body.begin(), input.body.end());
  buffer.push_back(std::byte(DELIMITER));
  return buffer;
}

Header Header::deserialize(std::vector<std::byte> input) {
  int step = 0;
  Header header;

  for (auto &byte : input) {

    switch (step) {
    // Header UC
    case 0:
      if (byte != std::byte{'U'}) {
        throw std::invalid_argument("expected 'U' in step " +
                                    std::to_string(step) + " but got" +
                                    (char)byte);
      }
      header.firt_part[0] = byte;
      step++;
      break;
    case 1:
      if (byte != std::byte{'C'}) {
        throw std::invalid_argument("expected 'C' in step " +
                                    std::to_string(step) + " but got" +
                                    (char)byte);
      }
      header.firt_part[1] = byte;
      step++;
      break;

    // Delimiter
    case 2:
    case 5:
    case 8:
    case 9:
      if (byte != DELIMITER) {
        throw std::invalid_argument("Expected delimiter in step " +
                                    std::to_string(step));
      }
      step++;
      break;

    // unkown pt2
    case 3:
      header.second_part[0] = byte;
      step++;
      break;
    case 4:
      header.second_part[1] = byte;
      step++;
      break;

    // unkown pt3
    case 6:
      header.third_part[0] = byte;
      step++;
      break;
    case 7:
      header.third_part[1] = byte;
      step++;
      break;
    }
  }
  return header;
}

std::vector<std::byte> Header::serialize(Header input) {
  std::vector<std::byte> buffer;
  buffer.insert(buffer.end(), input.firt_part, input.firt_part + 2);
  buffer.push_back(DELIMITER);
  buffer.insert(buffer.end(), input.second_part, input.second_part + 2);
  buffer.push_back(DELIMITER);
  buffer.insert(buffer.end(), input.third_part, input.third_part + 2);
  buffer.push_back(DELIMITER);
  buffer.push_back(DELIMITER);
  return buffer;
}

} // namespace sls3mcubridge