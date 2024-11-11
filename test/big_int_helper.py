import ctypes
import random
import signal
import sys

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


signal.signal(signal.SIGTERM, handler)

big_int_lib = ctypes.CDLL('./test/bigInt.so')

# bigInt *newBigInt(size_t len);
big_int_lib.newBigInt.argtypes = [ctypes.c_size_t]
big_int_lib.newBigInt.restype = ctypes.POINTER(BigInt)

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

