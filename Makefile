#Makeflags
MAKEFLAGS += --no-print-directory # to remove Entering directory output

# Flags
NEED_FLAGS = -pthread -lpthread -march=native -z noexecstack -std=c17 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE
PERFORMANCE_FLAGS = -O3 -flto=auto -fuse-linker-plugin
WARNING_FLAGS = -Wall -Wextra
LINKER_FLAGS = -lm
DEBUG_FLAGS = -O1 -g -no-pie
SANITIZER_FLAGS = -D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fsanitize=leak
TEST_FLAGS = -fPIC -g

# Automatically find source files
SOURCE_FILES_C := $(shell find src -type f -name '*.c')
SOURCE_FILES_S := $(shell find src -type f -name '*.S')

# Global build configuration (default: main)
BUILD_CONFIG ?= main
BUILD_ROOT = build
BUILD_DIR = $(BUILD_ROOT)/$(BUILD_CONFIG)

SWAP_DIR = swap_storage

OUTPUT_FILENAME = fib
OUTPUT_LIBNAME_TEST = pytest/bigInt.so
OUTPUT_FILENAME_TEST = pytest/fib

# Define object files based on BUILD_DIR
OBJECT_FILES = $(patsubst src/%.c, $(BUILD_DIR)/%.o, $(SOURCE_FILES_C)) \
               $(patsubst src/%.S, $(BUILD_DIR)/%.o, $(SOURCE_FILES_S))

# ===== Recursive Targets =====
# Wrapper targets that invoke make with the proper BUILD_CONFIG and CFLAGS

.PHONY: main debug sanitize test clean setup

all: main

main:
	@$(MAKE) all-target OUTPUT_FILENAME=$(OUTPUT_FILENAME) CFLAGS="$(NEED_FLAGS) $(PERFORMANCE_FLAGS) $(WARNING_FLAGS)" BUILD_CONFIG=main

debug:
	@$(MAKE) all-target OUTPUT_FILENAME=$(OUTPUT_FILENAME) CFLAGS="$(NEED_FLAGS) $(WARNING_FLAGS) $(DEBUG_FLAGS)" BUILD_CONFIG=debug

sanitize:
	@$(MAKE) all-target OUTPUT_FILENAME=$(OUTPUT_FILENAME) CFLAGS="$(NEED_FLAGS) $(WARNING_FLAGS) $(DEBUG_FLAGS) $(SANITIZER_FLAGS)" BUILD_CONFIG=sanitize

test:
	@$(MAKE) all-test OUTPUT_LIBNAME_TEST=$(OUTPUT_LIBNAME_TEST) CFLAGS="$(NEED_FLAGS) $(WARNING_FLAGS) $(TEST_FLAGS) $(PERFORMANCE_FLAGS)" BUILD_CONFIG=test
	pytest -n auto

# ===== Internal Targets =====
.PHONY: all-target all-test

# Link final executable using objects in the configuration-specific build directory
all-target: $(OUTPUT_FILENAME)

$(OUTPUT_FILENAME): $(OBJECT_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LINKER_FLAGS)

# Link shared library for testing
all-test: $(OUTPUT_LIBNAME_TEST)

$(OUTPUT_LIBNAME_TEST): $(OBJECT_FILES)
	$(CC) -shared -o $@ $(CFLAGS) $^
	$(CC) $(CFLAGS) -o $(OUTPUT_FILENAME_TEST) $^ $(LINKER_FLAGS)

# Pattern rule for compiling C files (handles nested directories)
$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Pattern rule for compiling Assembly files (handles nested directories)
$(BUILD_DIR)/%.o: src/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

setup:
	sudo apt install valgrind
	sudo apt install python3-full python3-pip
	python3 -m venv ./venv
	source ./venv/bin/activate
	pip3 install --upgrade pip
	pip install pytest
	pip install psutil
	pip install matplotlib
	pip install pytest-xdist
	pip install gmpy2

clean:
	rm -f $(OUTPUT_FILENAME)
	rm -f $(OUTPUT_LIBNAME_TEST)
	rm -f $(OUTPUT_FILENAME_TEST)
	rm -rf $(BUILD_ROOT)
	rm -rf $(SWAP_DIR)
