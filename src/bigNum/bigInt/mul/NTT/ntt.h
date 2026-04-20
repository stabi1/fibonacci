#ifndef FIBONACCI_NTT_H
#define FIBONACCI_NTT_H

#include "nttAsm.h"
#include "../../bigInt.h"

bigInt *ntt_mul(const bigInt *A, const bigInt *B);

#define prime 0xd800000000000001ULL

// assumes a and b are already < mod
static inline uint64_t add_mod(uint64_t a, uint64_t b) {
    uint64_t s = a + b;
    if (s < a || s >= prime) s -= prime;
    return s;
}

// assumes a and b are already < mod
static inline uint64_t sub_mod(uint64_t a, uint64_t b) {
    uint64_t s = a - b;
    if (a < b) s += prime;
    return s;
}

// 2^128/prime
static const unsigned __int128 INV_PRIME_128 = ((unsigned __int128) 0x1ULL << 64 | 0x2f684bda12f684bcULL);

// assumes a and b are already < mod
static inline uint64_t mul_mod(uint64_t a, uint64_t b) {
    // 1) full 128-bit product t = a * b
    unsigned __int128 t = (unsigned __int128) a * (unsigned __int128) b; // 0 <= t < 2^128

    // 2) compute b' = floor(b * 2^64 / MOD)
    //    b' = (b * inv128) >> 64
    uint64_t bprime = (uint64_t) (((unsigned __int128) b * INV_PRIME_128) >> 64);

    // 3) q = high64(a * b')
    uint64_t q = (uint64_t) (((unsigned __int128) a * (unsigned __int128) bprime) >> 64);

    // 4) r = t - q * MOD
    unsigned __int128 r = t - (unsigned __int128) q * (unsigned __int128) prime;

    // 5) correct: r, usually not executed more than once
    while (r >= (unsigned __int128) prime){
        r -= (unsigned __int128) prime;
    }

    return (uint64_t) r;
}

// `root_of_unity[k]` is a primitive 2^k-th root of unity
static uint64_t const root_of_unity[] = {
        0x0000000000000001,
        0xd800000000000000,
        0x338fcae5b5e93de7,
        0xadd13b12108a2af2,
        0x696a3ce7266add23,
        0x5809779d8e634611,
        0xba24aadfb9526c9e,
        0x01d1d661ee1baa4f,
        0xcd4aaa3c1f010bc7,
        0xcea6ac0c66646e66,
        0xbc7b659aac049d19,
        0xc27e3db8f569268b,
        0x493bc33d10bddcfe,
        0x6fedde2eee804bea,
        0x18bea6ace9b74936,
        0x61eaeb2d02e5dfbd,
        0x519331ffa30a7ddd,
        0x37a33058c8717fb7,
        0x07bf99f53889cb95,
        0x8c7ce5c8ebd57620,
        0xa4b63103ed05843b,
        0xb33c091422770a61,
        0x173e84b7153c9c5b,
        0xc1e7df8a18ab1560,
        0xbfe3ffde14a5c998,
        0x86c04a46d338d5ca,
        0x4d61602b57f8a2b7,
        0xcd752deb10dacb9e,
        0xcf41e213a88f3896,
        0x499a0795e3cde4cc,
        0x46f34947da5220ac,
        0x9e7a70b5f3449591,
        0x9130284f31693b78,
        0x11dd15a6cf83f252,
        0xa300bcea77a25e0d,
        0x74a200045f58128e,
        0x3e8b388898d96a18,
        0x844c386a2dd18862,
        0x2f20ee260ea52608,
        0x04c835b6d5c7852e,
        0x35bab97bb67fbdcb,
        0xbc86a2b6c113f7a7,
        0x97d5762fa79665d5,
        0x70477ee1d4a099bf,
        0x769bf42b1d9f6d7a,
        0x8290a9f9793abab3,
        0x3e5f1e9127360b6a,
        0x2c167a25402ba5d5,
        0xb02625f134b141a9,
        0x88167a3f31827a93,
        0x86aca5e67c632bac,
        0x4c9ba3cfef935ff7,
        0x5510185ef8a3d1a8,
        0xd0d563dbe61c4589,
        0xb612bd7372db4a63,
        0xbf12067fe6e73d59,
        0x01c6d97978fe1cdb,
        0x0d7417f041be58cc,
        0x94e541bb9f76dfad,
        0x6765c793fa10079d,
};

