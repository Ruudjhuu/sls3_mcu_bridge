#include "package.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace sls3mcubridge {

const std::byte DELIMITER = std::byte(0x0);

Package Package::deserialize(std::vector<std::byte> input,
                             std::vector<std::byte>::iterator &position) {
  int step = 0;
  bool error = false;

  Package package;
  package.header = Header::deserialize(input, position);

  for (int i = 0; i < package.header.body_size; i++) {
    package.body.push_back(*position);
    position++;
  };
  return package;
}

std::vector<std::byte> Package::serialize(Package input) {
  std::vector<std::byte> buffer = Header::serialize(input.header);
  buffer.insert(buffer.end(), input.body.begin(), input.body.end());
  return buffer;
}

Header Header::deserialize(std::vector<std::byte> input,
                           std::vector<std::byte>::iterator &position) {
  bool break_loop = false;
  int step = 0;
  Header header;

  for (; position != input.end(); position++) {

    switch (step) {
    // Header UC
    case 0:
      if (*position != std::byte{'U'}) {
        throw std::invalid_argument("expected 'U' in step " +
                                    std::to_string(step) + " but got" +
                                    (char)*position);
      }
      header.firt_part[0] = *position;
      step++;
      break;
    case 1:
      if (*position != std::byte{'C'}) {
        throw std::invalid_argument("expected 'C' in step " +
                                    std::to_string(step) + " but got" +
                                    (char)*position);
      }
      header.firt_part[1] = *position;
      step++;
      break;

    // Delimiter
    case 2:
      if (*position != DELIMITER) {
        throw std::invalid_argument("Expected delimiter in step " +
                                    std::to_string(step));
      }
      step++;
      break;

    // unkown pt2
    case 3:
      header.second_part = *position;
      step++;
      break;

    // body size
    case 4:
      header.body_size = std::to_integer<uint8_t>(*position);
      step++;
      break;

    case 5:
      if (*position != DELIMITER) {
        throw std::invalid_argument("Expected delimiter in step " +
                                    std::to_string(step));
      }
      break_loop = true;
      step++;
      break;
    }
    if (break_loop) {
      position++;
      break;
    }
  }
  return header;
}

std::vector<std::byte> Header::serialize(Header input) {
  std::vector<std::byte> buffer;
  buffer.insert(buffer.end(), input.firt_part, input.firt_part + 2);
  buffer.push_back(DELIMITER);
  buffer.push_back(input.second_part);
  buffer.push_back(std::byte(input.body_size));
  buffer.push_back(DELIMITER);
  return buffer;
}

} // namespace sls3mcubridge