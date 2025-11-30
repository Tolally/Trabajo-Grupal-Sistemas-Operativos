CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -O2 -Iinclude

SRCDIR := source
BINDIR := bin

TARGETS := $(BINDIR)/buscador $(BINDIR)/cache $(BINDIR)/motor

all: $(TARGETS)

$(BINDIR)/buscador: $(SRCDIR)/buscador.cpp
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BINDIR)/cache: $(SRCDIR)/cache.cpp
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BINDIR)/motor: $(SRCDIR)/motor.cpp
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

# Ejecutar con variables de entorno cargadas desde ../.env
run-motor: $(BINDIR)/motor
	@if [ -f ../.env ]; then set -a; . ../.env; set +a; fi; \
		cd .. && ./Buscador/bin/motor

run-cache: $(BINDIR)/cache
	@if [ -f ../.env ]; then set -a; . ../.env; set +a; fi; \
		cd .. && ./Buscador/bin/cache

run-buscador: $(BINDIR)/buscador
	@if [ -f ../.env ]; then set -a; . ../.env; set +a; fi; \
		cd .. && ./Buscador/bin/buscador

clean:
	rm -rf $(BINDIR)/*.o $(TARGETS)

.PHONY: all clean run-motor run-cache run-buscador