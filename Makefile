CC = g++

CFLAGS =  -g -Wall -std=c++11 
CLIBFLAGS = -l curl

TARGET = test

all: $(TARGET)

$(TARGET): $(TARGET).o
	$(CC) -o $(TARGET) $(TARGET).o $(CLIBFLAGS)

$(TARGET).o: $(TARGET).cpp
	$(CC) -c $(TARGET).cpp $(CFLAGS)

clean:
	$(RM) $(TARGET);\
	$(RM) *.o