#include "package.hpp"
#include "libremidi/config.hpp"
#include "libremidi/message.hpp"

#include <climits>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

namespace sls3mcubridge::tcp {

const std::byte DELIMITER = std::byte(0x0);

Header::Header(std::byte buffer[], int &bytes_read) : m_body_size(4) {
  bool break_loop = false;
  int step = 0;
  std::byte *current_byte = buffer;
  for (int i = 0; i < HEADER_SIZE; i++) {

    switch (step) {
    // Header UC
    case 0:
      if (*current_byte != HEADEr_FIRST_BYTE) {
        throw std::invalid_argument("expected first byte to be 'U'");
      }
      step++;
      break;
    case 1:
      if (*current_byte != HEADER_SECOND_BYTE) {
        throw std::invalid_argument("expected first byte to be 'C'");
      }
      step++;
      break;

    // Delimiter
    case 2:
      if (*current_byte != DELIMITER) {
        throw std::invalid_argument("Expected delimiter in step " +
                                    std::to_string(step));
      }
      step++;
      break;

    // unkown
    case 3:
      if (*current_byte != HEADER_UNKOWN_BYTE) {
        throw std::invalid_argument("Expected 0x01 in step " +
                                    std::to_string(step));
      }
      step++;
      break;

    // body size
    case 4:
      m_body_size = std::to_integer<size_t>(*current_byte);
      step++;
      break;

    // delimiter
    case 5:
      if (*current_byte != DELIMITER) {
        throw std::invalid_argument("Expected delimiter in step " +
                                    std::to_string(step));
      }
      step++;
      break;
    default:
      throw std::out_of_range("Unexpected step handled");
    }
    bytes_read++;
    current_byte++;
  }
}

std::vector<std::byte> Header::serialize() {
  std::vector<std::byte> buffer;
  buffer.push_back(HEADEr_FIRST_BYTE);
  buffer.push_back(HEADER_SECOND_BYTE);
  buffer.push_back(DELIMITER);
  buffer.push_back(HEADER_UNKOWN_BYTE);
  buffer.push_back(std::byte(m_body_size));
  buffer.push_back(DELIMITER);
  return buffer;
}

std::shared_ptr<Body> Body::create(std::byte buffer[], size_t body_size,
                                   int &bytes_read) {
  int step = 0;
  uint16_t type_int = 0;
  std::byte *byte = buffer;
  for (int i = 0; i < BODY_HEADER_SIZE; i++) {
    switch (step) {
    case 0:
      type_int = static_cast<uint16_t>(*byte) << (sizeof(*byte) * CHAR_BIT);
      step++;
      break;
    case 1:
      type_int |= static_cast<uint16_t>(*byte);
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
    default:
      throw std::out_of_range("Unexpected step handled");
    }
    byte++;
    bytes_read++;
  }

  Type type = Type::Unkown;
  try {
    type = int16_to_type_map.at(type_int);
  } catch (std::out_of_range &) {
    type = Type::Unkown;
  }

  size_t sub_body_size = body_size - BODY_HEADER_SIZE;

  switch (type) {
  case Type::IncommingMidi:
    return std::make_shared<IncommingMidiBody>(byte, sub_body_size, bytes_read);
  case Type::OutgoingMidi:
    return std::make_shared<OutgoingMidiBody>(byte, sub_body_size, bytes_read);
  case Type::SysEx:
    return std::make_shared<SysExMidiBody>(byte, sub_body_size, bytes_read);
  case Type::Unkown:
    return std::make_shared<UnkownBody>(byte, sub_body_size, bytes_read);
  }
}

std::vector<std::byte> Body::serialize() {
  std::vector<std::byte> tmp;
  uint16_t type_int = find_type_in_map(m_type);
  tmp.push_back(
      std::byte(type_int >> (sizeof(decltype(tmp)::value_type) * CHAR_BIT)));
  tmp.push_back(std::byte(type_int));
  tmp.push_back(DELIMITER);
  tmp.push_back(DELIMITER);
  return tmp;
}

uint16_t Body::find_type_in_map(Type type) {
  for (const auto &iter : int16_to_type_map) {
    if (iter.second == type) {
      return iter.first;
    }
  }
  // not found
  return 0;
}

IncommingMidiBody::IncommingMidiBody(std::byte buffer[], size_t size,
                                     int &bytes_read)
    : Body(Body::Type::IncommingMidi, size) {
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
      m_message.bytes.push_back(static_cast<unsigned char>(*current_byte));
      break;
    default:
      throw std::out_of_range("Unexpected step handled");
    }
    bytes_read++;
    current_byte++;
  }
}

