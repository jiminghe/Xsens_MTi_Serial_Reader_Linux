#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <string>
#include <libserial/SerialPort.h>
#include <libserial/SerialStream.h>
#include <vector>


class SerialHandler {
public:
    SerialHandler(const std::string& portName, 
                  LibSerial::BaudRate baudRate = LibSerial::BaudRate::BAUD_115200);

    ~SerialHandler();

    char readByte();
    void sendBytes(const std::vector<uint8_t>& bytes);
    void sendWithChecksum(const std::vector<uint8_t>& bytes);

private:
    std::string m_portName;
    LibSerial::BaudRate m_baudRate;
    LibSerial::SerialStream m_serialStream;
};

#endif  // SERIALHANDLER_H
