CFLAGS=-c -Wall -std=c++11
LDFLAGS=
SOURCES:=$(shell find code -name "*.cpp")
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=lifx-api

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) 