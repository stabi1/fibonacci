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
The OS must be Linux!

cpu features needed:
* bmi2




# TMP:
valgrind ./fib -o n -v -n 5000000
-> total heap usage: 680,772 allocs, 680,772 frees, 482,708,078 bytes allocated
