import ctypes
import random
from pathlib import Path
from typing import List, Tuple

random.seed(42)

command_list = ["", "--do-swap --swap-threshold 0", "--deactivate-caches", "-m --max-threads 8", "-m --max-threads 10"]
command_list_non_multithread = ["", "--do-swap --swap-threshold 0", "--deactivate-caches"]

FIB_DIR = Path(__file__).parent
NUMER_FILES_PATH = FIB_DIR.joinpath("numer_files")


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


def trim_string(s, remove_front, remove_back):
    return s[remove_front:] if remove_back == 0 else s[remove_front:-remove_back]


def remove_leading_zeros_str(s: str):
    return s.lstrip('0') or '0'


tmp_hex = random_hex_string(500 * 16)
tmp_hex_near = tmp_hex[:-2] + "00"

positive_test_numbers_small_names: List[str] = [
    "zero.txt",
    "one.txt",
    "rand_len_15.txt",
    "rand_len_100.txt",
    "rand_len_60x16.txt"
]

positive_test_numbers: List[Tuple[str, str]] = [
    ("0", "zero.txt"),
    ("1", "one.txt"),
    (random_hex_string(1), "rand_len_1.txt"),
    (random_hex_string(2), "rand_len_2.txt"),
    (random_hex_string(15), "rand_len_15.txt"),
    (random_hex_string(16), "rand_len_16.txt"),
    (random_hex_string(17), "rand_len_17.txt"),
    (random_hex_string(31), "rand_len_31.txt"),
    (random_hex_string(32), "rand_len_32.txt"),
    (random_hex_string(33), "rand_len_33.txt"),
    (random_hex_string(63), "rand_len_63.txt"),
    (random_hex_string(64), "rand_len_64.txt"),
    (random_hex_string(65), "rand_len_65.txt"),
    (random_hex_string(100), "rand_len_1.txt"),
    (random_hex_string(60 * 16), "rand_len00_60x16.txt"),
    (random_hex_string(60 * 16 + 1), "rand_len_60x16+1.txt"),
    (tmp_hex, "tmp_hex.txt"),
    (tmp_hex_near, "tmp_hex_near.txt"),
    (random_hex_string(500 * 16 + 1), "rand_len_500x16+1.txt"),
    (random_hex_string(2000 * 16), "rand_len_2000x16.txt"),
    (random_hex_string(2000 * 16 + 1), "rand_len_2000x16+1.txt"),
    (random_hex_string(720 * 16 - 1), "rand_len_720x16-1.txt"),
    (random_hex_string(1152 * 16 - 6), "rand_len_1152*16-6.txt"),
    (random_hex_string(10000 * 16 - 3), "rand_len_10000*16-3.txt"),
    ("1" + "0" * 16 * 15 + "0" * 5, "rand_len_1+0xaLot.txt"),
]

negative_test_numbers = [("-" + hexstr, "-" + name) for hexstr, name in positive_test_numbers]
negative_test_numbers_small_names = ["-" + s for s in positive_test_numbers_small_names]

all_test_numbers = positive_test_numbers + negative_test_numbers
all_test_numbers_names = [s[1] for s in all_test_numbers]
all_test_numbers_small_names = positive_test_numbers_small_names + negative_test_numbers_small_names

all_test_number_pairs_names = [(a, b) for a in all_test_numbers_names for b in all_test_numbers_names]
all_test_number_pairs_small_names = [(a, b) for a in all_test_numbers_small_names for b in all_test_numbers_small_names]


# positive_test_number_pairs_names = [(a, b) for a in positive_test_numbers for b in positive_test_numbers]
# positive_test_number_pairs_small_names = [(a, b) for a in positive_test_numbers_small_names for b in positive_test_numbers_small_names]


def add_partial_big_int_to_filename(filename: str, under: int = 20, over: int = 20) -> str:
    delimiter: str = ":"
    return f"{filename}{delimiter}{under}{delimiter}{over}"
