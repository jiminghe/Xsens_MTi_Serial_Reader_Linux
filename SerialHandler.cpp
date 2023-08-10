#include "SerialHandler.h"
#include <iostream>
#include <iomanip>

SerialHandler::SerialHandler(const std::string& portName, 
                             LibSerial::BaudRate baudRate)
    : m_portName(portName), m_baudRate(baudRate)
{
    try
    {
        m_serialStream.Open(m_portName);

        m_serialStream.SetBaudRate(m_baudRate);
        m_serialStream.SetCharacterSize(LibSerial::CharacterSize::CHAR_SIZE_8);
        m_serialStream.SetFlowControl(LibSerial::FlowControl::FLOW_CONTROL_NONE);
        m_serialStream.SetParity(LibSerial::Parity::PARITY_NONE);
        m_serialStream.SetStopBits(LibSerial::StopBits::STOP_BITS_1);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

SerialHandler::~SerialHandler()
{
    if (m_serialStream.IsOpen())
        m_serialStream.Close();
}

char SerialHandler::readByte()
{
    char byte;
    m_serialStream.get(byte);

    if (!m_serialStream.good())
        throw std::runtime_error("Failed to read from the serial port.");

    return byte;
}

void SerialHandler::sendBytes(const std::vector<uint8_t>& bytes) {
    // Debugging: Print the bytes in hex format
    std::cout << "Sending bytes: ";
    for (uint8_t byte : bytes) {
        std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
    std::cout << std::endl;

    // Send the bytes
    m_serialStream.write(reinterpret_cast<const char*>(&bytes[0]), bytes.size());
    m_serialStream.flush();
}


void SerialHandler::sendWithChecksum(const std::vector<uint8_t>& bytes) {
    std::vector<uint8_t> messageWithChecksum = bytes;  // copy the original message
    uint8_t checksum = 0;
    
    for (size_t i = 1; i < bytes.size(); ++i) {  // starting from index 2 to exclude the preamble
        checksum += bytes[i];  // calculate the checksum
    }
    checksum = checksum & 0xff;
    if(checksum != 0)
    {
        checksum = (256 - checksum) & 0xff;
    }

    messageWithChecksum.push_back(checksum);  // append the checksum to the end

    sendBytes(messageWithChecksum);  // send the complete message
}
