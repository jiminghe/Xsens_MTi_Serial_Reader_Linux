# Xsens_MTi_Serial_Reader_Linux
Use C++ Serial library([libserial](https://github.com/crayzeewulf/libserial)) to config/read data from Xsens MTi.

Tested on ubuntu 20.04 with MTi-3-DK.

Install the dependency libary

ubuntu
```
sudo apt install libserial-dev
```
then
```
make
```

change the port and baudrate in the main.cpp to your setup:
```
SerialHandler serial("/dev/ttyUSB0", LibSerial::BaudRate::BAUD_115200);
```

run the program:
```
./mti_serial
```
