#pragma once

#include <vector>

namespace sls3mcubridge {

class Parser {
public:
  Parser() {}
  std::vector<unsigned char> to_midi(unsigned char input[]);
};
} // namespace sls3mcubridge