#include "rtmidi/RtMidi.h"
#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

namespace sls3mcubridge {

class MidiDevice : public std::enable_shared_from_this<MidiDevice> {
public:
  MidiDevice(std::string name, RtMidi::Api api);
  void start_reading(std::function<void(int, std::vector<std::byte>)> callback);
  void send_message(std::vector<std::byte> message);

private:
  void handle_midi_read(double timeStamp, std::vector<unsigned char> *message);
  static void handle_midi_read_static(double timeStamp,
                                      std::vector<unsigned char> *message,
                                      void *userData);
  std::string m_name;
  RtMidiIn m_in;
  RtMidiOut m_out;
  std::function<void(int, std::vector<std::byte>)> m_read_callback;
};

} // namespace sls3mcubridge