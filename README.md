# Fibonacci

Compute the nth Fibonacci number and much more. Optimized for speed with my own bignum implementation written in C and x86 Assembly.
Please forgive the poor documentation.

### Capabilities:

- Can calculate the following to arbitrary size/precision:
    - *e*
    - π
    - ϕ (golden ratio)
    - Square root
    - Fibonacci number
    - Convert between binary, decimal and hexadecimal strings

## Number types

Uses `bigInt` as integer type \
Uses `bigFrac` as fixed-point (scaled integer) type

## Algorithms

The different algorithms that this program uses are listed below

### Multiplication

- Naïve multiplication
- Karatsuba multiplication
- Toom-Cook (Toom-3) multiplication
- Schönhage–Strassen algorithm

### Division

- Knuth's Algorithm D
- Burnikel-Ziegler Division

### Square root

- Newton's method

## Requirements

The OS must be Linux and the architecture x86-64

## Makefile

`make setup` to initialize the repo after clone \
`make` to compile with all optimizations \
`make debug` to compile for debugging \
`make sanitize` to compile with sanitizers \
`make test` to compile for tests and run tests \
`make clean` to remove all temporary files

## Usage

See help message

## Tests:

Run all tests: `make test` \
Run all partial bigInt tests: `pytest -m partialBigInt_test`

## Misc

* Max length = ~2000 petabytes (64bit bit-length counter)
* Max ssaMax length = ~576 petabytes (log2(2*bitlength)<63)
* BigInt division uses truncating division
