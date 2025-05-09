# Fibonacci
Compute the nth fibonacci number. Optimized for speed with my own bignum implementation written in C and Assembly-x86


## Memory
Single thread fibonacci takes up around 5.5 times as much memory as the size of the predicted result.
Single thread conversion to decimal takes around 9 times as much memory as the size of the predicted result.

Example for n=500000000; the result size is 40 MB:
* Then peak memory usage is around 220 MB
* Then peak memory usage for the conversion to decimal is 360 MB


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
* Max ssaMax length = ~576 petabytes (log2(2*bitlength)<63)
* BigInt Division uses truncating division
