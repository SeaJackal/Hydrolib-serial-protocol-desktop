#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
#include <unistd.h>

#include "hydrolib_common.h"
#include "serial_port_stream.hpp"

#include "hydrolib_log_distributor.hpp"
#include "hydrolib_serial_protocol_master.hpp"

#define RING_BUFFER_CAPACITY 2048
#define PUBLIC_MEMORY_LENGTH 20

class TestLogStream
{
public:
    hydrolib_ReturnCode Push(const void *data, unsigned length)
    {
        for (unsigned i = 0; i < length; i++)
        {
            std::cout << (reinterpret_cast<const char *>(data))[i];
        }
        return HYDROLIB_RETURN_OK;
    }
    hydrolib_ReturnCode Open() { return HYDROLIB_RETURN_OK; }
    hydrolib_ReturnCode Close() { return HYDROLIB_RETURN_OK; }
};

class TestPublicMemory
{
public:
    TestPublicMemory()
    {
        for (int i = 0; i < PUBLIC_MEMORY_LENGTH; i++)
        {
            memory[i] = i;
        }
    }

public:
    hydrolib_ReturnCode Read(void *write_buffer, unsigned address,
                             unsigned length)
    {
        return HYDROLIB_RETURN_OK;
    }

public:
    hydrolib_ReturnCode Write(const void *write_buffer, unsigned address,
                              unsigned length)
    {
        if (address + length > PUBLIC_MEMORY_LENGTH)
        {
            return HYDROLIB_RETURN_FAIL;
        }
        memcpy(&memory[address], write_buffer, length);
        return HYDROLIB_RETURN_OK;
    }

public:
    uint8_t memory[PUBLIC_MEMORY_LENGTH];
};

int ProcessRead(int argc, char *argv[]);
int ProcessWrite(int argc, char *argv[]);

bool StrToInt(const char *str, int &dest);

inline TestLogStream log_stream;
inline hydrolib::logger::LogDistributor distributor("[%s] [%l] %m\n",
                                                    log_stream);
inline hydrolib::logger::Logger logger("Serializer", 0, distributor);

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Not enough arguments" << endl;
        return -1;
    }

    int opt;

    opt = getopt(argc, argv, "-");

    if (opt != 1)
    {
        cout << "No command suggested" << endl;
        return -1;
    }

    if (!strcmp(optarg, "read"))
    {
        return ProcessRead(argc, argv);
    }
    else if (!strcmp(optarg, "write"))
    {
        return ProcessWrite(argc, argv);
    }
    else
    {
        cout << "Urecognized command: " << optarg << endl;
        return -1;
    }
}

int ProcessRead(int argc, char *argv[])
{
    int opt;

    int slave = -1;
    int reg = -1;
    char dev[50] = {0};

    while ((opt = getopt(argc, argv, ":s:r:d:")) != -1)
    {
        switch (opt)
        {
        case 'd':
            strcpy(dev, optarg);
            break;
        case 's':
            if (!StrToInt(optarg, slave))
            {
                slave = -1;
            }
            break;
        case 'r':
            if (!StrToInt(optarg, reg))
            {
                reg = -1;
            }
            break;
        case ':':
            cout << "No value for the option: " << optopt << endl;
            return -1;
        case '?':
            cout << "Wrong option: " << optopt << endl;
            return -1;
        default:
            cout << "Unknown error" << endl;
            return -1;
        }
    }

    if (slave == -1)
    {
        cout << "No slave specified" << endl;
        return -1;
    }

    if (reg == -1)
    {
        cout << "No reg specified" << endl;
        return -1;
    }

    if (!dev[0])
    {
        cout << "No device specified" << endl;
        return -1;
    }

    uint8_t buffer;
    SerialPortStream transeiver(dev);
    TestPublicMemory public_memory;
    int fd = transeiver.GetFileDescriptor();
    if (fd < 0)
    {
        cout << "Can't open device" << endl;
        return -1;
    }
    hydrolib::serial_protocol::Master sp_handler(1, fd, public_memory, logger,
                                                 [&]() {});

    sp_handler.TransmitRead(slave, reg, sizeof(char),
                            reinterpret_cast<uint8_t *>(&buffer));

    this_thread::sleep_for(chrono::milliseconds(500));

    if (sp_handler.ProcessRx() != HYDROLIB_RETURN_OK)
    {
        cout << "Got nothing" << endl;
        return -1;
    }
    else
    {
        cout << "Got value: " << buffer << endl;
        return 0;
    }
}

int ProcessWrite(int argc, char *argv[])
{
    int opt;

    int slave = -1;
    int reg = -1;
    char dev[50] = {0};
    char value = 0;

    while ((opt = getopt(argc, argv, "-:s:r:d:")) != -1)
    {
        switch (opt)
        {
        case 1:
            value = *optarg;
            break;
        case 'd':
            strcpy(dev, optarg);
            break;
        case 's':
            if (!StrToInt(optarg, slave))
            {
                slave = -1;
            }
            break;
        case 'r':
            if (!StrToInt(optarg, reg))
            {
                reg = -1;
            }
            break;
        case ':':
            cout << "No value for the option: " << optopt << endl;
            return -1;
        case '?':
            cout << "Wrong option: " << optopt << endl;
            return -1;
        default:
            cout << "Unknown error" << endl;
            return -1;
        }
    }

    if (slave == -1)
    {
        cout << "No slave specified" << endl;
        return -1;
    }

    if (reg == -1)
    {
        cout << "No reg specified" << endl;
        return -1;
    }

    if (!dev[0])
    {
        cout << "No device specified" << endl;
        return -1;
    }

    if (!value)
    {
        cout << "No value specified" << endl;
        return -1;
    }

    SerialPortStream transeiver(dev);
    TestPublicMemory public_memory;
    int fd = transeiver.GetFileDescriptor();
    if (fd < 0)
    {
        cout << "Can't open device" << endl;
        return -1;
    }
    hydrolib::serial_protocol::Master sp_handler(1, fd, public_memory, logger,
                                                 [&]() {});

    sp_handler.TransmitWrite(slave, reg, sizeof(char),
                             reinterpret_cast<uint8_t *>(&value));
    return 0;
}

bool StrToInt(const char *str, int &dest)
{
    int i = 0;
    while (str[i])
    {
        if (str[i] != '0')
        {
            dest = atoi(str);
            if (!dest)
            {
                return false;
            }
            return true;
        }
        else
        {
            i++;
        }
    }
    dest = 0;
    return true;
}
