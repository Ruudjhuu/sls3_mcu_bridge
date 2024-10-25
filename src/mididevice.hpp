#include <functional>
#include <memory>

#include "libremidi/libremidi.hpp"
#include "libremidi/message.hpp"

namespace sls3mcubridge {

class MidiDevice : public std::enable_shared_from_this<MidiDevice> {
public:
  explicit MidiDevice(const std::string &name);
  MidiDevice(const MidiDevice &obj) = delete;
  MidiDevice(MidiDevice &&obj) = delete;
  MidiDevice &operator=(const MidiDevice &obj) = delete;
  MidiDevice &operator=(MidiDevice &&obj) = delete;
  ~MidiDevice();
  void start_reading(
      const std::function<void(int, const libremidi::message &)> &callback);
  void send_message(const libremidi::message &message);

private:
  std::string m_name;
  libremidi::midi_out m_out;
  std::shared_ptr<libremidi::midi_in> m_in;
};

} // namespace sls3mcubridge