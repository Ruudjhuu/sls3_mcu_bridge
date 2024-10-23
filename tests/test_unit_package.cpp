
#include "gtest/gtest.h"
#include <cstddef>

#include "package.hpp"

namespace sls3mcubridge {
namespace tcp {

TEST(TestTcpPackageCreation, test_incommingmidibody_create_by_buffer) {
  const size_t size = 10;
  std::byte input[size] = {std::byte(0x4d), std::byte(0x4d), std::byte(0x00),
                           std::byte(0x00), std::byte(0x6c), std::byte(0x00),
                           std::byte(0x90), std::byte(0x10), std::byte(0x7f),
                           std::byte(0x00)};
  int read_bytes = 0;

  auto body = Body::create(input, size, read_bytes);

  ASSERT_EQ(body->get_type(), Body::Type::IncommingMidi);
  ASSERT_EQ(std::vector<std::byte>(input, input + size), body->serialize());
  ASSERT_EQ(read_bytes, size);
}

TEST(TestTcpPackageCreation, test_outgoingmidibody_create_by_buffer) {
  const size_t size = 52;
  std::byte input[size] = {
      std::byte(0x4d), std::byte(0x41), std::byte(0x00), std::byte(0x00),
      std::byte(0x68), std::byte(0x00), std::byte(0x0f), std::byte(0xb0),
      std::byte(0x40), std::byte(0x30), std::byte(0xb0), std::byte(0x41),
      std::byte(0x30), std::byte(0xb0), std::byte(0x42), std::byte(0x30),
      std::byte(0xb0), std::byte(0x43), std::byte(0x30), std::byte(0xb0),
      std::byte(0x44), std::byte(0x30), std::byte(0xb0), std::byte(0x45),
      std::byte(0x30), std::byte(0xb0), std::byte(0x46), std::byte(0x30),
      std::byte(0xb0), std::byte(0x47), std::byte(0x30), std::byte(0xb0),
      std::byte(0x48), std::byte(0x30), std::byte(0xb0), std::byte(0x49),
      std::byte(0x30), std::byte(0xb0), std::byte(0x4b), std::byte(0x30),
      std::byte(0xb0), std::byte(0x4a), std::byte(0x30), std::byte(0xb0),
      std::byte(0x30), std::byte(0x30), std::byte(0xb0), std::byte(0x31),
      std::byte(0x30), std::byte(0xb0), std::byte(0x32), std::byte(0x30)};
  int read_bytes = 0;

  auto body = Body::create(input, size, read_bytes);

  ASSERT_EQ(body->get_type(), Body::Type::OutgoingMidi);
  ASSERT_EQ(std::vector<std::byte>(input, input + size).size(),
            body->serialize().size());
  ASSERT_EQ(std::vector<std::byte>(input, input + size), body->serialize());
  ASSERT_EQ(read_bytes, size);
}

TEST(TestTcpPackageCreation, test_sysexmidibody_create_by_buffer) {
  const size_t size = 15;
  std::byte input[size] = {std::byte(0x53), std::byte(0x53), std::byte(0x00),
                           std::byte(0x00), std::byte(0x68), std::byte(0x00),
                           std::byte(0x07), std::byte(0x00), std::byte(0xf0),
                           std::byte(0x00), std::byte(0x00), std::byte(0x66),
                           std::byte(0x15), std::byte(0x00), std::byte(0xf7)};
  int read_bytes = 0;

  auto body = Body::create(input, size, read_bytes);

  ASSERT_EQ(body->get_type(), Body::Type::SysEx);
  ASSERT_EQ(std::vector<std::byte>(input, input + size), body->serialize());
  ASSERT_EQ(read_bytes, size);
}

TEST(TestTcpPackageCreation, test_unkownbody_create_by_buffer) {
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

TEST(TestTcpPackageCreation, test_header_serialization) {
  const size_t size = 6;
  std::byte input[size] = {std::byte('U'),  std::byte('C'),  std::byte(0x00),
                           std::byte(0x01), std::byte(0x0a), std::byte(0x00)};

  int read_bytes = 0;
  auto output = Header(input, read_bytes);

  ASSERT_EQ(std::vector<std::byte>(input, input + size), output.serialize());
  ASSERT_EQ(read_bytes, size);
}

TEST(TestTcpPackageCreation, test_package_serialization) {
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
} // namespace tcp
} // namespace sls3mcubridge