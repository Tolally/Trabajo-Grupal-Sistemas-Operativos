CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -O2 -Iinclude

SRCDIR := source
BINDIR := bin

TARGETS := $(BINDIR)/BUSCADOR_SistOpe $(BINDIR)/CACHE $(BINDIR)/motor

all: $(TARGETS)

$(BINDIR)/BUSCADOR_SistOpe: $(SRCDIR)/buscador.cpp
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BINDIR)/CACHE: $(SRCDIR)/cache.cpp
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BINDIR)/motor: $(SRCDIR)/motor.cpp
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

# Ejecutar con variables de entorno cargadas desde ../.env
run-motor: $(BINDIR)/motor
	@if [ -f ../.env ]; then set -a; . ../.env; set +a; fi; \
		cd .. && ./Buscador/bin/motor

run-cache: $(BINDIR)/CACHE
	@if [ -f ../.env ]; then set -a; . ../.env; set +a; fi; \
		cd .. && ./Buscador/bin/CACHE

run-buscador: $(BINDIR)/BUSCADOR_SistOpe
	@if [ -f ../.env ]; then set -a; . ../.env; set +a; fi; \
		cd .. && ./Buscador/bin/BUSCADOR_SistOpe

clean:
	rm -rf $(BINDIR)/*.o $(TARGETS)

.PHONY: all clean run-motor run-cache run-buscador
