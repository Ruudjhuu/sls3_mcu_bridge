#include "package.hpp"
#include "gtest/gtest.h"
#include <cstddef>
#include <vector>

TEST(TestPackageSerialization, test_package_serialization) {
  std::vector<std::byte> input = {
      std::byte('U'),  std::byte('C'),  std::byte(0x00), std::byte(0x01),
      std::byte(0x0a), std::byte(0x00), std::byte(0x4d), std::byte(0x4d),
      std::byte(0x00), std::byte(0x00), std::byte(0x6c), std::byte(0x00),
      std::byte(0x90), std::byte(0x10), std::byte(0x7f), std::byte(0x00),
  };

  auto position = input.begin();
  auto output = sls3mcubridge::Package::deserialize(input, position);
  ASSERT_EQ(input, sls3mcubridge::Package::serialize(output));
  ASSERT_EQ(input.end(), position);
}

TEST(TestPackageSerialization,
     test_package_serialization_start_at_different_position) {
  std::vector<std::byte> input = {
      std::byte(0x00), std::byte(0x00), std::byte('U'),  std::byte('C'),
      std::byte(0x00), std::byte(0x01), std::byte(0x0a), std::byte(0x00),
      std::byte(0x4d), std::byte(0x4d), std::byte(0x00), std::byte(0x00),
      std::byte(0x6c), std::byte(0x00), std::byte(0x90), std::byte(0x10),
      std::byte(0x7f), std::byte(0x00),
  };

  std::vector<std::byte> expected(input.begin() + 2, input.end());

  auto position = input.begin() + 2;
  auto output = sls3mcubridge::Package::deserialize(input, position);
  ASSERT_EQ(expected, sls3mcubridge::Package::serialize(output));
  ASSERT_EQ(input.end(), position);
}

TEST(TestPackageSerialization,
     test_package_serialization_2_packages_in_one_buffer) {
  std::vector<std::byte> pkg1 = {
      std::byte('U'),  std::byte('C'),  std::byte(0x00), std::byte(0x01),
      std::byte(0x0a), std::byte(0x00), std::byte(0x4d), std::byte(0x4d),
      std::byte(0x00), std::byte(0x00), std::byte(0x6c), std::byte(0x00),
      std::byte(0x90), std::byte(0x10), std::byte(0x7f), std::byte(0x00),
  };
  std::vector<std::byte> pkg2 = {
      std::byte('U'),  std::byte('C'),  std::byte(0x00), std::byte(0x02),
      std::byte(0x0a), std::byte(0x00), std::byte(0x4d), std::byte(0x4d),
      std::byte(0x00), std::byte(0x00), std::byte(0x6c), std::byte(0x00),
      std::byte(0x91), std::byte(0x11), std::byte(0x7c), std::byte(0x00),
  };

  auto input = pkg1;
  input.insert(input.end(), pkg2.begin(), pkg2.end());
  auto position = input.begin();

  auto output1 = sls3mcubridge::Package::deserialize(input, position);
  ASSERT_EQ(input.begin() + pkg1.size(), position);
  auto output2 = sls3mcubridge::Package::deserialize(input, position);

  ASSERT_EQ(pkg1, sls3mcubridge::Package::serialize(output1));
  ASSERT_EQ(pkg2, sls3mcubridge::Package::serialize(output2));
  ASSERT_EQ(input.end(), position);
}

TEST(TestBodySerialization, test_body_serialization) {
  std::vector<std::byte> input = {
      std::byte(0x4d), std::byte(0x4d), std::byte(0x00), std::byte(0x00),
      std::byte(0x6c), std::byte(0x00), std::byte(0x90), std::byte(0x10),
      std::byte(0x7f), std::byte(0x00)};
  auto position = input.begin();
  auto output = sls3mcubridge::Body::deserialize(input);
  ASSERT_EQ(input, sls3mcubridge::Body::serialize(output));
}

TEST(TestMidiSerialization, test_midi_serialization) {
  std::vector<std::byte> input = {std::byte(0x6c), std::byte(0x00),
                                  std::byte(0x90), std::byte(0x10),
                                  std::byte(0x7f), std::byte(0x00)};
  auto position = input.begin();
  auto output = sls3mcubridge::MidiContent::deserialize(input);
  ASSERT_EQ(input, sls3mcubridge::MidiContent::serialize(output));
}

TEST(TestHeaderSerialization, test_header_serialization) {
  std::vector<std::byte> input = {std::byte('U'),  std::byte('C'),
                                  std::byte(0x00), std::byte(0x01),
                                  std::byte(0x0a), std::byte(0x00)};

  auto position = input.begin();
  auto output = sls3mcubridge::Header::deserialize(input, position);
  ASSERT_EQ(input, sls3mcubridge::Header::serialize(output));
  ASSERT_EQ(position, input.end());
}

TEST(TestHeaderSerialization,
     test_header_serialization_start_at_different_position) {
  std::vector<std::byte> input = {
      std::byte(0x00), std::byte(0x00), std::byte('U'),  std::byte('C'),
      std::byte(0x00), std::byte(0x01), std::byte(0x0a), std::byte(0x00)};

  std::vector<std::byte> expectation(input.begin() + 2, input.end());

  auto position = input.begin() + 2;
  auto output = sls3mcubridge::Header::deserialize(input, position);
  ASSERT_EQ(expectation, sls3mcubridge::Header::serialize(output));
  ASSERT_EQ(position, input.end());
}

TEST(TestHeaderSerialization, test_header_serialization_longer_input) {
  std::vector<std::byte> input = {
      std::byte('U'),  std::byte('C'),  std::byte(0x00), std::byte(0x01),
      std::byte(0x0a), std::byte(0x00), std::byte(0x00), std::byte(0x00),
  };

  std::vector<std::byte> expectation(input.begin(), input.end() - 2);

  auto position = input.begin();
  auto output = sls3mcubridge::Header::deserialize(input, position);
  ASSERT_EQ(expectation, sls3mcubridge::Header::serialize(output));
  ASSERT_EQ(position, input.end() - 2);
}