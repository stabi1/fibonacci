NEEDFLAGS= -O3 -pthread -lpthread -march=native -z noexecstack
WARNINGFLAGS=-Wall -Wextra
DEBUGFLAGS=-g -no-pie
SANITIZERFLAGS=-D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fsanitize=leak

SOURCEFILES=src/main.c src/util.c src/bigInt/bigIntMul.c src/bigInt/bigIntDiv.c src/bigInt/bigIntMethods.c src/bigInt/bigIntAsm.S src/bigInt/mulAsm.S test/tests.c src/bigInt/bigIntUtil.c -lm

OUTPUT_FILENAME=fib
OUTPUT_LIBNAME=test/bigInt.so

all: main

main: $(SOURCEFILES)
	$(CC) $(NEEDFLAGS) $(WARNINGFLAGS) -o $(OUTPUT_FILENAME) $^

debug: $(SOURCEFILES)
	$(CC) $(NEEDFLAGS) $(WARNINGFLAGS) $(DEBUGFLAGS) -o $(OUTPUT_FILENAME) $^

sanitize: $(SOURCEFILES)
	$(CC) $(NEEDFLAGS) $(WARNINGFLAGS) $(DEBUGFLAGS) $(SANITIZERFLAGS) -o $(OUTPUT_FILENAME) $^

test: $(SOURCEFILES)
	$(CC) -shared -o $(OUTPUT_LIBNAME) $(NEEDFLAGS) $(WARNINGFLAGS) -fPIC $^

clean:
	rm -f $(OUTPUT_FILENAME)
	rm -f $(OUTPUT_LIBNAME)

.PHONY: all main debug sanitize test clean
