# Flags
NEED_FLAGS=-O3 -pthread -lpthread -march=native -z noexecstack
WARNING_FLAGS=-Wall -Wextra
LINKER_FLAGS=-lm
DEBUG_FLAGS=-g -no-pie
SANITIZER_FLAGS=-D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fsanitize=leak
TEST_FLAGS=-fPIC

# Source files
SOURCE_FILES_C=src/main.c src/util.c src/bigInt/bigIntMul.c src/bigInt/bigIntDiv.c src/bigInt/bigIntMethods.c src/bigInt/bigIntIO.c src/test/tests.c src/bigInt/bigIntUtil.c src/bigInt/config.c src/bigInt/bigIntHigherFunctions.c
SOURCE_FILES_S=src/bigInt/bigIntAsm.S src/bigInt/mulAsm.S

BUILD_ROOT=build

OUTPUT_FILENAME=fib
OUTPUT_LIBNAME=test/bigInt.so

all: main

main: TARGET_NAME=main
main: CFLAGS=$(NEED_FLAGS) $(WARNING_FLAGS)
main: $(OUTPUT_FILENAME)

debug: TARGET_NAME+=debug
debug: CFLAGS=$(NEED_FLAGS) $(WARNING_FLAGS) $(DEBUG_FLAGS)
debug: $(OUTPUT_FILENAME)

sanitize: TARGET_NAME=sanitize
sanitize: CFLAGS=$(NEED_FLAGS) $(WARNING_FLAGS) $(DEBUG_FLAGS) $(SANITIZER_FLAGS)
sanitize: $(OUTPUT_FILENAME)

test: TARGET_NAME=test
test: CFLAGS=$(NEED_FLAGS) $(WARNING_FLAGS) $(TEST_FLAGS)
test: $(OUTPUT_LIBNAME)

# BUILD_DIR=$(BUILD_ROOT)/$(TARGET_NAME) TODO
BUILD_DIR=$(BUILD_ROOT)
OBJECT_FILES=$(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SOURCE_FILES_C)) \
              $(patsubst src/%.S,$(BUILD_DIR)/%.o,$(SOURCE_FILES_S))

# Link the final executable
$(OUTPUT_FILENAME): $(OBJECT_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LINKER_FLAGS)

# Link the shared library
$(OUTPUT_LIBNAME): $(OBJECT_FILES)
	$(CC) -shared -o $@ $(CFLAGS) $^

# Compile C files
$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)  # Ensure the directory structure exists
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile assembly files
$(BUILD_DIR)/%.o: src/%.S
	@mkdir -p $(dir $@)  # Ensure the directory structure exists
	$(CC) $(CFLAGS) -c -o $@ $<

setup:
	sudo apt install python3-full python3-pip
	python3 -m venv ./venv
	pip install pytest

clean:
	rm -f $(OUTPUT_FILENAME)
	rm -f $(OUTPUT_LIBNAME)
	rm -rf $(BUILD_ROOT)

.PHONY: all main debug sanitize test clean setup
