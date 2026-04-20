import math
import random
import time


def random_hex(length: int) -> str:
    if length <= 0: return ""
    num_bits = length * 4
    return f"{random.getrandbits(num_bits):0{length}x}"


def add_modF(a: int, b: int, nprime: int) -> int:
    mod = (1 << nprime) + 1
    return (a + b) % mod


def sub_modF(a: int, b: int, nprime: int) -> int:
    mod = (1 << nprime) + 1
    return (a - b) % mod


def mul_2exp_modF(a: int, d: int, nprime: int) -> int:
    mod = (1 << nprime) + 1
    res = (a << d) % mod
    return res


def div_2exp_modF(a: int, k_bits: int, nprime: int) -> int:
    mod = (1 << nprime) + 1
    # Division by 2^k_bits modulo (2^nprime + 1) is equivalent to
    # multiplying by 2^(2*nprime - k_bits)
    inv_shift = 2 * nprime - k_bits
    return (a << inv_shift) % mod


def fft_initl(k: int) -> list[list[int]]:
    """Builds bit-reversal arrays"""
    l = [[0]]
    for i in range(1, k + 1):
        g = 1 << (i - 1)
        prev = l[i - 1]
        current = [0] * (1 << i)
        for j in range(g):
            current[j] = 2 * prev[j]
            current[g + j] = 1 + current[j]
        l.append(current)
    return l


def fft_fft(Ap: list[int], K: int, fft_l: list[list[int]], omega: int, nprime: int, inc: int = 1, start: int = 0):
    """Forward FFT"""
    if K == 2:
        a0, a1 = Ap[start], Ap[start + inc]
        Ap[start] = add_modF(a0, a1, nprime)
        Ap[start + inc] = sub_modF(a0, a1, nprime)
    else:
        K2 = K >> 1
        fft_fft(Ap, K2, fft_l, 2 * omega, nprime, inc * 2, start)
        fft_fft(Ap, K2, fft_l, 2 * omega, nprime, inc * 2, start + inc)

        k_idx = K.bit_length() - 1
        lk = fft_l[k_idx]

        for j in range(K2):
            idx0 = start + 2 * j * inc
            idx1 = start + (2 * j + 1) * inc
            a0, a1 = Ap[idx0], Ap[idx1]

            shift = lk[2 * j] * omega
            a1_twiddled = mul_2exp_modF(a1, shift, nprime)

            Ap[idx0] = add_modF(a0, a1_twiddled, nprime)
            Ap[idx1] = sub_modF(a0, a1_twiddled, nprime)


def fft_fftinv(Ap: list[int], K: int, omega: int, nprime: int, start: int = 0):
    """Inverse FFT"""
    mod = (1 << nprime) + 1
    if K == 2:
        a0, a1 = Ap[start], Ap[start + 1]
        Ap[start] = (a0 + a1) % mod
        Ap[start + 1] = (a0 - a1) % mod
    else:
        K2 = K >> 1
        fft_fftinv(Ap, K2, 2 * omega, nprime, start)
        fft_fftinv(Ap, K2, 2 * omega, nprime, start + K2)

        for j in range(K2):
            idx0 = start + j
            idx1 = start + j + K2
            a0, a1 = Ap[idx0], Ap[idx1]

            shift = j * omega
            a1_twiddled = mul_2exp_modF(a1, shift, nprime)

            Ap[idx0] = (a0 + a1_twiddled) % mod
            Ap[idx1] = (a0 - a1_twiddled) % mod


def mul_fft_decompose(N_val: int, K: int, nprime: int, M: int, Mp: int) -> list[int]:
    A = []
    mask = (1 << M) - 1
    for i in range(K):
        chunk = N_val & mask
        N_val >>= M
        val = mul_2exp_modF(chunk, i * Mp, nprime)
        A.append(val)
    return A


def mul_fft_internal(A_val: int, B_val: int, K: int, k_k: int, M: int, Mp: int, nprime: int, fft_l: list[list[int]]) -> int:
    Ap = mul_fft_decompose(A_val, K, nprime, M, Mp)
    Bp = mul_fft_decompose(B_val, K, nprime, M, Mp)

    fft_fft(Ap, K, fft_l, 2 * Mp, nprime)
    fft_fft(Bp, K, fft_l, 2 * Mp, nprime)

    mod = (1 << nprime) + 1
    for i in range(K):
        Ap[i] = (Ap[i] * Bp[i]) % mod

    fft_fftinv(Ap, K, 2 * Mp, nprime)

    # Post-inverse FFT division
    res = [0] * K
    res[0] = div_2exp_modF(Ap[0], k_k, nprime)
    for i in range(1, K):
        res[i] = div_2exp_modF(Ap[i], k_k + (K - i) * Mp, nprime)

    p = 0
    for i in range(K - 1, -1, -1):
        j = (K - i) & (K - 1)
        bj = res[j]
        if bj < 0:
            print("Taf")
        sh_bits = i * M
        p += (bj << sh_bits)

        # Subtract modulus if wrap-around negative occurred
        T_val = (i + 1) << (2 * M)
        if bj > T_val:
            print("Uff")
            p -= (mod << sh_bits)

    return p


