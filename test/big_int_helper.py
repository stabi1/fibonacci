import ctypes
import random
from typing import List

random.seed(42)


class BigInt(ctypes.Structure):
    _fields_ = [
        ("start", ctypes.c_size_t),  # size_t start
        ("end", ctypes.c_size_t),  # size_t end
        ("arrayOwner", ctypes.c_bool),  # bool arrayOwner
        ("bigIntArray", ctypes.POINTER(ctypes.c_uint64)),  # uint64_t* bigIntArray
        ("negative", ctypes.c_bool)  # bool negative
    ]


def random_hex_string(length: int) -> str:
    """Generate a random hex string of a specified length"""
    # First character should not be '0'
    hex_string = random.choice('123456789ABCDEF')
    hex_string += ''.join(random.choice('0123456789ABCDEF') for _ in range(length - 1))
    return hex_string


def random_dec_string(length: int) -> str:
    """Generate a random dec string of a specified length"""
    # First character should not be '0'
    hex_string = random.choice('123456789')
    hex_string += ''.join(random.choice('0123456789') for _ in range(length - 1))
    return hex_string


def getPartialBigInt(big_int_instance: ctypes.POINTER(BigInt), new_start: int, new_end: int) -> ctypes.POINTER(BigInt):
    big_int = big_int_instance.contents
    old_end = big_int.end
    if old_end < new_end or new_start >= new_end:
        raise ValueError("Invalid new_start or new_end")
    big_int.end = new_end
    big_int.start = new_start
    return ctypes.pointer(big_int)


def trim_string(s, remove_front, remove_back):
    return s[remove_front:-remove_back]


def handler(signum, frame):
    print("Caught signal, preventing exit")


big_int_lib = ctypes.CDLL('./test/bigInt.so')

# bigInt *newBigInt(size_t len);
big_int_lib.newBigInt.argtypes = [ctypes.c_size_t]
big_int_lib.newBigInt.restype = ctypes.POINTER(BigInt)

# bool isValidBigInt(bigInt *x);
big_int_lib.isValidBigInt.argtypes = [ctypes.POINTER(BigInt)]
big_int_lib.isValidBigInt.restype = ctypes.c_bool

# size_t getOccupiedBlocks(bigInt *x);
big_int_lib.getOccupiedBlocks.argtypes = [ctypes.POINTER(BigInt)]
big_int_lib.getOccupiedBlocks.restype = ctypes.c_size_t

# char *bigIntToHexString(bigInt *x);
big_int_lib.bigIntToHexString.argtypes = [ctypes.POINTER(BigInt)]
big_int_lib.bigIntToHexString.restype = ctypes.c_char_p

# char *bigIntToDecString(bigInt *x);
big_int_lib.bigIntToDecString.argtypes = [ctypes.POINTER(BigInt)]
big_int_lib.bigIntToDecString.restype = ctypes.c_char_p

# bigInt *hexStringToBigInt(char *hex);
big_int_lib.hexStringToBigInt.argtypes = [ctypes.c_char_p]
big_int_lib.hexStringToBigInt.restype = ctypes.POINTER(BigInt)

# bigInt *decStringToBigInt(char *dec);
big_int_lib.decStringToBigInt.argtypes = [ctypes.c_char_p]
big_int_lib.decStringToBigInt.restype = ctypes.POINTER(BigInt)

# bigInt *add(bigInt *x, bigInt *y);
big_int_lib.add.argtypes = [ctypes.POINTER(BigInt), ctypes.POINTER(BigInt)]
big_int_lib.add.restype = ctypes.POINTER(BigInt)

# bigInt *sub(bigInt *x, bigInt *y);
big_int_lib.sub.argtypes = [ctypes.POINTER(BigInt), ctypes.POINTER(BigInt)]
big_int_lib.sub.restype = ctypes.POINTER(BigInt)

# bigInt *mul(bigInt *x, bigInt *y);
big_int_lib.mul.argtypes = [ctypes.POINTER(BigInt), ctypes.POINTER(BigInt)]
big_int_lib.mul.restype = ctypes.POINTER(BigInt)


tmp_hex = random_hex_string(500 * 16)
tmp_hex_near = tmp_hex[:-2] + "00"

positive_test_numbers: List[str] = [
    "0",
    "1",
    random_hex_string(1),
    random_hex_string(2),
    random_hex_string(15),
    random_hex_string(16),
    random_hex_string(17),
    random_hex_string(31),
    random_hex_string(32),
    random_hex_string(33),
    random_hex_string(63),
    random_hex_string(64),
    random_hex_string(65),
    random_hex_string(100),
    random_hex_string(60 * 16),
    random_hex_string(60 * 16 + 1),
    tmp_hex,
    tmp_hex_near,
    random_hex_string(500 * 16 + 1),
    random_hex_string(2000 * 16),
    random_hex_string(2000 * 16 + 1),
    random_hex_string(720*16 - 1),
    random_hex_string(1152*16 - 6),
    random_hex_string(10000 * 16 - 3),
]


def get_negative_test_numbers() -> List[str]:
    return ["-" + s for s in positive_test_numbers]


def get_all_test_numbers() -> List[str]:
    return positive_test_numbers + positive_test_numbers


def get_all_test_number_pairs():
    return [(a, b) for a in get_all_test_numbers() for b in get_all_test_numbers()]
