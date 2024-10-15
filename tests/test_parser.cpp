#include "parser.hpp"
#include "gtest/gtest.h"
#include <cstddef>

TEST(TestParser, test_parser_midi_device_1) {
  sls3mcubridge::Parser parser;
  const size_t size = 16;
  std::byte input[size] = {
      std::byte('U'),  std::byte('C'),  std::byte(0x00), std::byte(0x01),
      std::byte(0x0a), std::byte(0x00), std::byte(0x4d), std::byte(0x4d),
      std::byte(0x00), std::byte(0x00), std::byte(0x6c), std::byte(0x00),
      std::byte(0x90), std::byte(0x10), std::byte(0x7f), std::byte(0x00),
  };
  std::vector<std::byte> expected_body = {std::byte(0x90), std::byte(0x10),
                                          std::byte(0x7f)};

  auto output = parser.serialize(input, size);
  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output.at(0).midi_dev, 1);
  ASSERT_EQ(output.at(0).body, expected_body);
}

TEST(TestParser, test_parser_midi_device_2) {
  sls3mcubridge::Parser parser;
  const size_t size = 16;
  std::byte input[size] = {
      std::byte('U'),  std::byte('C'),  std::byte(0x00), std::byte(0x01),
      std::byte(0x0a), std::byte(0x00), std::byte(0x4d), std::byte(0x4d),
      std::byte(0x00), std::byte(0x00), std::byte(0x6d), std::byte(0x00),
      std::byte(0x90), std::byte(0x10), std::byte(0x7f), std::byte(0x00),
  };
  auto output = parser.serialize(input, size);
  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output.at(0).midi_dev, 2);
}

TEST(TestParser, test_parser_midi_device_3) {
  sls3mcubridge::Parser parser;
  const size_t size = 16;
  std::byte input[size] = {
      std::byte('U'),  std::byte('C'),  std::byte(0x00), std::byte(0x01),
      std::byte(0x0a), std::byte(0x00), std::byte(0x4d), std::byte(0x4d),
      std::byte(0x00), std::byte(0x00), std::byte(0x6e), std::byte(0x00),
      std::byte(0x90), std::byte(0x10), std::byte(0x7f), std::byte(0x00),
  };
  auto output = parser.serialize(input, size);
  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output.at(0).midi_dev, 3);
}
