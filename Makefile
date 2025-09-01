# Makefile - build for simulator.cpp
CXX := g++
SRC := simulator.cpp
BIN := binary

# Flags
CXX_DEBUG_FLAGS   := -std=c++17 -g -O0 -Wall -Wextra -DLOCAL_DEBUG -fsanitize=address,undefined
CXX_RELEASE_FLAGS := -std=c++17 -O2 -Wall -Wextra

# Argumentos para a execução: make debug RUNARGS="..."
RUNARGS ?=

.PHONY: all debug release clean

all: debug

debug: $(SRC)
	$(CXX) $(CXX_DEBUG_FLAGS) -o $(BIN) $(SRC)
	@echo "Running ./$(BIN) $(RUNARGS)"
	./$(BIN) $(RUNARGS)

release: $(SRC)
	$(CXX) $(CXX_RELEASE_FLAGS) -o $(BIN) $(SRC)
	@echo "Running ./$(BIN) $(RUNARGS)"
	./$(BIN) $(RUNARGS)

clean:
	-rm -f $(BIN)

clean-res:
	-rm -rf res/*.pgm \
	rm -rf res/*.mp4

plot:
	python3 res/plot.py --anim

work: clean-res release plot
	-vlc res/animation.mp4
