#pragma once

#include <vector>

namespace sls3mcubridge {
struct tcp_data {
  unsigned char *header;
  unsigned char *body;
};

class Parser {
public:
  Parser() {}
  std::vector<unsigned char> to_midi(unsigned char input[]);
};
} // namespace sls3mcubridge