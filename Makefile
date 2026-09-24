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
ENGINE_OBJECTS := $(BUILD_DIR)/game.o $(BUILD_DIR)/board.o $(BUILD_DIR)/piece.o $(BUILD_DIR)/collision.o $(BUILD_DIR)/rotation.o $(BUILD_DIR)/randomizer.o $(BUILD_DIR)/scoring.o
ENGINE_TEST := $(BUILD_DIR)/test_engine
RENDER_TEST := $(BUILD_DIR)/test_renderer
ENGINE_TEST_SOURCES := $(filter-out tests/test_lifecycle.c tests/test_renderer.c,$(wildcard tests/test_*.c))
SOURCES := src/main.c src/game.c src/board.c src/piece.c src/collision.c src/rotation.c src/randomizer.c src/scoring.c src/input.c src/renderer.c src/preview.c src/text.c src/ui.c
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
# Assertions must execute even if callers supply -DNDEBUG.
TEST_FLAGS = $(COMPILE_FLAGS) -UNDEBUG
COMPILE_FLAGS = -std=c17 $(WARNINGS) $(ARCH_FLAGS) $(MODE_FLAGS) $(CFLAGS)

.PHONY: all run debug release sanitize sanitize-engine test test-engine test-sdl clean check-sdl
all: $(TARGET)

check-sdl:
	@test -n "$(SDL_CONFIG)" && test -x "$(SDL_CONFIG)" || { echo 'SDL2 not found. Install with: brew install sdl2 (or set SDL_CONFIG).'; exit 1; }

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: src/%.c | check-sdl $(BUILD_DIR)
	$(CC) $(CPPFLAGS) -Iinclude $(SDL_CFLAGS) $(COMPILE_FLAGS) -MMD -MP -c $< -o $@

$(ENGINE_OBJECTS): $(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) -Iinclude $(COMPILE_FLAGS) -MMD -MP -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(COMPILE_FLAGS) $(LDFLAGS) $^ $(SDL_LIBS) $(LDLIBS) -o $@

$(TEST_TARGET): tests/test_lifecycle.c $(ENGINE_OBJECTS) $(BUILD_DIR)/input.o $(wildcard include/*.h) Makefile | check-sdl
	$(CC) $(CPPFLAGS) -Iinclude $(SDL_CFLAGS) $(TEST_FLAGS) $(LDFLAGS) tests/test_lifecycle.c $(ENGINE_OBJECTS) $(BUILD_DIR)/input.o $(SDL_LIBS) $(LDLIBS) -o $@

$(ENGINE_TEST): $(ENGINE_TEST_SOURCES) $(ENGINE_OBJECTS) $(wildcard include/*.h) Makefile
	$(CC) $(CPPFLAGS) -Iinclude $(TEST_FLAGS) $(LDFLAGS) $(ENGINE_TEST_SOURCES) $(ENGINE_OBJECTS) $(LDLIBS) -o $@

$(RENDER_TEST): tests/test_renderer.c $(ENGINE_OBJECTS) $(BUILD_DIR)/renderer.o $(BUILD_DIR)/preview.o $(BUILD_DIR)/text.o $(BUILD_DIR)/ui.o $(wildcard include/*.h) Makefile | check-sdl
	$(CC) $(CPPFLAGS) -Iinclude $(SDL_CFLAGS) $(TEST_FLAGS) $(LDFLAGS) $(filter %.c %.o,$^) $(SDL_LIBS) $(LDLIBS) -o $@

run: $(TARGET)
	./$(TARGET)

debug:
	$(MAKE) MODE=debug all

release:
	$(MAKE) MODE=release all

sanitize:
	$(MAKE) MODE=sanitize test

sanitize-engine:
	$(MAKE) MODE=sanitize test-engine

test: test-engine test-sdl

test-engine: $(ENGINE_TEST)
	./$(ENGINE_TEST)

test-sdl: $(TARGET) $(TEST_TARGET) $(RENDER_TEST)
	./$(TEST_TARGET)
	SDL_VIDEODRIVER=dummy ./$(RENDER_TEST)
	SDL_VIDEODRIVER=dummy ./$(TARGET) --smoke-test

clean:
	rm -rf build

-include $(OBJECTS:.o=.d)
