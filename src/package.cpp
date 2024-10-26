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

Header::Header(BufferView<std::byte *> buffer_view) : m_body_size(0) {
  bool break_loop = false;
  int step = 0;
  for (const auto &iter : buffer_view) {
    switch (step) {
    // Header UC
    case 0:
      if (iter != HEADEr_FIRST_BYTE) {
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
      m_body_size = std::to_integer<size_t>(iter);
      step++;
      break;

    // delimiter
    case 5:
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
  buffer.push_back(HEADEr_FIRST_BYTE);
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
  auto iter = buffer_view.begin();
  for (int i = 0; i < BODY_HEADER_SIZE; i++) {
    switch (step) {
    case 0:
      type_int = static_cast<uint16_t>(*iter) << (sizeof(*iter) * CHAR_BIT);
      step++;
      break;
    case 1:
      type_int |= static_cast<uint16_t>(*iter);
      step++;
      break;
    case 2:
    case 3:
      if (*iter != DELIMITER) {
        throw std::invalid_argument("Expected delimiter at step " +
                                    std::to_string(step));
      }
      step++;
      break;
    default:
      throw std::out_of_range("Unexpected step handled");
    }
    iter++;
  }
  Type type = Type::Unkown;
  try {
    type = int16_to_type_map.at(type_int);
  } catch (std::out_of_range &) {
    type = Type::Unkown;
  }

  auto sub_body_view = BufferView(iter, buffer_view.end());

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
  for (const auto &iter : int16_to_type_map) {
    if (iter.second == type) {
      return iter.first;
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

OutgoingMidiBody::OutgoingMidiBody(BufferView<std::byte *> buffer_view)
    : Body(Body::Type::OutgoingMidi,
           BODY_HEADER_SIZE + buffer_view.distance()) {
  int step = 0;
  int nr_of_messages = 0;
  int midi_message_location = 0;
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
      nr_of_messages = static_cast<int>(iter);
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
  for (auto &iter : m_messages) {
    for (auto &itt : iter) {
      tmp.push_back(std::byte(itt));
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

std::vector<std::byte> UnkownBody::serialize() {
  auto body_header = Body::serialize();
  std::vector<std::byte> tmp;
  tmp.insert(tmp.end(), body_header.begin(), body_header.end());
  tmp.insert(tmp.end(), m_content.begin(), m_content.end());
  return tmp;
}
} // namespace sls3mcubridge::tcp
