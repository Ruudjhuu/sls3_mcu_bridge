#pragma once

#include "libremidi/message.hpp"
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <vector>

namespace sls3mcubridge {
namespace tcp {
class ISerialize {
public:
  virtual std::vector<std::byte> serialize() = 0;
};

class Header : ISerialize {
public:
  const int HEADER_SIZE = 6;
  const std::byte FIRST_BYTE = std::byte('U');
  const std::byte SECOND_BYTE = std::byte('C');

  Header(std::byte buffer[], int &bytes_read);
  std::vector<std::byte> serialize();
  size_t get_body_size() { return m_body_size; }

private:
  std::byte m_unkown_byte;
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
  virtual std::vector<std::byte> serialize();
  static inline const std::map<uint16_t, Type> int16_to_type_map = {
      {19789, Type::IncommingMidi},
      {19777, Type::OutgoingMidi},
      {21331, Type::SysEx}};

  static std::shared_ptr<Body> create(std::byte buffer[], size_t body_size,
                                      int &bytes_read);
  Type get_type() { return m_type; }

protected:
  Body(Body::Type type, size_t size) : m_type(type), m_size(size) {}
  const Type m_type;
  size_t m_size;

private:
  Body(Body &body);
  static uint16_t find_type_in_map(Type type);
  int16_t type_int;
};

class IncommingMidiBody : public Body {
public:
  IncommingMidiBody(std::byte buffer[], size_t size, int &bytes_read);
  std::vector<std::byte> serialize();
  int get_device_index();
  libremidi::message &get_message() { return m_message; }

private:
  std::byte m_device;
  libremidi::message m_message;
};

class OutgoingMidiBody : public Body {
public:
  OutgoingMidiBody(std::byte buffer[], size_t size, int &bytes_read);
  std::vector<std::byte> serialize();
  int get_device_index();
  const std::vector<libremidi::message> &get_messages() { return m_messages; }

private:
  std::byte m_device;
  std::vector<libremidi::message> m_messages;
};

class SysExMidiBody : public Body {
public:
  SysExMidiBody(std::byte buffer[], size_t size, int &bytes_read);
  std::vector<std::byte> serialize();
  int get_device_index();
  const libremidi::message &get_message() { return m_message; }

private:
  std::byte m_device;
  libremidi::message m_message;
};

class UnkownBody : public Body {
public:
  UnkownBody(std::byte buffer[], size_t size, int &bytes_read);
  std::vector<std::byte> serialize();
  std::vector<std::byte> get_content() { return m_content; }

private:
  std::vector<std::byte> m_content;
};

class Package : ISerialize {
public:
  Package(std::byte buffer[], int &bytes_read);
  std::vector<std::byte> serialize();
  std::shared_ptr<Body> get_body() { return m_body; }

private:
  Header m_header;
  std::shared_ptr<Body> m_body;
};
} // namespace tcp
} // namespace sls3mcubridge