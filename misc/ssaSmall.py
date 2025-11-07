import random

random.seed(10)

BASE_THRESHOLD_BITS = 10000


def random_hex(length: int) -> str:
    if length <= 0:
        return ""
    num_bits = length * 4
    random_number = random.getrandbits(num_bits)
    hex_str = f"{random_number:0{length}x}"  # Pad with zeros if needed
    return hex_str


def bit_length(x: int) -> int:
    if x == 0:
        return 0
    return x.bit_length()


def low_bits(x: int, n: int) -> int:
    return x & ((1 << n) - 1)


def slice_int(number: int, offset: int, chunk_length: int) -> int:
    tmp_num = number >> (offset * 64)
    return low_bits(tmp_num, chunk_length * 64)


def add_mod(u: int, v: int, N: int):
    mod = (1 << N) + 1
    s = u + v
    if s >= mod:
        s -= mod
    return s


def sub_mod(u: int, v: int, N: int):
    mod = (1 << N) + 1
    s = u - v
    if s < 0:
        s += mod
    return s


def mul_mod(u: int, v: int, N: int):
    mod = (1 << N) + 1
    if u == 0 or v == 0:
        return 0
    return (u * v) % mod


def compute_twiddle_exponent_for_cooley_tukey(j: int, n: int):
    global FULL_LENGTH
    if FULL_LENGTH is None:
        raise ValueError("FULL_P not set")
    return j * (FULL_LENGTH // n)


def decode_twiddle_index(t: int, m: int, N: int):
    P = FULL_LENGTH
    t = t % (2 * P)
    sign = 1
    if t >= P:
        t -= P
        sign = -1
    shift_bits = (t * m) % N
    return shift_bits, sign


def rotate_bits_left_N(x: int, s: int, N: int):
    s = s % N
    if N == 0:
        return 0
    maskN = (1 << N) - 1
    x &= maskN
    if s == 0:
        return x
    top = x >> (N - s)
    lowshift = (x << s) & maskN
    return lowshift | top


def twiddle_mul(x: int, t: int, mod: int, N: int):
    x_rep = x % mod
    shift_bits, sign = decode_twiddle_index(t, mod, N)
    y = rotate_bits_left_N(x_rep, shift_bits, N)
    if sign == -1:
        if y == 0:
            return 0
        return (mod - y) % mod
    else:
        return y % mod


def forward_fft_negacyclic_recursive(arr: list, lo: int, length: int, stride: int, mod: int, N: int):
    if length == 1:
        return
    half = length // 2
    forward_fft_negacyclic_recursive(arr, lo, half, stride * 2, mod, N)
    forward_fft_negacyclic_recursive(arr, lo + stride, half, stride * 2, mod, N)
    for j in range(half):
        even_index = lo + j * stride * 2
        odd_index = lo + (j * 2 + 1) * stride
        u = arr[even_index]
        v = arr[odd_index]
        t = compute_twiddle_exponent_for_cooley_tukey(j, length)
        v_twiddled = twiddle_mul(v, t, mod, N)
        arr[even_index] = add_mod(u, v_twiddled, N)
        arr[odd_index] = sub_mod(u, v_twiddled, N)


def inverse_fft_negacyclic_recursive(arr: list, lo: int, length: int, stride: int, mod: int, N: int):
    if length == 1:
        return
    half = length // 2
    inverse_fft_negacyclic_recursive(arr, lo, half, stride * 2, mod, N)
    inverse_fft_negacyclic_recursive(arr, lo + stride, half, stride * 2, mod, N)
    for j in range(half):
        even_index = lo + j * stride * 2
        odd_index = lo + (j * 2 + 1) * stride
        u = arr[even_index]
        v = arr[odd_index]
        t = compute_twiddle_exponent_for_cooley_tukey(j, length)
        v_twiddled = twiddle_mul(v, -t, mod, N)  # inverse twiddle
        arr[even_index] = add_mod(u, v_twiddled, N)
        arr[odd_index] = sub_mod(u, v_twiddled, N)


def divide_by_2_mod(x: int, N: int):
    mod = (1 << N) + 1
    x = x % mod
    if (x & 1) == 0:
        return x >> 1
    else:
        return (x + mod) >> 1


def scale_array_by_inv_p_by_shifts(arr: list, k: int, N: int):
    for _iter in range(k):
        for i in range(len(arr)):
            arr[i] = divide_by_2_mod(arr[i], N)
    return arr


def limbs_to_integer(limbs: list[int], m: int):
    R = 0
    for i, limb in enumerate(limbs):
        R += (limb << (i * m))
    return R


def reconstruct_from_Z(Z: list[int], M: int, Nprime: int) -> int:
    K = len(Z)
    MOD = (1 << Nprime) + 1

    Zr = [z % MOD for z in Z]

    p = 0
    for i in range(K - 1, -1, -1):
        j = (K - i) & (K - 1)
        bj = Zr[j]
        sh_bits = i * M
        p += (bj << sh_bits)
        T = (i + 1) << (2 * M)
        if bj > T:
            p -= (MOD << sh_bits)
    return p


char1 = random_hex(1000 * 16)
char2 = random_hex(705 * 16)
A = int(char1, 16)
B = int(char2, 16)

bitLenA = bit_length(A)
bitLenB = bit_length(B)
S = bitLenA + bitLenB

k = 10
tmp = (1 << k) * 64
N = (S // tmp + 1) * tmp
M = N // (1 << k)

N_prime_unaligned = 2 * M + k + 3
align = 1 << k if (1 << k) > 64 else 64
N_prime = ((N_prime_unaligned + align - 1) // align) * align

g = (2 * N_prime) // (1 << k)

print(f"S: {S}; k: {k}; N: {N}; M: {M}; N'unaligned: {N_prime_unaligned}; N': {N_prime}; g: {g}")
numChunks = 1 << k
chunkLength = M // 64
print(f"numChunks: {numChunks}; chunkSize: {chunkLength}")

modulus = (1 << N_prime) + 1
FULL_LENGTH = numChunks

a: list[int] = []
b: list[int] = []
for c in range(0, numChunks):
    a.append(slice_int(A, c * chunkLength, chunkLength))
for c in range(0, numChunks):
    b.append(slice_int(B, c * chunkLength, chunkLength))

forward_fft_negacyclic_recursive(a, 0, numChunks, 1, modulus, N_prime)
forward_fft_negacyclic_recursive(b, 0, numChunks, 1, modulus, N_prime)

# pointwise multiply (mod M)
Z = [0] * numChunks
for c in range(numChunks):
    Z[c] = mul_mod(a[c], b[c], N_prime)

# inverse FFT (recursive)
inverse_fft_negacyclic_recursive(Z, 0, numChunks, 1, modulus, N_prime)

# scale by 1/P in ring Z/(2^N+1)
scale_array_by_inv_p_by_shifts(Z, k, N_prime)

R = reconstruct_from_Z(Z, M, N_prime)

print(f"R: {bit_length(R%N)}")

trueRes = A * B
if trueRes == R:
    print(f"Yay")
else:
    print(f"Nao")
    print(f"{bit_length(trueRes)} {bit_length(R)} {(bit_length(trueRes) - bit_length(R))//64}")


