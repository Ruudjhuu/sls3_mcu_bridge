#include "parser.hpp"
#include "gtest/gtest.h"
#include <cstddef>

TEST(TestParser, test_parser_midi_device_1) {
  std::vector<std::byte> input = {
      std::byte('U'),  std::byte('C'),  std::byte(0x00), std::byte(0x01),
      std::byte(0x0a), std::byte(0x00), std::byte(0x4d), std::byte(0x4d),
      std::byte(0x00), std::byte(0x00), std::byte(0x6c), std::byte(0x00),
      std::byte(0x90), std::byte(0x10), std::byte(0x7f), std::byte(0x00),
  };

  sls3mcubridge::Package package;

  auto output = package.deserialize(input);
  ASSERT_EQ(input, package.serialize(output));
}

TEST(TestHeaderAerialization, test_header_serialization) {
  std::vector<std::byte> input = {
      std::byte('U'),  std::byte('C'),  std::byte(0x00), std::byte(0x01),
      std::byte(0x0a), std::byte(0x00), std::byte(0x4d), std::byte(0x4d),
      std::byte(0x00), std::byte(0x00)};

  sls3mcubridge::Header header;

  auto output = header.deserialize(input);
  ASSERT_EQ(input, header.serialize(output));
}