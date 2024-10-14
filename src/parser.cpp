#include "parser.hpp"
#include <cstddef>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vector>

namespace sls3mcubridge {

const int DEVICE_BYTE_LOCATION = 10;

const unsigned char INPUT_DEVICE_ONE_BYTE = 0x6c;
const unsigned char INPUT_DEVICE_TWO_BYTE = 0x6d;
const unsigned char INPUT_DEVICE_THREE_BYTE = 0x6e;

const int TCP_MIDI_HEADER_SIZE = 12;
const int TCP_MIN_MESSAGE_SIZE = 14;

std::shared_ptr<midi_message> Parser::to_midi(unsigned char input[],
                                              size_t length) {
  if (length < TCP_MIN_MESSAGE_SIZE)
    throw std::invalid_argument("input lenght is too small.");

  auto message = std::make_shared<midi_message>();

  switch (input[DEVICE_BYTE_LOCATION]) {
  case INPUT_DEVICE_ONE_BYTE:
    message->device_number = 1;
    break;

  case INPUT_DEVICE_TWO_BYTE:
    message->device_number = 2;
    break;

  case INPUT_DEVICE_THREE_BYTE:
    message->device_number = 3;
    break;

  default:
    spdlog::error("Unkown midi device");
    throw std::runtime_error(
        "Unkown midi device number while parscing TCP message");
    break;
  }

  auto midi_body = std::make_shared<std::vector<unsigned char>>();
  for (auto cursor = &input[TCP_MIDI_HEADER_SIZE]; *cursor != '\0'; cursor++) {
    midi_body->push_back(*cursor);
  }
  message->message = midi_body;
  return message;
}

} // namespace sls3mcubridge