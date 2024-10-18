
#include "gtest/gtest.h"
#include <cstddef>

#include "new_package.hpp"

namespace sls3mcubridge {

TEST(TestBodySerialization, test_midibody_serialization) {
  const size_t size = 10;
  std::byte input[size] = {std::byte(0x4d), std::byte(0x4d), std::byte(0x00),
                           std::byte(0x00), std::byte(0x6c), std::byte(0x00),
                           std::byte(0x90), std::byte(0x10), std::byte(0x7f),
                           std::byte(0x00)};
  int read_bytes = 0;

  auto body = Body::create(input, size, read_bytes);

  ASSERT_EQ(body->get_type(), Body::Type::Midi);
  ASSERT_EQ(std::vector<std::byte>(input, input + size), body->serialize());
  ASSERT_EQ(read_bytes, size);
}

TEST(TestBodySerialization, test_unkownbody_serialization) {
  const size_t size = 10;
  std::byte input[size] = {std::byte(0x3d), std::byte(0x4d), std::byte(0x00),
                           std::byte(0x00), std::byte(0x6c), std::byte(0x00),
                           std::byte(0x90), std::byte(0x10), std::byte(0x7f),
                           std::byte(0x00)};
  int read_bytes = 0;

  auto body = Body::create(input, size, read_bytes);

  // unkown body type is set to 0
  input[0] = std::byte(0);
  input[1] = std::byte(0);

  ASSERT_EQ(body->get_type(), Body::Type::Unkown);
  ASSERT_EQ(std::vector<std::byte>(input, input + size), body->serialize());
  ASSERT_EQ(read_bytes, size);
}

TEST(TestHeaderSerialization, test_header_serialization) {
  const size_t size = 6;
  std::byte input[size] = {std::byte('U'),  std::byte('C'),  std::byte(0x00),
                           std::byte(0x01), std::byte(0x0a), std::byte(0x00)};

  int read_bytes = 0;
  auto output = Header(input, read_bytes);

  ASSERT_EQ(std::vector<std::byte>(input, input + size), output.serialize());
  ASSERT_EQ(read_bytes, size);
}

TEST(TestPackageSerialization, test_package_serialization) {
  const size_t size = 16;
  std::byte input[size] = {
      std::byte('U'),  std::byte('C'),  std::byte(0x00), std::byte(0x01),
      std::byte(0x0a), std::byte(0x00), std::byte(0x4d), std::byte(0x4d),
      std::byte(0x00), std::byte(0x00), std::byte(0x6c), std::byte(0x00),
      std::byte(0x90), std::byte(0x10), std::byte(0x7f), std::byte(0x00),
  };

  int bytes_read = 0;
  auto output = Package(input, bytes_read);
  ASSERT_EQ(std::vector<std::byte>(input, input + size), output.serialize());
  ASSERT_EQ(bytes_read, size);
}
} // namespace sls3mcubridge