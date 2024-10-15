#pragma once

#include <cstdint>
#include <vector>

namespace sls3mcubridge {

struct midi_message {
  int device_number;
  std::vector<std::byte> message;
};

struct Package {
  std::byte header;
  uint8_t midi_dev;
  std::vector<std::byte> body;
};

class Parser {
public:
  Parser() {}
  std::vector<Package> serialize(std::byte input[], size_t nr_bytes);
};
} // namespace sls3mcubridge