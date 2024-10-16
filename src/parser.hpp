#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace sls3mcubridge {

struct midi_message {
  int device_number;
  std::vector<std::byte> message;
};

struct Header {
  static Header deserialize(std::vector<std::byte> input,
                            std::vector<std::byte>::iterator &position);
  static std::vector<std::byte> serialize(Header input);

  std::byte firt_part[2]; // always UC
  std::byte second_part;  // unkown
  uint8_t body_size;
};

struct Package {
  static Package deserialize(std::vector<std::byte> input,
                             std::vector<std::byte>::iterator &position);
  static std::vector<std::byte> serialize(Package input);

  Header header;
  std::vector<std::byte> body;
};

class Parser {
public:
  Parser() {}
};
} // namespace sls3mcubridge