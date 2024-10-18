#include "new_package.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

namespace sls3mcubridge {

const std::byte DELIMITER = std::byte(0x0);

Header::Header(std::byte buffer[], int &bytes_read) : m_body_size(4) {
  bool break_loop = false;
  int step = 0;
  std::byte *current_byte = buffer;
  for (int i = 0; i < HEADER_SIZE; i++) {

    switch (step) {
    // Header UC
    case 0:
      if (*current_byte != FIRST_BYTE) {
        throw std::invalid_argument("expected first byte to be 'U'");
      }
      step++;
      break;
    case 1:
      if (*current_byte != SECOND_BYTE) {
        throw std::invalid_argument("expected first byte to be 'C'");
      }
      step++;
      break;

    // Delimiter
    case 2:
    case 5:
      if (*current_byte != DELIMITER) {
        throw std::invalid_argument("Expected delimiter in step " +
                                    std::to_string(step));
      }
      step++;
      break;

    // unkown
    case 3:
      m_unkown_byte = *current_byte;
      step++;
      break;

    // body size
    case 4:
      m_body_size = std::to_integer<size_t>(*current_byte);
      step++;
      break;
    }
    bytes_read++;
    current_byte++;
  }
}

std::vector<std::byte> Header::serialize() {
  std::vector<std::byte> buffer;
  buffer.push_back(FIRST_BYTE);
  buffer.push_back(SECOND_BYTE);
  buffer.push_back(DELIMITER);
  buffer.push_back(m_unkown_byte);
  buffer.push_back(std::byte(m_body_size));
  buffer.push_back(DELIMITER);
  return buffer;
}

std::shared_ptr<Body> Body::create(std::byte buffer[], size_t body_size,
                                   int &bytes_read) {
  int step = 0;
  int type_int = 0;
  std::byte *byte = buffer;
  for (int i = 0; i < BODY_HEADER_SIZE; i++) {
    switch (step) {
    case 0:
      type_int = uint16_t(*byte) << 8;
      step++;
      break;
    case 1:
      type_int |= uint16_t(*byte) << 0;
      step++;
      break;
    case 2:
    case 3:
      if (*byte != DELIMITER) {
        throw std::invalid_argument("Expected delimiter at step " +
                                    std::to_string(step));
      }
      step++;
      break;
    }
    byte++;
    bytes_read++;
  }

  Type type = Type::Unkown;
  try {
    type = int16_to_type_map.at(type_int);
  } catch (std::out_of_range) {
    type = Type::Unkown;
  }

  size_t sub_body_size = body_size - BODY_HEADER_SIZE;

  switch (type) {
  case Type::Midi:
    return std::make_shared<MidiBody>(byte, sub_body_size, bytes_read);
  case Type::Unkown:
    return std::make_shared<UnkownBody>(byte, sub_body_size, bytes_read);
  }
}

std::vector<std::byte> Body::serialize() {
  std::vector<std::byte> tmp;
  uint16_t type_int = find_type_in_map(m_type);
  tmp.push_back(std::byte(type_int >> 8));
  tmp.push_back(std::byte(type_int >> 0));
  tmp.push_back(DELIMITER);
  tmp.push_back(DELIMITER);
  return tmp;
}

uint16_t Body::find_type_in_map(Type type) {
  for (auto &it : int16_to_type_map) {
    if (it.second == type) {
      return it.first;
    }
  }
  // not found
  return 0;
}

MidiBody::MidiBody(std::byte buffer[], size_t size, int &bytes_read)
    : Body(Body::Type::Midi, size) {
  int step = 0;
  std::byte *current_byte = buffer;
  for (int i = 0; i < size; i++) {
    switch (step) {
    case 0:
      m_device = *current_byte;
      step++;
      break;
    case 1:
    case 3:
      if (*current_byte != DELIMITER) {
        throw std::invalid_argument("Delimiter expected");
      }
      step++;
      break;
    case 2:
      if (i == size - 1) {
        break;
      }
      m_message.push_back(*current_byte);
      break;
    }
    bytes_read++;
    current_byte++;
  }
}

std::vector<std::byte> MidiBody::serialize() {
  auto tmp = std::vector<std::byte>();
  auto body_header = Body::serialize();
  tmp.insert(tmp.end(), body_header.begin(), body_header.end());
  tmp.push_back(m_device);
  tmp.push_back(DELIMITER);
  tmp.insert(tmp.end(), m_message.begin(), m_message.end());
  tmp.push_back(DELIMITER);
  return tmp;
}

Package::Package(std::byte buffer[], int &bytes_read)
    : m_header(buffer, bytes_read),
      m_body(Body::create(buffer + bytes_read, m_header.get_body_size(),
                          bytes_read)) {}

std::vector<std::byte> Package::serialize() {
  std::vector<std::byte> tmp = m_header.serialize();
  auto body_vec = m_body->serialize();
  tmp.insert(tmp.end(), body_vec.begin(), body_vec.end());
  return tmp;
}

UnkownBody::UnkownBody(std::byte buffer[], size_t size, int &bytes_read)
    : Body(Body::Type::Unkown, size) {
  for (int i = 0; i < size; i++) {
    m_content.push_back(buffer[i]);
    bytes_read++;
  }
}

std::vector<std::byte> UnkownBody::serialize() {
  auto body_header = Body::serialize();
  std::vector<std::byte> tmp;
  tmp.insert(tmp.end(), body_header.begin(), body_header.end());
  tmp.insert(tmp.end(), m_content.begin(), m_content.end());
  return tmp;
}
} // namespace sls3mcubridge