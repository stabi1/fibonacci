import math

from gmpy2 import mpfr, get_context, const_pi

def reference_implementation_pi(n: int) -> str:
    if n < 0:
        raise ValueError("Number of digits must be non-negative")

    # MPFR precision is in bits: log2(10) ≈ 3.32193, add a safety margin
    bits = math.ceil((n + 5) * 3.32193)
    get_context().precision = bits

    pi: mpfr = const_pi()
    print("Pi calc done")

    pi_str = str(pi)
    res: str = pi_str[:n+2]

    if n == 0:
        res = res + "0"
    return res

res_o = reference_implementation_pi(1000000)
print(len(res_o))
