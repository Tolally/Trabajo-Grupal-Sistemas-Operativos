CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -O2

SRC_DIR := src
INC_DIR := include
BIN_DIR := bin

TARGET := $(BIN_DIR)/app
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $^ -o $@

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

.PHONY: clean run
clean:
	rm -rf $(BIN_DIR)

run: $(TARGET)
	set -a; . ../.env; set +a; ./bin/app
