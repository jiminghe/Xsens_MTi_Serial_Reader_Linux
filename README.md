# Xsens_MTi_Serial_Reader_Linux
Use C++ Serial library([libserial](https://github.com/crayzeewulf/libserial)) to config/read data from Xsens MTi.

Tested on ubuntu 20.04 with MTi-3-DK, and MTi-680G.

Install the dependency libary

ubuntu 20
```
sudo apt install libserial-dev
```
note: for ubuntu 18, you need to build from source following [this instruction](https://github.com/crayzeewulf/libserial#developers)

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
