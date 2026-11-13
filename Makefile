CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O2 -Iinclude -Iinclude/characters -Iinclude/core -Iinclude/entities -Iinclude/systems
TARGET := chroniques_de_la_faille
SOURCES := main.cpp $(wildcard src/characters/*.cpp) $(wildcard src/entities/*.cpp) $(wildcard src/systems/*.cpp)

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
