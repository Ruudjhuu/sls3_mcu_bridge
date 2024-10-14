#pragma once

#include <memory>
#include <vector>

namespace sls3mcubridge {

struct midi_message {
  int device_number;
  std::shared_ptr<std::vector<unsigned char>> message;
};

class Parser {
public:
  Parser() {}
  std::shared_ptr<midi_message> to_midi(unsigned char input[], size_t length);
};
} // namespace sls3mcubridge