CC = g++

CFLAGS =  -g -Wall 
CLIBFLAGS = -l curl 
CFLAGS += $$(pkg-config --cflags --libs libmongoc-1.0)
CFLAGS += -std=c++11 
TARGET = test

all: $(TARGET)

$(TARGET): $(TARGET).o
	$(CC) -o $(TARGET) $(TARGET).o $(CLIBFLAGS) $(CFLAGS)

$(TARGET).o: $(TARGET).cpp
	$(CC) -c $(TARGET).cpp $(CFLAGS)

clean:
	$(RM) $(TARGET);\
	$(RM) *.o