def main():
    random.seed(10)
    # A = int(random_hex(100000 * 16), 16)
    # B = int(random_hex(70005 * 16), 16)
    A = int(
        "E6931FAC9DAB2B36C248B87D6AE33F9A62D7183A5D5789E4B2D6B441E2411DC709E111C7E1E7ACB6F8CAC0BB2FC4C8BC2AE3BAAAB9165CC458E199CB89F51B135F7091A5ABB0874DF3E8CB4543A5EB93B0441E9CA4C2B0FB3D30875CBF29ABD5B1ACF38984B35AE882809DD4CFE7ABC5C61BAA52E053B4C3643F204EF259D2E98042A948AAC5E884CB3EC7DB925643FD34FDD467E2CCA406035CB2744CB90A63E51C9737903343947E02086541E4C48A99630AA9AECE153843A4B190274EBC955F8592E30A2205A485846248987550AAF2094EC59E7931DC650C7451CC61C0CB2C46A1B3F2C349FAFF763C7F8D14DDFF946351744378D62C59285A8D7915614F5A2AC9E0D68ACA6248A9227AB8F1930EE38AC7A9D239C9B026A481E49D53161F9A9513FE5271C32E9C21D156EB9F1BEA57F6AE4F1B1DE3B7FD9CEE2D9CCA7B4C242D26C31D000B7F90B7FE48A131C7DEBFBE58165266DE56E1EDF26939AF07EC69AB1B17D8DB62143F2228B51551C3D2C7DE3F5072BD4D18C3AEB64CB9E8CBA838667B6ED2B2FCAB04ABAE8676E318B402A7D15B30D2D7DDB78650CC6AF82BC3D7AA805B02DD9AA523B7374A1323EE6B516D1B81E5F709C2C790EDAF1C3FA9B0A1DBC6DABC2B5ED267244C458752002B106D6381FAD58A7E193657BDE0FE029120F8379316891F828B8D24A049E5B86D855BCFED56765F9DA1AC54CAEAF9257",
        16)
    B = int(
        "7BC67B451BC70B0E52817DD1B704A6B418A83FD4A9CA4C89E1A6E779F8D9E9DF18747591E5B314C05763EDCD59632423CA83F14D6F073D784DB2B7001643A6760F9F0DD6DDD0A59E241DC1ED720287896286F5CC3ADDF6C1ADF6ED35F477B0022981E5E1FBFE1BFB8E26B5BA93253275BF6A44B3FA1051CDFE3B3F5D2725A9A580FD5B04525B3182FCD2B3FDA124ACA3C901406A2B55CD8B95D48D13E379F1CCBCDFC39FEE4ACC5523AA0BDEF57E63A1F81CBABA9F45CAAED48D06BFB3D16836042BED57CAC84761BFEB59A0C81304908BB781E4BBDF230D2E977374B97BD0B6B7D38B736428826A0F2729BE2290256DC304E875C9D4B3FB2125AE3D0CD3130D6114989517ACA97DAA2485181EB31C07D2C6A5BCC587E048A6D2BEACD6FE206F225C708461B41FDB5AD087C5DC4FCAEEC3A3437A42E51B065D6E4332F71B109D3317681AB0FCBF31C9F1C23BA46B4F983AF9214D13AC3DDF6C03F3E9854C4D47741A5576812BE0B5CB8BF647B930687EC881DF76191F9C468C1736EEF1E59635EB6CBD2C73B00068C8FFEDEDEC2826D114DC1F8824924FA079056E25A5DDBAECEB90A18C51F919A83AE980E25BF06DD486427DFDEE6F708AA642625CE4C4298AFDD7AC48AA81B0C5608C2D801543EDEEAB5479C1342E4E4AEC719F1A46E33C1D761EFDF22116051E18FA714C72FAE0756835527748E4DF2E728E3DAAF94E9B3",
        16)

    time_1 = time.perf_counter()

    S = A.bit_length() + B.bit_length()
    k = 10
    limb_bits = 64

    # Emulating GMP's sizing calculations
    pl = (S + limb_bits - 1) // limb_bits
    pl = ((pl + (1 << k) - 1) >> k) << k
    N = pl * limb_bits
    M = N >> k

    maxLK = math.lcm(limb_bits, 1 << k)
    Nprime = ((2 * M + k + 2) // maxLK + 1) * maxLK
    Mp = Nprime >> k
    K = 1 << k

    print(f"N: {N}, M: {M}, Nprime: {Nprime}, Mp: {Mp}, K: {K}")

    fft_l = fft_initl(k)
    R = mul_fft_internal(A, B, K, k, M, Mp, Nprime, fft_l)

    # Final normalization equivalent modulo 2^N + 1
    R = R % ((1 << N) + 1)

    time_2 = time.perf_counter()

    time_3 = time.perf_counter()
    trueRes = A * B
    time_4 = time.perf_counter()

    print(f"Time fft: {time_2 - time_1} sec, time native: {time_4 - time_3} sec")
    if trueRes == R:
        print("Yay! Match found.")
    else:
        print("Nao")


if __name__ == "__main__":
    main()
