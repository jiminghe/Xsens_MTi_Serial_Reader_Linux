#include "XbusPacket.h"

XbusPacket::XbusPacket() : m_extendedLength(false), m_lengthValid(false) {}

void XbusPacket::feedByte(uint8_t byte) {
    if (m_buffer.empty() && byte != 0xFA) {
        // Wait for preamble
        return;
    }

    m_buffer.push_back(byte);

    if (m_buffer.size() == 4 && m_buffer[3] == 0xFF) {
        m_extendedLength = true;
    }

    if (m_extendedLength && m_buffer.size() == 5) {
        m_expectedLength = (static_cast<size_t>(m_buffer[4]) << 8) | m_buffer[5];
        m_lengthValid = true;
    } else if (!m_extendedLength && m_buffer.size() == 4) {
        m_expectedLength = m_buffer[3];
        m_lengthValid = true;
    }
}

bool XbusPacket::isPacketComplete() const {
    if (!m_lengthValid) return false;

    size_t totalLength = m_extendedLength ? 7 + m_expectedLength : 5 + m_expectedLength;
    return m_buffer.size() == totalLength;
}

void XbusPacket::reset() {
    m_buffer.clear();
    m_extendedLength = false;
    m_lengthValid = false;  // Reset the validity of the length
}

bool XbusPacket::validateChecksum() const {
    if (!isPacketComplete()) return false;

    uint8_t checksum = computeChecksum(m_buffer);
    return checksum == 0;
}

std::vector<uint8_t> XbusPacket::getData() const {
    if (!isPacketComplete() || !validateChecksum()) return {};

    size_t startIdx = m_extendedLength ? 6 : 4;
    size_t endIdx = startIdx + m_expectedLength;

    return std::vector<uint8_t>(m_buffer.begin() + startIdx, m_buffer.begin() + endIdx);
}

uint8_t XbusPacket::computeChecksum(const std::vector<uint8_t>& packet) const {
    uint16_t sum = 0; // Use a 16-bit integer for summation to avoid overflow.
    for (size_t i = 1; i < packet.size(); ++i) { // Exclude the preamble
        sum += packet[i];
    }

    return static_cast<uint8_t>(sum & 0xFF); // Return the lower byte of the sum
}

