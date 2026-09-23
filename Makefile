.DEFAULT_GOAL := all

ifeq ($(origin CC),default)
CC := clang
endif
SDL_PREFIX ?= /opt/homebrew
SDL_CONFIG ?= $(shell if test -x "$(SDL_PREFIX)/bin/sdl2-config"; then echo "$(SDL_PREFIX)/bin/sdl2-config"; else command -v sdl2-config; fi)
MODE ?= debug
BUILD_DIR := build/$(MODE)
TARGET := $(BUILD_DIR)/tetris
TEST_TARGET := $(BUILD_DIR)/test_lifecycle
SOURCES := src/main.c src/game.c src/input.c src/renderer.c
OBJECTS := $(SOURCES:src/%.c=$(BUILD_DIR)/%.o)
SDL_CFLAGS = $(shell "$(SDL_CONFIG)" --cflags)
SDL_LIBS = $(shell "$(SDL_CONFIG)" --libs)
WARNINGS := -Wall -Wextra -Wpedantic -Werror
ARCH_FLAGS :=
ifeq ($(shell uname -s),Darwin)
ARCH ?= arm64
ARCH_FLAGS := -arch $(ARCH)
endif
ifeq ($(MODE),debug)
MODE_FLAGS := -O0 -g
else ifeq ($(MODE),release)
MODE_FLAGS := -O2
else ifeq ($(MODE),sanitize)
MODE_FLAGS := -O0 -g -fsanitize=address,undefined -fno-omit-frame-pointer
else
$(error Unknown MODE '$(MODE)'; use debug, release or sanitize)
endif
COMPILE_FLAGS = -std=c17 $(WARNINGS) $(ARCH_FLAGS) $(MODE_FLAGS) $(CFLAGS)

.PHONY: all run debug release sanitize test clean check-sdl
all: $(TARGET)

check-sdl:
	@test -n "$(SDL_CONFIG)" && test -x "$(SDL_CONFIG)" || { echo 'SDL2 not found. Install with: brew install sdl2 (or set SDL_CONFIG).'; exit 1; }

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: src/%.c | check-sdl $(BUILD_DIR)
	$(CC) $(CPPFLAGS) -Iinclude $(SDL_CFLAGS) $(COMPILE_FLAGS) -MMD -MP -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(COMPILE_FLAGS) $(LDFLAGS) $^ $(SDL_LIBS) $(LDLIBS) -o $@

$(TEST_TARGET): tests/test_lifecycle.c $(BUILD_DIR)/game.o $(BUILD_DIR)/input.o include/game.h include/input.h | check-sdl
	$(CC) $(CPPFLAGS) -Iinclude $(SDL_CFLAGS) $(COMPILE_FLAGS) $(LDFLAGS) tests/test_lifecycle.c $(BUILD_DIR)/game.o $(BUILD_DIR)/input.o $(SDL_LIBS) $(LDLIBS) -o $@

run: $(TARGET)
	./$(TARGET)

debug:
	$(MAKE) MODE=debug all

release:
	$(MAKE) MODE=release all

sanitize:
	$(MAKE) MODE=sanitize test

test: $(TARGET) $(TEST_TARGET)
	./$(TEST_TARGET)
	SDL_VIDEODRIVER=dummy ./$(TARGET) --smoke-test

clean:
	rm -rf build

-include $(OBJECTS:.o=.d)
