#include "package.hpp"
#include "libremidi/config.hpp"
#include "libremidi/message.hpp"
#include "spdlog/spdlog.h"

#include <climits>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace sls3mcubridge::tcp {

const std::byte DELIMITER = std::byte(0x0);

const uint16_t SIZE_OF_BYTE = 8;

const uint8_t INCOMMING_MIDI_DEVICE_BASE = 0x6c;
const uint8_t OUTGOING_MIDI_DEVICE_BASE = 0x67;
const int NR_OF_SUPPORTED_INCOMMINGMIDI_DEVICES = 5;

std::map<uint16_t, Body::Type> int16_to_type_map() {
  try {
    static const std::map<uint16_t, Body::Type> int16_to_type_map = {
        {19789, Body::Type::IncommingMidi},
        {19777, Body::Type::OutgoingMidi},
        {21331, Body::Type::SysEx}};
    return int16_to_type_map;
  } catch (std::exception &exc) {
    spdlog::error("failed to create static int16 to type map.");
    throw exc;
  }
}

Header::Header(BufferView<std::byte *> buffer_view) {
  int step = 0;
  for (const auto &iter : buffer_view) {
    switch (step) {
    // Header UC
    case 0:
      if (iter != HEADER_FIRST_BYTE) {
        throw std::invalid_argument("expected first byte to be 'U'");
      }
      step++;
      break;
    case 1:
      if (iter != HEADER_SECOND_BYTE) {
        throw std::invalid_argument("expected first byte to be 'C'");
      }
      step++;
      break;

    // Delimiter
    case 2:
      if (iter != DELIMITER) {
        throw std::invalid_argument("Expected delimiter in step " +
                                    std::to_string(step));
      }
      step++;
      break;

    // unkown
    case 3:
      if (iter != HEADER_UNKOWN_BYTE) {
        throw std::invalid_argument("Expected 0x01 in step " +
                                    std::to_string(step));
      }
      step++;
      break;

    // body size
    case 4:
      m_body_size = std::to_integer<uint8_t>(iter);
      step++;
      break;

    // delimiter
    case 5: // NOLINT
      if (iter != DELIMITER) {
        throw std::invalid_argument("Expected delimiter in step " +
                                    std::to_string(step));
      }
      step++;
      break;
    default:
      throw std::out_of_range("Unexpected step handled");
    }
  }
}

std::vector<std::byte> Header::serialize() {
  std::vector<std::byte> buffer;
  buffer.push_back(HEADER_FIRST_BYTE);
  buffer.push_back(HEADER_SECOND_BYTE);
  buffer.push_back(DELIMITER);
  buffer.push_back(HEADER_UNKOWN_BYTE);
  buffer.push_back(std::byte(m_body_size));
  buffer.push_back(DELIMITER);
  return buffer;
}

