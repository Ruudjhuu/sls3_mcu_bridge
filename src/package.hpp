#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace sls3mcubridge {

struct Header {
  static Header deserialize(std::vector<std::byte> input,
                            std::vector<std::byte>::iterator &position);
  static std::vector<std::byte> serialize(Header input);

  std::byte firt_part[2]; // always "UC"
  std::byte second_part;  // unkown
  uint8_t body_size;
};

struct MidiContent {
  static MidiContent deserialize(std::vector<std::byte> input);
  static std::vector<std::byte> serialize(MidiContent input);

  std::byte device;
  std::vector<std::byte> message;
};

struct Body {
  static Body deserialize(std::vector<std::byte> input);
  static std::vector<std::byte> serialize(Body input);

  uint16_t type;
  MidiContent midi_content;
};

struct Package {
  static Package deserialize(std::vector<std::byte> input,
                             std::vector<std::byte>::iterator &position);
  static std::vector<std::byte> serialize(Package input);

  Header header;
  Body body;
};
} // namespace sls3mcubridge