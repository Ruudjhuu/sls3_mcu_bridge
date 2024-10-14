#include "parser.hpp"
#include <iostream>
#include <vector>

namespace sls3mcubridge {

std::vector<unsigned char> Parser::to_midi(unsigned char input[]) {
  std::cout << "to_midi" << std::endl;
  return std::vector<unsigned char>(0x1, 0x2);
}

} // namespace sls3mcubridge