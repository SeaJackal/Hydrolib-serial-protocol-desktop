#include <iostream>
#include <chrono>
#include <thread>

#include "serial_port_stream.hpp"

#include "hydrolib_serial_protocol_pack.hpp"

class TxQueue : public hydrolib::serialProtocol::SerialProtocolHandler::TxQueueInterface
{
public:
    TxQueue(const std::string &file_path) : serial_stream_(file_path)
    {
    }

    hydrolib_ReturnCode Push(void *buffer, uint32_t length) override
    {
        for (uint32_t i = 0; i < length; i++)
        {
            serial_stream_ << static_cast<uint8_t *>(buffer)[i];
        }
        return HYDROLIB_RETURN_OK;
    }

public:
    SerialPortStream serial_stream_;
};

using namespace std;

int main(int argc, char *argv[])
{
    // if (argc < 2)
    // {
    //     cout << "Not enough arguments" << endl;
    //     return -1;
    // }

    char a;
    char b;

    uint8_t buffer[255];

    // TxQueue tx_queue(argv[1]);
    TxQueue tx_queue("/dev/ttyUSB0");

    hydrolib::serialProtocol::SerialProtocolHandler sp_handler(1, tx_queue, nullptr, 0);

    while (1)
    {
        cin >> a;
        cout << "Transmitting \"" << a << "\"" << endl;
        sp_handler.TransmitWrite(2, 0, 1, reinterpret_cast<uint8_t *>(&a));
        sp_handler.TransmitRead(2, 1, 1, reinterpret_cast<uint8_t *>(&b));

        this_thread::sleep_for(chrono::milliseconds(500));

        uint32_t read_count = tx_queue.serial_stream_.read_all(buffer);

        sp_handler.Receive(buffer, read_count);

        if (!sp_handler.ProcessRx())
        {
            cout << "Got nothing" << endl;
        }
        else
        {
            cout << "Got from memory \"" << b << "\"" << endl;
        }
    }
}
