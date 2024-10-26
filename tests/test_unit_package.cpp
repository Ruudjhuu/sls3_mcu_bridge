
#include "gtest/gtest.h"
#include <array>
#include <cstddef>
#include <vector>

#include "libremidi/message.hpp"
#include "package.hpp"

namespace sls3mcubridge::tcp {

TEST(TestTcpPackageCreation, testIncommingmidibodyCreateByBuffer) {
  std::array<std::byte, 10> input = {
      std::byte(0x4d), std::byte(0x4d), std::byte(0x00), std::byte(0x00),
      std::byte(0x6c), std::byte(0x00), std::byte(0x90), std::byte(0x10),
      std::byte(0x7f), std::byte(0x00)};

  auto body = Body::create(BufferView(input.begin(), input.end()));

  ASSERT_EQ(body->get_type(), Body::Type::IncommingMidi);
  ASSERT_EQ(std::vector<std::byte>(input.begin(), input.end()),
            body->serialize());
  ASSERT_EQ(body->get_size(), input.size());
}

TEST(TestTcpPackageCreation, testIncommingmidibodyCreateByArgs) {
  std::vector<std::byte> expected_output = {
      std::byte(0x4d), std::byte(0x4d), std::byte(0x00), std::byte(0x00),
      std::byte(0x6c), std::byte(0x00), std::byte(0x90), std::byte(0x10),
      std::byte(0x7f), std::byte(0x00)};

  auto device = std::byte(0x6c);
  auto message = libremidi::message({0x90, 0x10, 0x7f});

  auto body = IncommingMidiBody(device, message);

  ASSERT_EQ(body.get_type(), Body::Type::IncommingMidi);
  ASSERT_EQ(expected_output, body.serialize());
  ASSERT_EQ(body.get_size(), expected_output.size());
}

TEST(TestTcpPackageCreation, testOutgoingmidibodyCreateByBuffer) {
  std::array<std::byte, 52> input = {
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

  auto body = Body::create(BufferView(input.begin(), input.end()));

  ASSERT_EQ(body->get_type(), Body::Type::OutgoingMidi);
  ASSERT_EQ(std::vector<std::byte>(input.begin(), input.end()),
            body->serialize());
  ASSERT_EQ(body->get_size(), input.size());
}

TEST(TestTcpPackageCreation, testOutgoingmidibodyCreateByArgs) {
  std::vector<std::byte> expected_output = {
      std::byte(0x4d), std::byte(0x41), std::byte(0x00), std::byte(0x00),
      std::byte(0x68), std::byte(0x00), std::byte(0x02), std::byte(0xb0),
      std::byte(0x40), std::byte(0x30), std::byte(0xb0), std::byte(0x41),
      std::byte(0x30)};

  auto device = std::byte(0x68);
  auto messages = {libremidi::message({0xb0, 0x40, 0x30}),
                   libremidi::message({0xb0, 0x41, 0x30})};

  auto body = OutgoingMidiBody(device, messages);

  ASSERT_EQ(body.get_type(), Body::Type::OutgoingMidi);
  ASSERT_EQ(expected_output, body.serialize());
  ASSERT_EQ(body.get_size(), expected_output.size());
}

TEST(TestTcpPackageCreation, testSysexmidibodyCreateByBuffer) {
  std::array<std::byte, 15> input = {
      std::byte(0x53), std::byte(0x53), std::byte(0x00), std::byte(0x00),
      std::byte(0x68), std::byte(0x00), std::byte(0x07), std::byte(0x00),
      std::byte(0xf0), std::byte(0x00), std::byte(0x00), std::byte(0x66),
      std::byte(0x15), std::byte(0x00), std::byte(0xf7)};
  int read_bytes = 0;

  auto body = Body::create(BufferView(input.begin(), input.end()));

  ASSERT_EQ(body->get_type(), Body::Type::SysEx);
  ASSERT_EQ(std::vector<std::byte>(input.begin(), input.end()),
            body->serialize());
  ASSERT_EQ(body->get_size(), input.size());
}

TEST(TestTcpPackageCreation, testSysexmidibodyCreateByArgs) {
  std::vector<std::byte> expected_output = {
      std::byte(0x53), std::byte(0x53), std::byte(0x00), std::byte(0x00),
      std::byte(0x68), std::byte(0x00), std::byte(0x07), std::byte(0x00),
      std::byte(0xf0), std::byte(0x00), std::byte(0x00), std::byte(0x66),
      std::byte(0x15), std::byte(0x00), std::byte(0xf7)};

  auto device = std::byte(0x68);
  auto message = libremidi::message({0xf0, 0x00, 0x00, 0x66, 0x15, 0x00, 0xf7});
  auto body = SysExMidiBody(device, message);

  ASSERT_EQ(body.get_type(), Body::Type::SysEx);
  ASSERT_EQ(expected_output, body.serialize());
  ASSERT_EQ(body.get_size(), expected_output.size());
}

TEST(TestTcpPackageCreation, testUnkownbodyCreateByBuffer) {
  std::array<std::byte, 10> input = {
      std::byte(0x3d), std::byte(0x4d), std::byte(0x00), std::byte(0x00),
      std::byte(0x6c), std::byte(0x00), std::byte(0x90), std::byte(0x10),
      std::byte(0x7f), std::byte(0x00)};

  auto body = Body::create(BufferView(input.begin(), input.end()));

  // unkown body type is set to 0
  input[0] = std::byte(0);
  input[1] = std::byte(0);

  ASSERT_EQ(body->get_type(), Body::Type::Unkown);
  ASSERT_EQ(std::vector<std::byte>(input.begin(), input.end()),
            body->serialize());
  ASSERT_EQ(body->get_size(), input.size());
}

TEST(TestTcpPackageCreation, testHeaderSerialization) {
  const size_t size = 6;
  std::array<std::byte, 6> input = {std::byte('U'),  std::byte('C'),
                                    std::byte(0x00), std::byte(0x01),
                                    std::byte(0x0a), std::byte(0x00)};

  auto output = Header(BufferView(input.begin(), input.end()));

  ASSERT_EQ(std::vector<std::byte>(input.begin(), input.end()),
            output.serialize());
}

TEST(TestTcpPackageCreation, testHeaderCreateByArg) {
  std::vector<std::byte> expected_output = {std::byte('U'),  std::byte('C'),
                                            std::byte(0x00), std::byte(0x01),
                                            std::byte(0x0a), std::byte(0x00)};

  auto output = Header(10);

  ASSERT_EQ(expected_output, output.serialize());
}

TEST(TestTcpPackageCreation, testPackageSerialization) {
  std::array<std::byte, 16> input = {
      std::byte('U'),  std::byte('C'),  std::byte(0x00), std::byte(0x01),
      std::byte(0x0a), std::byte(0x00), std::byte(0x4d), std::byte(0x4d),
      std::byte(0x00), std::byte(0x00), std::byte(0x6c), std::byte(0x00),
      std::byte(0x90), std::byte(0x10), std::byte(0x7f), std::byte(0x00),
  };

  int bytes_read = 0;
  auto output = Package(BufferView(input.begin(), input.end()));
  ASSERT_EQ(std::vector<std::byte>(input.begin(), input.end()),
            output.serialize());
  ASSERT_EQ(output.get_size(), input.size());
}
} // namespace sls3mcubridge::tcp
