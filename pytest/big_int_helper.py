import ctypes
import random
from pathlib import Path
from typing import List, Tuple

random.seed(42)

command_list = ["", "--do-swap --swap-threshold 0", "--deactivate-caches", "-m --max-threads 8", "-m --max-threads 10"]
command_list_non_multithread = ["", "--do-swap --swap-threshold 0", "--deactivate-caches"]


FIB_DIR = Path(__file__).parent

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


def get_partial_bigint(big_int_instance: ctypes.POINTER(BigInt), new_start: int, new_end: int) -> ctypes.POINTER(BigInt):
    big_int = big_int_instance.contents
    old_start = big_int.start
    old_end = big_int.end
    if old_end < new_end or old_start > new_start or new_start >= new_end:
        raise ValueError(
            f"Invalid new_start or new_end, new_start: {new_start}, new_end: {new_end}; old_start: {old_start}, old_end: {old_end}")
    big_int.end = new_end
    big_int.start = new_start
    return ctypes.pointer(big_int)


def trim_string(s, remove_front, remove_back):
    return s[remove_front:] if remove_back == 0 else s[remove_front:-remove_back]


def remove_leading_zeros_str(s: str):
    return s.lstrip('0') or '0'


big_int_lib = ctypes.CDLL('./pytest/bigInt.so')

# bigInt *newBigInt(size_t len);
big_int_lib.newBigInt.argtypes = [ctypes.c_size_t]
big_int_lib.newBigInt.restype = ctypes.POINTER(BigInt)

# void freeBigInt(bigInt *toDelete);
big_int_lib.newBigInt.freeBigInt = [ctypes.POINTER(BigInt)]
big_int_lib.newBigInt.freeBigInt = None

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

# bigInt *shiftAdd(bigInt *x, bigInt *toShift, size_t n)
big_int_lib.shiftAdd.argtypes = [ctypes.POINTER(BigInt), ctypes.POINTER(BigInt), ctypes.c_size_t]
big_int_lib.shiftAdd.restype = ctypes.POINTER(BigInt)

tmp_hex = random_hex_string(500 * 16)
tmp_hex_near = tmp_hex[:-2] + "00"


def bigint_to_python_hex_string(big_int_instance: ctypes.POINTER(BigInt)) -> str:
    hex_str = big_int_lib.bigIntToHexString(big_int_instance)
    return hex_str.decode()


zero = ["0"]
positive_test_numbers_no_zero: List[str] = [
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
    random_hex_string(720 * 16 - 1),
    random_hex_string(1152 * 16 - 6),
    random_hex_string(10000 * 16 - 3),
    "1" + "0" * 16 * 15 + "0" * 5
]

positive_test_numbers = positive_test_numbers_no_zero + zero

negative_test_numbers = ["-" + s for s in positive_test_numbers_no_zero]

all_test_numbers = positive_test_numbers + negative_test_numbers

all_test_number_pairs = [(a, b) for a in all_test_numbers for b in all_test_numbers]

positive_test_number_pairs = [(a, b) for a in positive_test_numbers for b in positive_test_numbers]


def create_partial_bigints() -> List[Tuple[ctypes.POINTER(BigInt), str]]:
    big_int_length = 500
    test_hex_string = random_hex_string(16 * big_int_length)
    bounds = [(0, 1), (1, 2), (1, 20), (50, 61), (200, 400)]
    res_list = []
    for start, end in bounds:
        big_int_instance = big_int_lib.hexStringToBigInt(test_hex_string.encode())
        big_int_partial = get_partial_bigint(big_int_instance, start, end)
        partial_hex_str = remove_leading_zeros_str(trim_string(test_hex_string, (big_int_length - end) * 16, start * 16))  # switched because stings are big endian
        res_list.append((big_int_partial, partial_hex_str))
        if partial_hex_str == "":
            print(f"Got you {start} {end}")
    return res_list


partial_bigInts = create_partial_bigints()

partial_bigInts_pairs = [(a, b) for a in partial_bigInts for b in partial_bigInts]
