# Flags
NEED_FLAGS=-pthread -lpthread -march=native -z noexecstack
PERFORMANCE_FLAGS=-O3 -flto -fuse-linker-plugin
WARNING_FLAGS=-Wall -Wextra
LINKER_FLAGS=-lm
DEBUG_FLAGS=-O1 -g -no-pie
SANITIZER_FLAGS=-D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fsanitize=leak
TEST_FLAGS=-fPIC -g

# Source files
SOURCE_FILES_C=src/main.c src/util.c src/bigInt/bigIntMul.c src/bigInt/bigIntAlloc.c src/bigInt/bigIntDiv.c src/bigInt/bigIntMethods.c src/bigInt/bigIntIO.c src/bigInt/bigIntUtil.c src/bigInt/config.c src/bigInt/bigIntHigherFunctions.c src/bigInt/misc.c src/test/tests.c src/test/parseTestArgs.c src/test/testMethods.c src/bigDec/bigDecString.c
SOURCE_FILES_S=src/bigInt/bigIntAsm.S src/bigInt/mulAsm.S

BUILD_ROOT=build
SWAP_DIR=swap_storage

OUTPUT_FILENAME=fib
OUTPUT_LIBNAME_TEST=pytest/bigInt.so
OUTPUT_FILENAME_TEST=pytest/fib

all: main

main: TARGET_NAME=main
main: CFLAGS=$(NEED_FLAGS) $(PERFORMANCE_FLAGS) $(WARNING_FLAGS)
main: $(OUTPUT_FILENAME)

debug: TARGET_NAME+=debug
debug: CFLAGS=$(NEED_FLAGS) $(WARNING_FLAGS) $(DEBUG_FLAGS)
debug: $(OUTPUT_FILENAME)

sanitize: TARGET_NAME=sanitize
sanitize: CFLAGS=$(NEED_FLAGS) $(WARNING_FLAGS) $(DEBUG_FLAGS) $(SANITIZER_FLAGS)
sanitize: $(OUTPUT_FILENAME)

test: TARGET_NAME=test
test: CFLAGS=$(NEED_FLAGS) $(WARNING_FLAGS) $(TEST_FLAGS) $(PERFORMANCE_FLAGS)
test: $(OUTPUT_LIBNAME_TEST)
	pytest

# BUILD_DIR=$(BUILD_ROOT)/$(TARGET_NAME) TODO
BUILD_DIR=$(BUILD_ROOT)
OBJECT_FILES=$(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SOURCE_FILES_C)) \
              $(patsubst src/%.S,$(BUILD_DIR)/%.o,$(SOURCE_FILES_S))

# Link the final executable
$(OUTPUT_FILENAME): $(OBJECT_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LINKER_FLAGS)

# Link the shared library for testing
$(OUTPUT_LIBNAME_TEST): $(OBJECT_FILES)
	$(CC) -shared -o $@ $(CFLAGS) $^
	$(CC) $(CFLAGS) -o $(OUTPUT_FILENAME_TEST) $^ $(LINKER_FLAGS)

# Compile C files
$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)  # Ensure the directory structure exists
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile assembly files
$(BUILD_DIR)/%.o: src/%.S
	@mkdir -p $(dir $@)  # Ensure the directory structure exists
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

clean:
	rm -f $(OUTPUT_FILENAME)
	rm -f $(OUTPUT_LIBNAME_TEST)
	rm -f $(OUTPUT_FILENAME_TEST)
	rm -rf $(BUILD_ROOT)
	rm -rf $(SWAP_DIR)

.PHONY: all main debug sanitize test clean setup
