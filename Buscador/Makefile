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

clean:
	rm -rf $(BINDIR)/*.o $(TARGETS)

.PHONY: all clean