std::shared_ptr<Body> Body::create(BufferView<std::byte *> buffer_view) {
  int step = 0;
  uint16_t type_int = 0;
  auto header_view =
      BufferView(buffer_view.begin(), buffer_view.begin() + BODY_HEADER_SIZE);
  for (auto &iter : header_view) {
    switch (step) {
    case 0:
      type_int =
          static_cast<uint16_t>(static_cast<uint16_t>(iter) << SIZE_OF_BYTE);
      step++;
      break;
    case 1:
      type_int |= static_cast<uint16_t>(iter);
      step++;
      break;
    case 2:
    case 3:
      if (iter != DELIMITER) {
        throw std::invalid_argument("Expected delimiter at step " +
                                    std::to_string(step));
      }
      step++;
      break;
    default:
      throw std::out_of_range("Unexpected step handled");
    }
  }
  Type type = Type::Unkown;
  try {
    type = int16_to_type_map().at(type_int);
  } catch (std::out_of_range &) {
    type = Type::Unkown;
  }

  auto sub_body_view = BufferView(header_view.end(), buffer_view.end());

  switch (type) {
  case Type::IncommingMidi:
    return std::make_shared<IncommingMidiBody>(sub_body_view);
  case Type::OutgoingMidi:
    return std::make_shared<OutgoingMidiBody>(sub_body_view);
  case Type::SysEx:
    return std::make_shared<SysExMidiBody>(sub_body_view);
  case Type::Unkown:
  default:
    return std::make_shared<UnkownBody>(sub_body_view);
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
  for (auto const &[key, val] : int16_to_type_map()) {
    if (val == type) {
      return key;
    }
  }
  // not found
  return 0;
}

IncommingMidiBody::IncommingMidiBody(BufferView<std::byte *> buffer_view)
    : Body(Body::Type::IncommingMidi,
           BODY_HEADER_SIZE + buffer_view.distance()) {
  int step = 0;
  for (auto iter : buffer_view) {
    switch (step) {
    case 0:
      m_device = iter;
      step++;
      break;
    case 1:
      if (iter != DELIMITER) {
        throw std::invalid_argument("Delimiter expected");
      }
      step++;
      break;
    case 2:
      m_message.bytes.push_back(static_cast<unsigned char>(iter));
      break;
    default:
      throw std::out_of_range("Unexpected step handled");
    }
  }
  m_message.bytes.pop_back();
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
  for (int i = 0; i < NR_OF_SUPPORTED_INCOMMINGMIDI_DEVICES; i++) {
    if (m_device == std::byte(INCOMMING_MIDI_DEVICE_BASE + i)) {
      return i;
    }
  }

  throw std::invalid_argument("Could not determine midi device index");
}

OutgoingMidiBody::OutgoingMidiBody(
    std::byte device, const std::vector<libremidi::message> &messages)
    : Body(Body::Type::OutgoingMidi, 0), m_device(device),
      m_messages(messages) {
  size_t tmp_size = 0;
  tmp_size += BODY_HEADER_SIZE;
  tmp_size += 3;
  for (auto const &iter : m_messages) {
    tmp_size += iter.size();
  }
  set_size(tmp_size);
}

OutgoingMidiBody::OutgoingMidiBody(BufferView<std::byte *> buffer_view)
    : Body(Body::Type::OutgoingMidi,
           BODY_HEADER_SIZE + buffer_view.distance()) {
  int step = 0;
  libremidi::midi_bytes tmp_midi_bytes;

  for (const auto &iter : buffer_view) {
    switch (step) {
    case 0:
      m_device = iter;
      step++;
      break;
    case 1:
      if (iter != DELIMITER) {
        throw std::invalid_argument("Delimiter expected");
      }
      step++;
      break;
    case 2:
      step++;
      break;
    case 3:
      tmp_midi_bytes.push_back(static_cast<unsigned char>(iter));
      if (tmp_midi_bytes.size() == 3) {
        m_messages.emplace_back(tmp_midi_bytes, 0);
        tmp_midi_bytes.clear();
      }
      break;
    default:
      throw std::out_of_range("Unexpected step handled");
    }
  }
}

std::vector<std::byte> OutgoingMidiBody::serialize() {
  auto tmp = std::vector<std::byte>();
  auto body_header = Body::serialize();
  tmp.insert(tmp.end(), body_header.begin(), body_header.end());
  tmp.push_back(m_device);
  tmp.push_back(DELIMITER);
  tmp.push_back(std::byte(m_messages.size()));
  for (const auto &iter : m_messages) {
    for (const auto &iterr : iter) {
      tmp.push_back(std::byte(iterr));
    }
  }
  return tmp;
}

SysExMidiBody::SysExMidiBody(BufferView<std::byte *> buffer_view)
    : Body(Body::Type::SysEx, BODY_HEADER_SIZE + buffer_view.distance()) {
  int step = 0;
  size_t sysex_length = 0;
  for (const auto &iter : buffer_view) {
    switch (step) {
    case 0:
      m_device = iter;
      step++;
      break;
    case 1:
    case 3:
      if (iter != DELIMITER) {
        throw std::invalid_argument("Delimiter expected");
      }
      step++;
      break;
    case 2:
      sysex_length = static_cast<size_t>(iter);
      step++;
      break;
    case 4: {
      m_message.bytes.push_back(static_cast<unsigned char>(iter));
      break;
    default:
      throw std::out_of_range("Unexpected step handled");
    }
    }
  }
  if (m_message.size() != sysex_length) {
    throw std::invalid_argument("Sysex body has an unexpected length of:" +
                                std::to_string(m_message.size()));
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

Package::Package(BufferView<std::byte *> buffer_view)
    : m_header(
          BufferView(buffer_view.begin(), buffer_view.begin() + HEADER_SIZE)),
      m_body(Body::create(
          BufferView(buffer_view.begin() + HEADER_SIZE,
                     buffer_view.begin() + HEADER_SIZE +
                         static_cast<int64_t>(m_header.get_body_size())))) {}

std::vector<std::byte> Package::serialize() {
  std::vector<std::byte> tmp = m_header.serialize();
  auto body_vec = m_body->serialize();
  tmp.insert(tmp.end(), body_vec.begin(), body_vec.end());
  return tmp;
}

std::byte Package::index_to_midi_device_byte(int index) {
  return std::byte(OUTGOING_MIDI_DEVICE_BASE + index);
}

std::vector<std::byte> UnkownBody::serialize() {
  auto body_header = Body::serialize();
  std::vector<std::byte> tmp;
  tmp.insert(tmp.end(), body_header.begin(), body_header.end());
  tmp.insert(tmp.end(), m_content.begin(), m_content.end());
  return tmp;
}
} // namespace sls3mcubridge::tcp
