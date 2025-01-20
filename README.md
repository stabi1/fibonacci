# Fibonacci
Compute the nth fibonacci number. Optimized for speed with my own bignum implementation written in C and Assembly-x86


## Memory
Single thread fibonacci takes up around 5.5 times as much memory as the size of the predicted result.
Single thread conversion to decimal takes around 9 times as much memory as the size of the predicted result.

Example for n=500000000; the result size is 40 MB:
* Then peak memory usage is around 220 MB
* Then peak memory usage for the conversion to decimal is 360 MB

TMP: ./fib -o n -v --do-swap --swap-threshold 1 -n 500000000


## Requirements
The OS must be Linux! \
cpu features needed:
* bmi2


## Tests:
Run all tests: `make test` \
Run all valgrind tests: `pytest -m valgrind_test` \
Run all non valgrind tests (fast): `pytest -m "not valgrind_test"`



## Misc
* Max length = ~2000 petabytes (64bit bit-length counter)


# TMP:
valgrind ./fib -o n -v -n 5000000
No cache-> total heap usage: 680,772 allocs, 680,772 frees, 482,708,078 bytes allocated
BigIntStruct cache-> total heap usage: 304,186 allocs, 304,186 frees, 467,644,846 bytes allocated
All cache-> total heap usage: 35,252 allocs, 35,252 frees, 127,921,638 bytes allocated

valgrind ./fib -o n -v -n 20000000
1k cache -> total heap usage: 161,772 allocs, 161,772 frees, 975,581,332 bytes allocated
10k cache -> total heap usage: 124,497 allocs, 124,497 frees, 264,087,596 bytes allocated
100K cache -> total heap usage: 121,847 allocs, 121,847 frees, 38,345,708 bytes allocated