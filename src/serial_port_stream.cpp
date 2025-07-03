#include "serial_port_stream.hpp"

#include <fcntl.h>
#include <unistd.h>

SerialPortStream::SerialPortStream(const std::string &file_path,
                                   const Config &config)
{
    fd_ = open(file_path.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd_ < 0)
    {
        close(fd_);
        return;
    }

    if (tcgetattr(fd_, &tty_) != 0)
    {
        close(fd_);
        fd_ = -1;
        return;
    }

    switch (config.speed)
    {
    case Config::Speed::SPEED_115200:
        cfsetospeed(&tty_, B115200);
        cfsetispeed(&tty_, B115200);
        break;
    }

    tty_.c_cflag = (tty_.c_cflag & ~CSIZE) | CS8; // 8-bit characters
    tty_.c_iflag &= ~IGNBRK;                      // disable break processing
    tty_.c_lflag = 0;     // no signaling chars, no echo, no
                          // canonical processing
    tty_.c_oflag = 0;     // no remapping, no delays
    tty_.c_cc[VMIN] = 0;  // read doesn't block
    tty_.c_cc[VTIME] = 5; // 0.5 seconds read timeout

    tty_.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty_.c_cflag |= (CLOCAL | CREAD); // ignore modem controls,
                                      // enable reading

    switch (config.parity)
    {
    case Config::Parity::NONE:
        tty_.c_cflag &= !PARENB;
        break;
    case Config::Parity::ODD:
        tty_.c_cflag |= (PARENB | PARODD);
        break;
    }

    switch (config.stop_bits)
    {
    case Config::StopBits::ONE:
        tty_.c_cflag &= ~CSTOPB;
        break;
    }

    tty_.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd_, TCSANOW, &tty_) != 0)
    {
        close(fd_);
        fd_ = -1;
        return;
    }
}

SerialPortStream::~SerialPortStream() { close(fd_); }

int SerialPortStream::GetFileDescriptor() { return fd_; }

bool SerialPortStream::IsValid() const { return fd_ != -1; }
