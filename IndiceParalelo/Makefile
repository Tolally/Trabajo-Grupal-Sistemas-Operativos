CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -O2

INC_DIR := include
SRC_DIR := src
BIN_DIR := bin

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BIN_DIR)/%.o,$(SRCS))
TARGET := $(BIN_DIR)/indice_paralelo

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -o $@ $(OBJS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

run: all
	$(TARGET)

clean:
	rm -rf $(BIN_DIR)/*.o $(TARGET)