// `conj_of_unity[k]` is the inverse of `root_of_unity[k]`
static uint64_t const conj_of_unity[] = {
        0x0000000000000001,
        0xd800000000000000,
        0xa470351a4a16c21a,
        0xa0d248a4d1b495d7,
        0xb17105377ff1c5c4,
        0xd3233cb689f1280e,
        0x273329ba2b67d7e9,
        0x4436aadbe53e12f2,
        0xa1b4e0481d07ef9e,
        0xb1491b7a6910a0fd,
        0x98df1907d1d9bfb2,
        0x1a08f30514851d74,
        0x87d19fd7a85bd977,
        0x1ea9d5f771e901af,
        0x5c698885fe2f6ffd,
        0xd630b36410451b8e,
        0xaea9c3776313fb4d,
        0x9673b6bb07320066,
        0xd4b524232307690b,
        0x996cde702a884b41,
        0x9b91b21ac31772aa,
        0xb9215357ca070a34,
        0xb6bf0fa3a6fa20c4,
        0xac14f6d093e34d99,
        0x306d3327a301dc37,
        0xa6f4ae7966060d61,
        0x2192880aeb83672f,
        0x28a44fa8cc46acca,
        0x43200ba623f7dd18,
        0xb5b8bdd30121f416,
        0x149d50b3c5545322,
        0x6d34826b137c85c1,
        0x6b6d9f0f44fe9cce,
        0x19fdb50796db68be,
        0x5424889ab29841e7,
        0x407266e3fc55bb91,
        0x480d04045ba3c6a1,
        0x6fb68aacd41d4640,
        0x26905e6a14ca6bc9,
        0xa72755c78beb3a11,
        0x1cbdf241d85928ac,
        0xa7b68b81fd79a25f,
        0xa8c4a86ade6a4696,
        0x2462226a4c94ef55,
        0x3dbbc453730b3196,
        0x2fe5a4a1c212addf,
        0x28b9e0b942870e86,
        0xa1e4408ed0b177cd,
        0x02ffa5ba5c79c85a,
        0x6368cc279474fe20,
        0x8c56b26b797baca0,
        0x0301b634751fbb01,
        0x72cd5a174d2f73e0,
        0x0edc0c75ac780d8c,
        0x756aba8fa88a5254,
        0xc233bf9150999f26,
        0x2f7d3d721a808f51,
        0x507bed13bd6eb623,
        0xa6f67d59259349b8,
        0xae69595d70624893,
};

// `power_of_half[k]` is 2^(-k)
static uint64_t const power_of_half[] = {
        0x0000000000000001,
        0x6c00000000000001,
        0xa200000000000001,
        0xbd00000000000001,
        0xca80000000000001,
        0xd140000000000001,
        0xd4a0000000000001,
        0xd650000000000001,
        0xd728000000000001,
        0xd794000000000001,
        0xd7ca000000000001,
        0xd7e5000000000001,
        0xd7f2800000000001,
        0xd7f9400000000001,
        0xd7fca00000000001,
        0xd7fe500000000001,
        0xd7ff280000000001,
        0xd7ff940000000001,
        0xd7ffca0000000001,
        0xd7ffe50000000001,
        0xd7fff28000000001,
        0xd7fff94000000001,
        0xd7fffca000000001,
        0xd7fffe5000000001,
        0xd7ffff2800000001,
        0xd7ffff9400000001,
        0xd7ffffca00000001,
        0xd7ffffe500000001,
        0xd7fffff280000001,
        0xd7fffff940000001,
        0xd7fffffca0000001,
        0xd7fffffe50000001,
        0xd7ffffff28000001,
        0xd7ffffff94000001,
        0xd7ffffffca000001,
        0xd7ffffffe5000001,
        0xd7fffffff2800001,
        0xd7fffffff9400001,
        0xd7fffffffca00001,
        0xd7fffffffe500001,
        0xd7ffffffff280001,
        0xd7ffffffff940001,
        0xd7ffffffffca0001,
        0xd7ffffffffe50001,
        0xd7fffffffff28001,
        0xd7fffffffff94001,
        0xd7fffffffffca001,
        0xd7fffffffffe5001,
        0xd7ffffffffff2801,
        0xd7ffffffffff9401,
        0xd7ffffffffffca01,
        0xd7ffffffffffe501,
        0xd7fffffffffff281,
        0xd7fffffffffff941,
        0xd7fffffffffffca1,
        0xd7fffffffffffe51,
        0xd7ffffffffffff29,
        0xd7ffffffffffff95,
        0xd7ffffffffffffcb,
        0xd7ffffffffffffe6,
};

#endif //FIBONACCI_NTT_H
