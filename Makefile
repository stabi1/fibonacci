NEEDFLAGS=-O3 -pthread -lpthread -msse -msse2 -msse3 -msse4 -msse4.1 -msse4.2 -mavx -mavx2 -mlzcnt -mbmi2 -madx -mabm -no-pie
WARNINGFLAGS=-Wall -Wextra
DEGUGFLAGS=-g
SANITIZERFLAGS=-D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fsanitize=leak

SOURCEFILES=src/main.c src/mul.c src/BigInt.c src/BigIntAsm.S src/mul.c src/mulAsm.S test/tests.c src/util.c

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
	rm -f main
