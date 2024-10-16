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
  package.body = Body::deserialize(
      std::vector<std::byte>(position, position + package.header.body_size));
  position += package.header.body_size;
  return package;
}

std::vector<std::byte> Package::serialize(Package input) {
  std::vector<std::byte> buffer = Header::serialize(input.header);
  auto body_buffer = Body::serialize(input.body);
  buffer.insert(buffer.end(), body_buffer.begin(), body_buffer.end());
  return buffer;
}

Body Body::deserialize(std::vector<std::byte> input) {
  int step = 0;
  Body body;
  std::vector<std::byte> content_buffer;
  for (auto &it : input) {
    switch (step) {
    case 0:
      body.type = uint16_t(it) << 8;
      step++;
      break;
    case 1:
      body.type |= uint16_t(it) << 0;
      step++;
      break;
    case 2:
    case 3:
      if (it != DELIMITER) {
        throw std::invalid_argument("Expected delimiter at step " +
                                    std::to_string(step));
      }
      step++;
      break;
    case 4:
      // incomming midi
      if (body.type == 19789) {
        content_buffer.push_back(it);
      } else {
        throw std::invalid_argument("Unkown body type");
      }
      break;
    }
  }
  body.midi_content = MidiContent::deserialize(content_buffer);
  return body;
}

std::vector<std::byte> Body::serialize(Body input) {
  std::vector<std::byte> buffer;
  buffer.push_back(std::byte(input.type >> 8));
  buffer.push_back(std::byte(input.type >> 0));
  buffer.push_back(DELIMITER);
  buffer.push_back(DELIMITER);
  auto tmp = MidiContent::serialize(input.midi_content);
  buffer.insert(buffer.end(), tmp.begin(), tmp.end());
  return buffer;
}

MidiContent MidiContent::deserialize(std::vector<std::byte> input) {
  int step = 0;
  auto content = MidiContent();
  for (auto it = input.begin(); it != input.end(); it++) {
    switch (step) {
    case 0:
      content.device = *it;
      step++;
      break;
    case 1:
      if (*it != DELIMITER) {
        throw std::invalid_argument("Delimiter expected");
      }
      step++;
      break;
    case 2:
      if (it + 1 == input.end()) {
        break;
      }
      content.message.push_back(*it);
      break;
    }
  }
  return content;
}

std::vector<std::byte> MidiContent::serialize(MidiContent input) {
  auto tmp = std::vector<std::byte>();
  tmp.push_back(input.device);
  tmp.push_back(DELIMITER);
  tmp.insert(tmp.end(), input.message.begin(), input.message.end());
  tmp.push_back(DELIMITER);
  return tmp;
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