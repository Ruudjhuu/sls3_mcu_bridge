#pragma once

#include <cstddef>
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

  std::byte firt_part[2];
  std::byte second_part[2];
  std::byte third_part[2];
};

struct Package {
  static Package deserialize(std::vector<std::byte> input,
                             std::vector<std::byte>::iterator &position);
  static std::vector<std::byte> serialize(Package input);

  Header header;
  std::byte midi_dev;
  std::vector<std::byte> body;
};

class Parser {
public:
  Parser() {}
};
} // namespace sls3mcubridge