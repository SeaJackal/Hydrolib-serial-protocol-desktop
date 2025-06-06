#ifndef SERIAL_PORT_SREAM_H_
#define SERIAL_PORT_SREAM_H_

#include <cstdint>
#include <string>

#include <termios.h>

class SerialPortStream {
public:
  struct Config {
  public:
    enum class Speed { SPEED_115200 };
    enum class Parity { ODD };
    enum class StopBits { ONE };

  public:
    Speed speed;
    Parity parity;
    StopBits stop_bits;
  };

public:
  SerialPortStream(const std::string &file_path,
                   const Config &config = {Config::Speed::SPEED_115200,
                                           Config::Parity::ODD,
                                           Config::StopBits::ONE});
  ~SerialPortStream();

private:
  int fd_;
  struct termios tty_;

public:
  SerialPortStream &operator<<(uint8_t byte);
  SerialPortStream &operator>>(uint8_t &byte);

  uint32_t ReadAll(uint8_t *buffer);

  int GetFileDescriptor();

  bool IsValid() const;
};

#endif
