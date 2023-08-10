#ifndef XBUSPACKET_H
#define XBUSPACKET_H

#include <vector>
#include <cstdint>

class XbusPacket {
public:
    XbusPacket();

    // Add a byte to the internal buffer and update state accordingly.
    void feedByte(uint8_t byte);

    // Check if a complete and valid packet has been received.
    bool isPacketComplete() const;

    // Check if the packet's checksum is valid.
    bool validateChecksum() const;  // Make this method public.

    // Reset the state of the packet parser.
    void reset();

    // Get the data bytes of the packet if it's complete and valid.
    std::vector<uint8_t> getData() const;
    const std::vector<uint8_t>& getRawData() const { return m_buffer; }

private:
    uint8_t computeChecksum(const std::vector<uint8_t>& packet) const;

    std::vector<uint8_t> m_buffer;
    bool m_extendedLength;
    size_t m_expectedLength;
    bool m_lengthValid; // New member to track the validity of m_expectedLength
};

#endif
