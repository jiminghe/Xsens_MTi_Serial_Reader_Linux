#include "SerialHandler.h"
#include "XbusPacket.h"
#include "DataPacketParser.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

using namespace std;

int main()
{
    try
    {
        SerialHandler serial("/dev/ttyUSB0", LibSerial::BaudRate::BAUD_115200);
        XbusPacket packet;

        std::vector<uint8_t> goToConfig = {0xFA, 0xFF, 0x30, 0x00};
        std::vector<uint8_t> goToMeasurement = {0xFA, 0xFF, 0x10, 0x00};
        serial.sendWithChecksum(goToConfig);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  //sleep for 0.1sec
        serial.sendWithChecksum(goToMeasurement);

        std::cout << "Listening for packets..." << std::endl;

        while (true)
        {
            char byte = serial.readByte();
            packet.feedByte(static_cast<uint8_t>(byte));

            if (packet.isPacketComplete())
            {
                if (packet.validateChecksum())
                {
                    Xbus xbusData;
                    std::vector<uint8_t> rawData = packet.getRawData();
                    // Create an instance of the Xbus struct
                    DataPacketParser::parseDataPacket(rawData, xbusData); // Parse the raw packet data


                    if (xbusData.sampleTimeFineAvailable)
                    {

                        printf("sampleTimeFine: %u, ", xbusData.sampleTimeFine);

                    }

                    if (xbusData.utcTimeAvailable)
                    {
                        printf("utctime epochSeconds: %.9f\n", xbusData.utcTime);
                    }

                    if (xbusData.eulerAvailable)
                    {

                        printf("Roll, Pitch, Yaw: [%.2f, %.2f, %.2f], ", xbusData.euler[0], xbusData.euler[1], xbusData.euler[2]);

                    }

                    if (xbusData.quaternionAvailable)
                    {
                        printf("q0, q1, q2, q2: [%.4f, %.4f, %.4f, %.4f], ", xbusData.quat[0], xbusData.quat[1], xbusData.quat[2], xbusData.quat[3]);
                    }

                    if (xbusData.rotAvailable)
                    {
                        float rateOfTurnDegree[3];
                        //RateOfTurn is in radians/sec, convert to degrees/sec.
                        rateOfTurnDegree[0] = xbusData.rad2deg * xbusData.rot[0];
                        rateOfTurnDegree[1] = xbusData.rad2deg * xbusData.rot[1];
                        rateOfTurnDegree[2] = xbusData.rad2deg * xbusData.rot[2];

                        printf("RateOfTurn: [%.2f, %.2f, %.2f], ", rateOfTurnDegree[0], rateOfTurnDegree[1], rateOfTurnDegree[2]);
                    }

                    if (xbusData.accAvailable)
                    {
                        printf("Acceleration: [%.2f, %.2f, %.2f], ", xbusData.acc[0], xbusData.acc[1], xbusData.acc[2]);

                    }

                    if (xbusData.magAvailable)
                    {
                        printf("Magnetic Field: [%.2f, %.2f, %.2f]\n", xbusData.mag[0], xbusData.mag[1], xbusData.mag[2]);

                    }

                    if (xbusData.latlonAvailable && xbusData.altitudeAvailable)
                    {
                        printf("Lat, Lon, ALt: [%.9f, %.9f, %.9f]\n", xbusData.latlon[0], xbusData.latlon[1], xbusData.altitude);
                    }

                    if (xbusData.velocityAvailable)
                    {
                        printf("Vel E, N, U: [%.9f, %.9f, %.9f]\n", xbusData.vel[0], xbusData.vel[1], xbusData.vel[2]);
                    }

                    printf("\n");

                    packet.reset();
                }
                else
                {
                    std::cerr << "Checksum invalid!" << std::endl;
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
