# used for informal benchmarking
# from https://www.craig-wood.com/nick/articles/pi-chudnovsky/

import math
from gmpy2 import mpz
import time
import gmpy2

def pi_chudnovsky_bs(digits):
    """
    Compute int(pi * 10**digits)

    This is done using Chudnovsky's series with binary splitting
    """
    C = 640320
    C3_OVER_24 = C**3 // 24
    def bs(a, b):
        """
        Computes the terms for binary splitting the Chudnovsky infinite series

        a(a) = +/- (13591409 + 545140134*a)
        p(a) = (6*a-5)*(2*a-1)*(6*a-1)
        b(a) = 1
        q(a) = a*a*a*C3_OVER_24

        returns P(a,b), Q(a,b) and T(a,b)
        """
        if b - a == 1:
            # Directly compute P(a,a+1), Q(a,a+1) and T(a,a+1)
            if a == 0:
                Pab = Qab = mpz(1)
            else:
                Pab = mpz((6*a-5)*(2*a-1)*(6*a-1))
                Qab = mpz(a*a*a*C3_OVER_24)
            Tab = Pab * (13591409 + 545140134*a) # a(a) * p(a)
            if a & 1:
                Tab = -Tab
        else:
            # Recursively compute P(a,b), Q(a,b) and T(a,b)
            # m is the midpoint of a and b
            m = (a + b) // 2
            # Recursively calculate P(a,m), Q(a,m) and T(a,m)
            Pam, Qam, Tam = bs(a, m)
            # Recursively calculate P(m,b), Q(m,b) and T(m,b)
            Pmb, Qmb, Tmb = bs(m, b)
            # Now combine
            Pab = Pam * Pmb
            Qab = Qam * Qmb
            Tab = Qmb * Tam + Pam * Tmb
        return Pab, Qab, Tab
    # how many terms to compute
    DIGITS_PER_TERM = math.log10(C3_OVER_24/6/2/6)
    N = int(digits/DIGITS_PER_TERM + 1)

    # Calclate P(0,N) and Q(0,N)
    start_time1 = time.perf_counter()
    P, Q, T = bs(0, N)
    now_time1 = time.perf_counter()
    print(f"Time for binary splitting: {round(now_time1 - start_time1, 5)}")

    start_time1 = time.perf_counter()
    one_squared = mpz(10)**(2*digits)
    sqrtC = gmpy2.isqrt(10005 * one_squared)
    now_time1 = time.perf_counter()
    print(f"Time for root: {round(now_time1 - start_time1, 5)}")

    start_time1 = time.perf_counter()
    res =  (Q*426880*sqrtC) // T
    now_time1 = time.perf_counter()
    print(f"Time for rest: {round(now_time1 - start_time1, 5)}")

    return res

start_time = time.perf_counter()
resOut = pi_chudnovsky_bs(1000000)
now_time = time.perf_counter()
print(f"Time to calc: {round(now_time - start_time, 5)}")

start_time = time.perf_counter()
res_string = str(resOut)
now_time = time.perf_counter()
print(f"Time to convert: {round(now_time - start_time, 5)} for {len(res_string)}")


