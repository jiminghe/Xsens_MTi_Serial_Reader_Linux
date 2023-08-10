#ifndef DATA_PACKET_PARSER_H
#define DATA_PACKET_PARSER_H

#include <vector>
#include <cstdint>
#include <cmath>
#include <ctime>


struct Xbus {
    float euler[3];
    bool eulerAvailable = false;

    float quat[4];
    bool quaternionAvailable = false;  

    float acc[3];
    bool accAvailable = false;  

    float rot[3];
    bool rotAvailable = false;

    double latlon[2]; //FP16.32
    bool latlonAvailable = false;

    double altitude; //FP16.32
    bool altitudeAvailable = false;

    double vel[3]; //FP16.32
    bool velocityAvailable = false;

    float mag[3];
    bool magAvailable = false;

    uint16_t packetCounter;
    bool packetCounterAvailable = false;

    uint32_t sampleTimeFine;
    bool sampleTimeFineAvailable = false;

    double utcTime; //seconds since 1st Jan 1970
    struct tm utcTimeInfo = {0};
    bool utcTimeAvailable = false;

    uint32_t statusWord;
    bool statusWordAvailable = false;


    static constexpr double rad2deg = 57.295779513082320876798154814105;
    static constexpr double minusHalfPi = -1.5707963267948966192313216916397514420985846996875529104874;
    static constexpr double halfPi = 1.5707963267948966192313216916397514420985846996875529104874;

    static float asinClamped(float x) {
        if (x <= -1.0 ) return minusHalfPi;
        if (x >= 1.0) return halfPi;
        return std::asin(x);
    }


    void convertQuatToEuler() {
        if (!quaternionAvailable) {
            // Handle error: Quaternion data not available.
            return;
        }
        
        float sqw = quat[0] * quat[0];
        float dphi = 2.0 * (sqw + quat[3] * quat[3]) - 1.0;
        float dpsi = 2.0 * (sqw + quat[1] * quat[1]) - 1.0;
    
        euler[0] = std::atan2(2.0 * (quat[2] * quat[3] + quat[0] * quat[1]), dphi) * rad2deg;
        euler[1] = -asinClamped(2.0 * (quat[1] * quat[3] - quat[0] * quat[2]))* rad2deg;
        euler[2] = std::atan2(2.0 * (quat[1] * quat[2] + quat[0] * quat[3]), dpsi)* rad2deg;

        eulerAvailable = true;
    }
};

class DataPacketParser {
public:
    static void parseDataPacket(const std::vector<uint8_t>& packet, Xbus& xbusData);

private:
    static void dataswapendian(uint8_t* data, int len);
    static void parseMTData2(Xbus* self, uint8_t* data, uint8_t datalength);
    static double parseFP1632(const uint8_t* data);
};

#endif // DATA_PACKET_PARSER_H
