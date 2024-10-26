#pragma once

#include "libremidi/message.hpp"
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <map>
#include <memory>
#include <vector>

namespace sls3mcubridge {
namespace tcp {

const int HEADER_SIZE = 6;
const std::byte HEADEr_FIRST_BYTE = std::byte('U');
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

class Header : public ISerialize {
public:
  explicit Header(BufferView<std::byte *> buffer_view);
  explicit Header(size_t body_size)
      : m_body_size(static_cast<uint8_t>(body_size)) {}
  std::vector<std::byte> serialize() override;
  size_t get_body_size() const { return m_body_size; }

private:
  uint8_t m_body_size;
};

class Body : ISerialize {
public:
  enum Type {
    Unkown,
    IncommingMidi,
    OutgoingMidi,
    SysEx,
  };

  static const int BODY_HEADER_SIZE = 4;
  std::vector<std::byte> serialize() override;
  static inline const std::map<uint16_t, Type> int16_to_type_map = {
      {19789, Type::IncommingMidi},
      {19777, Type::OutgoingMidi},
      {21331, Type::SysEx}};

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
  int get_device_index();
  libremidi::message &get_message() { return m_message; }

private:
  std::byte m_device;
  libremidi::message m_message;
};

class OutgoingMidiBody : public Body {
public:
  OutgoingMidiBody(std::byte device,
                   const std::vector<libremidi::message> &message);
  explicit OutgoingMidiBody(BufferView<std::byte *> buffer_view);
  std::vector<std::byte> serialize() override;
  int get_device_index();
  const std::vector<libremidi::message> &get_messages() { return m_messages; }

private:
  std::byte m_device;
  std::vector<libremidi::message> m_messages;
};

class SysExMidiBody : public Body {
public:
  SysExMidiBody(std::byte device, const libremidi::message &message)
      : Body(Body::Type::SysEx, BODY_HEADER_SIZE + 4 + message.size()),
        m_device(device), m_message(message) {}
  explicit SysExMidiBody(BufferView<std::byte *> buffer_view);
  std::vector<std::byte> serialize() override;
  int get_device_index();
  const libremidi::message &get_message() { return m_message; }

private:
  std::byte m_device;
  libremidi::message m_message;
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
  explicit Package(std::shared_ptr<Body> &body)
      : m_body(body), m_header(body->get_size()) {}
  std::vector<std::byte> serialize() override;
  std::shared_ptr<Body> get_body() { return m_body; }
  size_t get_size() const { return HEADER_SIZE + m_body->get_size(); }

private:
  Header m_header;
  std::shared_ptr<Body> m_body;
};
} // namespace tcp
} // namespace sls3mcubridge