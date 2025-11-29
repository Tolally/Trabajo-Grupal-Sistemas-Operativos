CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
BIN_DIR = bin
SRC_DIR = src

TARGET = $(BIN_DIR)/analisis_rendimiento
SRC = $(SRC_DIR)/analisis_rendimiento.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
