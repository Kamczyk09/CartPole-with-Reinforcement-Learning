CC = gcc
CFLAGS = -Wall -O2 -Ic_engine/include

LIBS_GAME = -lraylib -lm -lpthread -ldl -lrt -lX11

LIBS_LIB = -lm

all: game

# --- 1. Raw game ---
game: c_engine/src/game.c c_engine/src/physics.c
	@mkdir -p build
	$(CC) $(CFLAGS) c_engine/src/game.c c_engine/src/physics.c -o build/cartpole_game $(LIBS_GAME)
	@echo "The game has been compiled! You can run it with: ./build/cartpole_game"

# --- 2. Library for Python/RL (Headless) ---
# Note: We don't include game.c or Raylib!
lib: c_engine/src/physics.c
	@mkdir -p build
	$(CC) $(CFLAGS) -fPIC -shared c_engine/src/physics.c -o build/libcartpole.so $(LIBS_LIB)
	@echo "The library has been compiled! It's ready to be used by Python."

# --- 3. Clean ---
clean:
	rm -rf build/*
	@echo "The build folder has been cleaned."