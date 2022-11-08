# Optimize, turn on additional warnings
#CFLAGS=-O3 -pthread -lpthread -msse -msse2 -msse3 -msse4-msse4.1 -msse4.2 -mavx -mavx2 -mlzcnt -mbmi2 -madx -mabm -g -Wall -Wextra -no-pie -D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fsanitize=leak #with debug
CFLAGS=-O3 -pthread -lpthread -msse -msse2 -msse3 -msse4 -msse4.1 -msse4.2 -mavx -mavx2 -mlzcnt -mbmi2 -madx -mabm -Wall -Wextra -no-pie -g -Wall -Wextra #Middle
#CFLAGS=-O3 -pthread -msse -msse2 -msse3 -msse4 -msse4.1 -msse4.2 -mavx -mavx2 -mlzcnt -mbmi2 -madx -mabm -Wall -Wextra -no-pie #Fast

.PHONY: all
all: main
main: src/main.c src/mul.c src/BigInt.c src/BigIntAsm.S src/mul.c src/mulAsm.S test/tests.c src/util.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f main
