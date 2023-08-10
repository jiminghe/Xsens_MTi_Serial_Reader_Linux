#include "DataPacketParser.h"
#include <string.h>
#include <cstdint>
#include <arpa/inet.h>
#include <iostream>

void DataPacketParser::parseDataPacket(const std::vector<uint8_t> &packet, Xbus &xbusData)
{
    size_t offset = 2; // packet = FA FF 36 16 10 60 04 09 3A 7C B7 20 30 0C BF 49 1A 52 BE C3 6A 10 C3 2E E2 EF 3E

    // Check for MTData2 preamble
    if (packet[offset] == 0x36)
    {
        offset++;                              // move past MTData2 identifier
        uint8_t dataLengthMT = packet[offset]; // data length for MTData2
        offset++;                              // move to the first data type in MTData2

        size_t endMTData2 = offset + dataLengthMT; // Calculate the end of the MTData2 block

        while (offset < endMTData2)
        {
            offset += 2; // Move past the 2-byte dataType
            uint8_t dataLength = packet[offset];

            offset++;                                                      // Move past dataLength byte
            uint8_t *dataPtr = const_cast<uint8_t *>(&packet[offset - 3]); // Adjust this to point to the start of dataType
            parseMTData2(&xbusData, dataPtr, dataLength + 3);

            offset += dataLength; // Move past the data to the next data type
        }
    }
}

void DataPacketParser::dataswapendian(uint8_t *data, int len)
{
    uint8_t cpy[len];
    memcpy(cpy, data, len);
    for (int i = 0; i < len / 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            data[j + i * 4] = cpy[3 - j + i * 4];
        }
    }
}

double DataPacketParser::parseFP1632(const uint8_t *data)
{
    int32_t fpfrac;
    int16_t fpint;

    // Note: memcpy is used to prevent issues caused by alignment requirements
    memcpy(&fpfrac, data, sizeof(fpfrac));
    memcpy(&fpint, data + 4, sizeof(fpint));

    // Convert the big-endian to the host's byte order
    fpfrac = ntohl(fpfrac);
    fpint = ntohs(fpint);

    int64_t fp_i64 = (static_cast<int64_t>(fpint) << 32) | (static_cast<int64_t>(fpfrac) & 0xffffffff);

    double rv_d = static_cast<double>(fp_i64) / 4294967296.0;
    return rv_d;
}

void DataPacketParser::parseMTData2(Xbus *self, uint8_t *data, uint8_t datalength)
{
    int offset = 0;

    while (offset < datalength)
    {
        uint16_t dataId = (static_cast<uint16_t>(data[offset]) << 8) | data[offset + 1];
        offset += 3; // i.e 10 60 04 0a 14 33 17  ==> 0a 14 33 17

        switch (dataId)
        {
        case 0x1060: // Sample time fine
            dataswapendian(&data[offset], 4);
            self->sampleTimeFine = *reinterpret_cast<uint32_t *>(&data[offset]);
            self->sampleTimeFineAvailable = true;
            offset += 4;
            break;
        case 0x1010: // UTC Time stamp
        {
            dataswapendian(&data[offset], 4);
            uint32_t nanosec = *reinterpret_cast<uint32_t *>(&data[offset]);
            offset += 4;

            dataswapendian(&data[offset], 2);
            uint16_t year = *reinterpret_cast<uint16_t *>(&data[offset]);
            offset += 2;

            uint8_t month = *reinterpret_cast<uint8_t *>(&data[offset]);
            offset++;

            uint8_t day = *reinterpret_cast<uint8_t *>(&data[offset]);
            offset++;

            uint8_t hour = *reinterpret_cast<uint8_t *>(&data[offset]);
            offset++;

            uint8_t minute = *reinterpret_cast<uint8_t *>(&data[offset]);
            offset++;

            uint8_t sec = *reinterpret_cast<uint8_t *>(&data[offset]);
            offset++;

            // uint8_t flags = *reinterpret_cast<uint8_t *>(&data[offset]);
            offset++;
            self->utcTimeInfo.tm_year = year - 1900; // Years since 1900
            self->utcTimeInfo.tm_mon = month - 1;    // Months since January
            self->utcTimeInfo.tm_mday = day;
            self->utcTimeInfo.tm_hour = hour;
            self->utcTimeInfo.tm_min = minute;
            self->utcTimeInfo.tm_sec = sec;
            // Convert to time_t (seconds since 1st Jan 1970)
            self->utcTime = static_cast<double>(timegm(&self->utcTimeInfo)) + static_cast<double>(nanosec) * 1e-9;
            self->utcTimeAvailable = true;
            break;
        }
        case 0x2030: // Euler Angles
            dataswapendian(&data[offset], 12);
            memcpy(self->euler, &data[offset], 12);
            self->eulerAvailable = true;
            offset += 12;
            break;
        case 0x2010: // Quaternion.
            dataswapendian(&data[offset], 16);
            memcpy(self->quat, &data[offset], 16);
            self->quaternionAvailable = true;
            self->convertQuatToEuler();
            offset += 16;
            break;
        case 0x4020: // Acceleration
            dataswapendian(&data[offset], 12);
            memcpy(self->acc, &data[offset], 12);
            self->accAvailable = true;
            offset += 12;
            break;

        case 0x8020: // Rate of Turn
            dataswapendian(&data[offset], 12);
            memcpy(self->rot, &data[offset], 12);
            self->rotAvailable = true;
            offset += 12;
            break;

        case 0x5042: // Latitude Longitude, FP16.32
            self->latlon[0] = parseFP1632(&data[offset]);
            offset += 6;
            self->latlon[1] = parseFP1632(&data[offset]);
            self->latlonAvailable = true;
            offset += 6;
            break;
        case 0x5022: // AltitudeEllipsoid, FP16.32
            self->altitude = parseFP1632(&data[offset]);
            self->altitudeAvailable = true;
            offset += 6;
            break;
        case 0xC020: // Magnetic Field
            dataswapendian(&data[offset], 12);
            memcpy(self->mag, &data[offset], 12);
            self->magAvailable = true;
            offset += 12;
            break;
        case 0xE020: // StatusWord
            memcpy(&self->statusWord, &data[offset], 4);
            self->statusWordAvailable = true;
            offset += 4;
            break;
        case 0xD012: // Velocity, FP16.32
            self->vel[0] = parseFP1632(&data[offset]);
            offset += 6;
            self->vel[1] = parseFP1632(&data[offset]);
            offset += 6;
            self->vel[2] = parseFP1632(&data[offset]);
            offset += 6;
            self->velocityAvailable = true;
            break;
        default:
            std::cout << "Unrecognized data ID: 0x" << std::hex << dataId << " at offset " << std::dec << offset << ". Following bytes: ";
            for (int i = 0; i < std::min(static_cast<int>(datalength - offset), 5); i++)
            { // print up to next 5 bytes
                std::cout << std::hex << static_cast<int>(data[offset + i]) << " ";
            }
            std::cout << std::endl;
            break;
        }
    }
}