std::vector<std::byte> IncommingMidiBody::serialize() {
  auto tmp = std::vector<std::byte>();
  auto body_header = Body::serialize();
  tmp.insert(tmp.end(), body_header.begin(), body_header.end());
  tmp.push_back(m_device);
  tmp.push_back(DELIMITER);
  for (auto &iter : m_message) {
    tmp.push_back(std::byte(iter));
  }
  tmp.push_back(DELIMITER);
  return tmp;
}

int IncommingMidiBody::get_device_index() {
  switch (m_device) {
  case std::byte(0x6c):
    return 0;
  case std::byte(0x6d):
    return 1;
  case std::byte(0x6e):
    return 2;
  default:
    throw std::invalid_argument("Could not determine midi device index");
  }
}

OutgoingMidiBody::OutgoingMidiBody(
    std::byte device, const std::vector<libremidi::message> &messages)
    : Body(Body::Type::OutgoingMidi, 0), m_device(device),
      m_messages(messages) {
  size_t tmp_size = 0;
  tmp_size += BODY_HEADER_SIZE;
  tmp_size += 3;
  for (auto &iter : m_messages) {
    tmp_size += iter.size();
  }
  set_size(tmp_size);
}

OutgoingMidiBody::OutgoingMidiBody(std::byte buffer[], size_t size,
                                   int &bytes_read)
    : Body(Body::Type::OutgoingMidi, size) {
  int step = 0;
  int nr_of_messages = 0;
  std::byte *current_byte = buffer;
  for (int i = 0; i < size; i++) {
    switch (step) {
    case 0:
      m_device = *current_byte;
      step++;
      break;
    case 1:
      if (*current_byte != DELIMITER) {
        throw std::invalid_argument("Delimiter expected");
      }
      step++;
      break;
    case 2:
      nr_of_messages = (int)*current_byte;
      step++;
      break;
    case 3:
      for (int j = 0; j < nr_of_messages; j++) {
        m_messages.push_back(libremidi::message(
            {static_cast<unsigned char>(*current_byte),
             static_cast<unsigned char>(*(current_byte + 1)),
             static_cast<unsigned char>(*(current_byte + 2))}));
        current_byte += 3;
        bytes_read += 3;
      }
      return;
    default:
      throw std::out_of_range("Unexpected step handled");
    }
    bytes_read++;
    current_byte++;
  }
}

std::vector<std::byte> OutgoingMidiBody::serialize() {
  auto tmp = std::vector<std::byte>();
  auto body_header = Body::serialize();
  tmp.insert(tmp.end(), body_header.begin(), body_header.end());
  tmp.push_back(m_device);
  tmp.push_back(DELIMITER);
  tmp.push_back(std::byte(m_messages.size()));
  for (auto &iter : m_messages) {
    for (auto &itt : iter) {
      tmp.push_back(std::byte(itt));
    }
  }
  return tmp;
}

SysExMidiBody::SysExMidiBody(std::byte buffer[], size_t size, int &bytes_read)
    : Body(Body::Type::SysEx, size) {
  int step = 0;
  int sysex_length = 0;
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
      sysex_length = (int)*current_byte;
      step++;
      break;
    case 4: {
      libremidi::midi_bytes tmp_midi;
      for (int j = 0; j < sysex_length; j++) {
        tmp_midi.push_back((unsigned char)*current_byte);
        bytes_read++;
        current_byte++;
      }
      m_message.bytes = tmp_midi;
    }
      return;
    default:
      throw std::out_of_range("Unexpected step handled");
    }
    bytes_read++;
    current_byte++;
  }
}

std::vector<std::byte> SysExMidiBody::serialize() {
  auto tmp = std::vector<std::byte>();
  auto body_header = Body::serialize();
  tmp.insert(tmp.end(), body_header.begin(), body_header.end());
  tmp.push_back(m_device);
  tmp.push_back(DELIMITER);
  tmp.push_back(std::byte(m_message.size()));
  tmp.push_back(DELIMITER);
  for (auto &iter : m_message) {
    tmp.push_back(std::byte(iter));
  }
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
} // namespace sls3mcubridge::tcp
