#pragma once

#include "libremidi/message.hpp"

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <vector>

namespace sls3mcubridge::tcp {

const int HEADER_SIZE = 6;
const std::byte HEADER_FIRST_BYTE = std::byte('U');
const std::byte HEADER_SECOND_BYTE = std::byte('C');
const std::byte HEADER_UNKOWN_BYTE = std::byte(0x01);

template <class Iterator> class BufferView {

public:
  BufferView(Iterator begin, Iterator end) : m_begin(begin), m_end(end) {}
  size_t distance() { return std::distance(m_begin, m_end); }
  [[nodiscard]] Iterator begin() const { return m_begin; }
  [[nodiscard]] Iterator end() const { return m_end; }

private:
  Iterator m_begin;
  Iterator m_end;
};

class ISerialize {
public:
  virtual ~ISerialize() = default;
  ISerialize(const ISerialize &obj) = delete;
  ISerialize(ISerialize &&obj) = delete;
  ISerialize &operator=(const ISerialize &obj) = delete;
  ISerialize &operator=(ISerialize &&obj) = delete;

  virtual std::vector<std::byte> serialize() = 0;

protected:
  ISerialize() = default;
};

class MidiDeviceIndicator {
public:
  explicit MidiDeviceIndicator(const std::byte device_byte)
      : m_device_byte(device_byte) {}
  int get_index();
  std::byte get_byte() { return m_device_byte; }

private:
  std::byte m_device_byte;
};

class Header : public ISerialize {
public:
  explicit Header(BufferView<std::byte *> buffer_view);
  explicit Header() = default;
  std::vector<std::byte> serialize() override;
  [[nodiscard]] size_t get_body_size() const { return m_body_size; }
  void set_body_size(size_t size) { m_body_size = static_cast<uint8_t>(size); }

private:
  uint8_t m_body_size = 0;
};

class Body : ISerialize {
public:
  enum Type : uint8_t {
    Unkown,
    InitialResponse,
    IncommingMidi,
    OutgoingMidi,
    SysEx,
  };

  static const int BODY_HEADER_SIZE = 4;
  std::vector<std::byte> serialize() override;

  static std::shared_ptr<Body> create(BufferView<std::byte *> buffer_view);
  [[nodiscard]] const Type &get_type() const { return m_type; }
  [[nodiscard]] const size_t &get_size() const { return m_size; }

protected:
  Body(Body::Type type, size_t size) : m_type(type), m_size(size) {}
  // TODO(ruud): find solution to remove set_size
  void set_size(const size_t &size) { m_size = size; }

private:
  static uint16_t find_type_in_map(Type type);

  Type m_type;
  size_t m_size;
};

class IncommingMidiBody : public Body {
public:
  IncommingMidiBody(const std::byte device, const libremidi::message &message)
      : Body(Body::Type::IncommingMidi, BODY_HEADER_SIZE + 3 + message.size()),
        m_device(device), m_message(message) {}
  explicit IncommingMidiBody(BufferView<std::byte *> buffer_view);
  std::vector<std::byte> serialize() override;
  int get_device_index() { return m_device.get_index(); }
  libremidi::message &get_message() { return m_message; }

private:
  MidiDeviceIndicator m_device;
  libremidi::message m_message;
};

class OutgoingMidiBody : public Body {
public:
  OutgoingMidiBody(std::byte device,
                   const std::vector<libremidi::message> &message);
  explicit OutgoingMidiBody(BufferView<std::byte *> buffer_view);
  std::vector<std::byte> serialize() override;
  int get_device_index() { return m_device.get_index(); }
  const std::vector<libremidi::message> &get_messages() { return m_messages; }

private:
  MidiDeviceIndicator m_device;
  std::vector<libremidi::message> m_messages;
};

class SysExMidiBody : public Body {
public:
  SysExMidiBody(std::byte device, const libremidi::message &message)
      : Body(Body::Type::SysEx, BODY_HEADER_SIZE + 4 + message.size()),
        m_device(device), m_message(message) {}
  explicit SysExMidiBody(BufferView<std::byte *> buffer_view);
  std::vector<std::byte> serialize() override;
  int get_device_index() { return m_device.get_index(); }
  const libremidi::message &get_message() { return m_message; }

private:
  MidiDeviceIndicator m_device;
  libremidi::message m_message;
};

class InitialResponseBody : public Body {
public:
  explicit InitialResponseBody(BufferView<std::byte *> buffer_view)
      : Body(Body::Type::InitialResponse,
             BODY_HEADER_SIZE + buffer_view.distance()),
        m_content(buffer_view.begin(), buffer_view.end()) {}
  std::vector<std::byte> serialize() override;
  std::vector<std::byte> get_content() { return m_content; }
  uint8_t get_nr_of_midi_devices();

private:
  std::vector<std::byte> m_content;
};

class UnkownBody : public Body {
public:
  explicit UnkownBody(BufferView<std::byte *> buffer_view)
      : Body(Body::Type::Unkown, BODY_HEADER_SIZE + buffer_view.distance()),
        m_content(buffer_view.begin(), buffer_view.end()) {}
  std::vector<std::byte> serialize() override;
  std::vector<std::byte> get_content() { return m_content; }

private:
  std::vector<std::byte> m_content;
};

class Package : ISerialize {
public:
  explicit Package(BufferView<std::byte *> buffer_view);
  explicit Package(std::shared_ptr<Body> &body) : m_body(body) {
    m_header.set_body_size(body->get_size());
  }
  static std::byte index_to_midi_device_byte(int index);
  std::vector<std::byte> serialize() override;
  std::shared_ptr<Body> get_body() { return m_body; }
  [[nodiscard]] size_t get_size() const {
    return HEADER_SIZE + m_body->get_size();
  }

private:
  Header m_header;
  std::shared_ptr<Body> m_body;
};
} // namespace sls3mcubridge::tcp
