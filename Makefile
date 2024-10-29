# Name of your output binary
TARGET = mti_serial

# Automatically include all .cpp files in the current directory
SRC = $(wildcard *.cpp)

# Compiler
CC = g++

# Compiler flags
CFLAGS = -Wall -include cstddef

# Libraries to link against
LIBS = -lserial

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)
