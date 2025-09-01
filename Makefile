# Makefile - debug build for simulator.cpp
CXX := g++
SRC := simulator.cpp
BIN := binary 

# Flags de debug (ajuste se preferir)
CXXFLAGS := -std=c++17 -g -O2 -Wall -Wextra -DLOCAL_DEBUG -fsanitize=address,undefined

# Argumentos para a execução: make run RUNARGS="arg1 arg2"
RUNARGS ?=

.PHONY: all build run clean

all: build

build: $(BIN)

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

run: build
	@echo "Running ./$(BIN) $(RUNARGS)"
	./$(BIN) $(RUNARGS)

clean:
	-rm -f $(BIN)

