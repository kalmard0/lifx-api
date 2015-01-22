CFLAGS=-c -Wall -std=c++11 -g
LDFLAGS=
SOURCES:=$(shell find code -name "*.cpp")
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=lifx-api

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CXX) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
