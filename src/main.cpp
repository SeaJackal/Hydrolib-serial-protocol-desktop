#include <chrono>
#include <iostream>
#include <thread>
#include <unistd.h>

#include "hydrolib_common.h"
#include "serial_port_stream.hpp"

// extern "C" {
// #include "hydrolib_ring_queue.h"
// }
#include "hydrolib_log_distributor.hpp"
#include "hydrolib_serial_protocol_master.hpp"

#define RING_BUFFER_CAPACITY 2048
#define PUBLIC_MEMORY_LENGTH 20

// class Transeiver {
// public:
//   Transeiver(const std::string &file_path) : serial_stream_(file_path) {
//     hydrolib_RingQueue_Init(&ring_queue_, ring_buffer_,
//     RING_BUFFER_CAPACITY);
//   }

//   hydrolib_ReturnCode Push(const void *buffer, uint32_t length) {
//     for (uint32_t i = 0; i < length; i++) {
//       serial_stream_ << static_cast<const uint8_t *>(buffer)[i];
//     }
//     return HYDROLIB_RETURN_OK;
//   }

//   hydrolib_ReturnCode Read(void *data, unsigned data_length, unsigned shift)
//   {
//     return hydrolib_RingQueue_Read(&ring_queue_, data, data_length, shift);
//   }

//   void Drop(unsigned length) { hydrolib_RingQueue_Drop(&ring_queue_, length);
//   }

//   void Clear() { hydrolib_RingQueue_Clear(&ring_queue_); }

//   void Process() {
//     uint8_t rx_buffer[255];
//     unsigned length = serial_stream_.ReadAll(rx_buffer);
//     hydrolib_RingQueue_Push(&ring_queue_, rx_buffer, length);
//   }

// public:
//   SerialPortStream serial_stream_;

//   uint8_t ring_buffer_[RING_BUFFER_CAPACITY];
//   hydrolib_RingQueue ring_queue_;
// };

class TestLogStream {
public:
  hydrolib_ReturnCode Push(const void *data, unsigned length) {
    for (unsigned i = 0; i < length; i++) {
      std::cout << (reinterpret_cast<const char *>(data))[i];
    }
    return HYDROLIB_RETURN_OK;
  }
  hydrolib_ReturnCode Open() { return HYDROLIB_RETURN_OK; }
  hydrolib_ReturnCode Close() { return HYDROLIB_RETURN_OK; }
};

class TestPublicMemory {
public:
  TestPublicMemory() {
    for (int i = 0; i < PUBLIC_MEMORY_LENGTH; i++) {
      memory[i] = i;
    }
  }

public:
  const uint8_t *Read(unsigned address, unsigned length) {
    if (address + length > PUBLIC_MEMORY_LENGTH) {
      return nullptr;
    }
    return memory + address;
  }

public:
  hydrolib_ReturnCode Write(const void *write_buffer, unsigned address,
                            unsigned length) {
    if (address + length > PUBLIC_MEMORY_LENGTH) {
      return HYDROLIB_RETURN_FAIL;
    }
    memcpy(&memory[address], write_buffer, length);
    return HYDROLIB_RETURN_OK;
  }

public:
  uint8_t memory[PUBLIC_MEMORY_LENGTH];
};

inline TestLogStream log_stream;
inline hydrolib::logger::LogDistributor distributor("[%s] [%l] %m\n",
                                                    log_stream);
inline hydrolib::logger::Logger logger("Serializer", 0, distributor);

using namespace std;

int main(int argc, char *argv[]) {
  // if (argc < 2) {
  //   cout << "Not enough arguments" << endl;
  //   return -1;
  // }

  char a;
  char b;

  uint8_t buffer[255];
  // Transeiver transeiver(argv[1]);
  SerialPortStream transeiver("/dev/ttyUSB0");
  TestPublicMemory public_memory;
  int fd = transeiver.GetFileDescriptor();
  hydrolib::serial_protocol::Master sp_handler(1, fd, public_memory, logger,
                                               [&]() {});

  while (1) {
    cin >> a;
    cout << "Transmitting \"" << a << "\"" << endl;
    sp_handler.TransmitWrite(2, 0, 1, reinterpret_cast<uint8_t *>(&a));
    sp_handler.TransmitRead(2, 1, 1, reinterpret_cast<uint8_t *>(&b));

    this_thread::sleep_for(chrono::milliseconds(500));

    if (sp_handler.ProcessRx() != HYDROLIB_RETURN_OK) {
      cout << "Got nothing" << endl;
    } else {
      cout << "Got from memory \"" << b << "\"" << endl;
    }
  }
}
