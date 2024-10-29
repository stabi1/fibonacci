NEEDFLAGS= -O3 -pthread -lpthread -march=native -z noexecstack -no-pie
WARNINGFLAGS=-Wall -Wextra
DEGUGFLAGS=-g
SANITIZERFLAGS=-D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fsanitize=leak

SOURCEFILES=src/main.c src/bigInt/bigIntMul.c src/bigInt/bigIntDiv.c src/bigInt/bigIntMethods.c src/bigInt/bigIntAsm.S src/bigInt/mulAsm.S test/tests.c src/bigInt/bigIntUtil.c -lm

OUTPUTFILENAME=fib

.PHONY: all
all: main

.PHONY: main
main: $(SOURCEFILES)
	$(CC) $(NEEDFLAGS) $(WARNINGFLAGS) -o $(OUTPUTFILENAME) $^

.PHONY: debug
debug: $(SOURCEFILES)
	$(CC) $(NEEDFLAGS) $(WARNINGFLAGS) $(DEGUGFLAGS) -o $(OUTPUTFILENAME) $^

.PHONY: sanitize
sanitize: $(SOURCEFILES)
	$(CC) $(NEEDFLAGS) $(WARNINGFLAGS) $(DEGUGFLAGS) $(SANITIZERFLAGS) -o $(OUTPUTFILENAME) $^

.PHONY: clean
clean:
	rm -f $(OUTPUTFILENAME)
