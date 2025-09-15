CC := g++
CFLAGS := -std=c++17 -Wall -Wextra -O2

INC := -Iinclude

BIN_DIR := bin
SRC_DIR := src

PGM_SRCS := \
    $(SRC_DIR)/main.cpp \
    $(SRC_DIR)/interface.cpp \
    $(SRC_DIR)/validacion.cpp \
    $(SRC_DIR)/texto.cpp \
    $(SRC_DIR)/matematicas.cpp
PGM_OBJS := $(PGM_SRCS:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.o)

MULTI_SRCS := \
    $(SRC_DIR)/multi.cpp \
    $(SRC_DIR)/matematicas.cpp
MULTI_OBJS := $(MULTI_SRCS:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.o)

PGM_BIN := $(BIN_DIR)/pgm
MULTI_BIN := $(BIN_DIR)/multi

.PHONY: all clean run run-multi

CREATE_INDEX_SRC := $(SRC_DIR)/create_index.cpp
CREATE_INDEX_BIN := $(BIN_DIR)/create_index

all: $(PGM_BIN) $(MULTI_BIN) $(CREATE_INDEX_BIN)

$(CREATE_INDEX_BIN): $(CREATE_INDEX_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(CREATE_INDEX_SRC) -o $@

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(PGM_BIN): $(PGM_OBJS)
	$(CC) $(CFLAGS) $(PGM_OBJS) -o $@

$(MULTI_BIN): $(MULTI_OBJS)
	$(CC) $(CFLAGS) $(MULTI_OBJS) -o $@

run: $(PGM_BIN)
	@# Carga variables de entorno desde ../.env (raíz) y ejecuta el binario desde la raíz del repo
	@if [ -f ../.env ]; then set -a; . ../.env; set +a; fi; \
	USER_FILE=$${USER_FILE:-Menu/data/USUARIOS.txt}; \
	PERFILES_FILE=$${PERFILES_FILE:-Menu/data/PERFILES.TXT}; \
	cd .. && USER_FILE="$$USER_FILE" PERFILES_FILE="$$PERFILES_FILE" Menu/bin/pgm -u $$USER -p $$PASS -f $$FILE

run-multi: $(MULTI_BIN)
	$(MULTI_BIN) "$$A" "$$B" "$$SEP"

clean:
	rm -f $(BIN_DIR)/*.o $(PGM_BIN) $(MULTI_BIN)
