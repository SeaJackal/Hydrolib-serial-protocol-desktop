#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
#include <unistd.h>

#include "hydrolib_return_codes.hpp"
#include "serial_port_stream.hpp"

#include "hydrolib_bus_application_master.hpp"
#include "hydrolib_bus_datalink_stream.hpp"
#include "hydrolib_log_distributor.hpp"

#define RING_BUFFER_CAPACITY 2048
#define PUBLIC_MEMORY_LENGTH 20

class TestLogStream
{
};

int write([[maybe_unused]] TestLogStream &stream, const void *dest,
          unsigned length)
{
    for (unsigned i = 0; i < length; i++)
    {
        std::cout << (reinterpret_cast<const char *>(dest))[i];
    }
    return length;
}

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

    opt = getopt(argc, argv, "-h");

    if (opt != 1)
    {
        if (opt == 'h')
        {
            cout << "hydrosp [-h] write/read" << endl;
            cout << "\twrite - write to slave's memory" << endl;
            cout << "\tread - read from slave's memory" << endl;
            return 0;
        }
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
    int length = 4;
    as_int = false;
    char dev[50] = {0};

    while ((opt = getopt(argc, argv, ":s:r:d:l:ih")) != -1)
    {
        switch (opt)
        {
        case 'h':
            cout << "hydrosp read [-h] -s <slave> -r <reg> [-i] [-l <length>] -d <device>" << endl;
            cout << "\tslave - slave address on bus" << endl;
            cout << "\treg - address of register" << endl;
            cout << "\ti - value as integer (4 bytes)" << endl;
            cout << "\tlength - number of bytes (max = 4) to read" << endl;
            cout << "\tdevice - file device address" << endl;
            return 0;
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
        case 'l':
            if (!StrToInt(optarg, length))
            {
                length = -1;
            }
            break;
        case 'i':
            as_int = true;
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

    if (length == -1)
    {
        cout << "No length specified" << endl;
        return -1;
    }

    char buffer[256];
    SerialPortStream transeiver(dev);
    int fd = transeiver.GetFileDescriptor();
    if (fd < 0)
    {
        cout << "Can't open device" << endl;
        return -1;
    }
    hydrolib::bus::datalink::StreamManager stream_manager(2, fd, logger);
    hydrolib::bus::datalink::Stream stream(stream_manager, slave);
    hydrolib::bus::application::Master master(stream, logger);

    master.Read(buffer, reg, length);

    hydrolib::ReturnCode return_code = hydrolib::ReturnCode::NO_DATA;
    int timeout_count = 0;
    while (return_code != hydrolib::ReturnCode::OK)
    {
        this_thread::sleep_for(chrono::milliseconds(100));
        stream_manager.Process();
        return_code = master.Process();
        if (return_code == hydrolib::ReturnCode::TIMEOUT)
        {
            timeout_count++;
            cout << "Timeout, retrying: " << timeout_count << endl;
            if (timeout_count > 10)
            {
                cout << "Timeout, max retries reached" << endl;
                return -1;
            }
        }
    }

    if (as_int){
        int32_t int_buffer;
        memcpy(&int_buffer, buffer, sizeof(int32_t));
        cout << "Got value: " << int_buffer << endl;
    }
    else {
        cout << "Got value: " << buffer << endl;
    }
    return 0;
}

int ProcessWrite(int argc, char *argv[])
{
    int opt;

    int slave = -1;
    int reg = -1;
    char dev[50] = {0};
    char *value_str = nullptr;
    bool as_int = false;

    while ((opt = getopt(argc, argv, "-:s:r:d:ih")) != -1)
    {
        switch (opt)
        {
        case 'h':
            cout << "hydrosp write <-h> -s <slave> -r <reg> [-i] -d <device> <value>"
                 << endl;
            cout << "\tslave - slave address on bus" << endl;
            cout << "\treg - address of register" << endl;
            cout << "\ti - value as integer (4 bytes)" << endl;
            cout << "\tdevice - file device address" << endl;
            cout << "\tvalue - value to write (ASCII byte)" << endl;
            return 0;
        case 1:
            value_str = optarg;
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
        case 'i':
            as_int = true;
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

    if (!value_str)
    {
        cout << "No value specified" << endl;
        return -1;
    }

    SerialPortStream transeiver(dev);
    int fd = transeiver.GetFileDescriptor();
    if (fd < 0)
    {
        cout << "Can't open device" << endl;
        return -1;
    }
    hydrolib::bus::datalink::StreamManager stream_manager(2, fd, logger);
    hydrolib::bus::datalink::Stream stream(stream_manager, slave);
    hydrolib::bus::application::Master master(stream, logger);

    if (as_int)
    {
        int32_t int_value = atoi(value_str);
        master.Write(&int_value, reg, sizeof(int32_t));
    }
    else
    {
        char char_value = value_str[0];
        master.Write(&char_value, reg, sizeof(char));
    }

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
