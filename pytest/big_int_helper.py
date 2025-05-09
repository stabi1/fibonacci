import ctypes
import random
import shutil
from pathlib import Path
from typing import List, Tuple
from enum import Enum

import pytest
from fontTools.misc.cython import returns
from numpy.ma.core import nonzero

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


def prepare_test_folder(tmp_path: Path, filename_a: str, filename_b: str = None):
    # copy binary
    src = FIB_DIR.joinpath("fib")
    dest = tmp_path.joinpath("fib")
    shutil.copy(src, dest)

    # copy numer files
    if not tmp_path.joinpath(filename_a).exists():
        shutil.copy(NUMER_FILES_PATH.joinpath(filename_a), tmp_path.joinpath(filename_a))
    if filename_b is not None and not tmp_path.joinpath(filename_b).exists():
        shutil.copy(NUMER_FILES_PATH.joinpath(filename_b), tmp_path.joinpath(filename_b))


class OpsTestArithmetic(Enum):
    MUL = 1,
    DIV = 2,
    DIV_MOD = 3,
    ADD = 4,
    SUB = 5,
    SHIFT_LEFT = 6,
    SHIFT_RIGHT = 7


def arithmetic_test_helper(operation: OpsTestArithmetic, program_error: bool, tmp_path: Path, filename_a: str, output_file: str, command: str, filename_b: str = None,
                           output_file_2: str = None,
                           integer: int = None):
    with open(tmp_path.joinpath(filename_a), 'r') as f:
        hex_string_a = f.read()
    if filename_b is not None:
        with open(tmp_path.joinpath(filename_b), 'r') as f:
            hex_string_b = f.read()

    python_a = int(hex_string_a, 16)
    python_b = None
    if filename_b is not None:
        python_b = int(hex_string_b, 16)

    python_res = None
    python_res_2 = None
    try:
        if operation == OpsTestArithmetic.MUL:
            python_res = python_a * python_b
        elif operation == OpsTestArithmetic.DIV:
            abs_q = abs(python_a) // abs(python_b)  # truncating division
            sign = -1 if (python_a < 0) ^ (python_b < 0) else 1
            python_res = abs_q * sign
        elif operation == OpsTestArithmetic.DIV_MOD:
            abs_q = abs(python_a) // abs(python_b)  # truncating division
            sign = -1 if (python_a < 0) ^ (python_b < 0) else 1
            python_res = abs_q * sign
        elif operation == OpsTestArithmetic.ADD:
            python_res = python_a + python_b
        elif operation == OpsTestArithmetic.SUB:
            python_res = python_a - python_b
        elif operation == OpsTestArithmetic.SHIFT_LEFT:
            python_res = python_a << integer
        elif operation == OpsTestArithmetic.SHIFT_RIGHT:
            sign = -1 if python_a < 0 else 1  # non-arithmetic right shift
            a_abs = abs(python_a)
            python_res = (a_abs >> integer) * sign
        else:
            raise NotImplementedError("Invalid operation")

        if output_file_2 is not None:
            if operation == OpsTestArithmetic.DIV_MOD:
                python_res_2 = python_a - python_res * python_b  # mod of a truncating division
            else:
                raise NotImplementedError("Invalid arguments for arithmetic_test_helper")
    except NotImplementedError:
        raise
    except Exception as e:
        if program_error:
            return
        else:
            pytest.fail(f"Exception: {e} during Arithmetic, Program did not catch that!")

    if program_error:
        pytest.fail(f"Program threw error even though it was not supposed to")

    with open(tmp_path.joinpath(output_file), 'r') as f:
        res_string = f.read()
    tmp_path.joinpath(output_file).unlink()

    if output_file_2 is not None:
        with open(tmp_path.joinpath(output_file_2), 'r') as f:
            res_string_2 = f.read()
        tmp_path.joinpath(output_file_2).unlink()

    python_res_string = hex(python_res).replace("0x", "").upper()

    assert res_string == python_res_string.upper(), f"Command: {command}\n{res_string}\n{python_res_string}\n{hex(python_a).upper()}\n{hex(python_b).upper() if filename_b is not None else ""}"

    if output_file_2 is not None:
        python_res_string_2 = hex(python_res_2).replace("0x", "").upper()
        assert res_string_2 == python_res_string_2.upper(), f"Res2 false; Command: {command}"


tmp_hex = random_hex_string(500 * 16)
tmp_hex_near = tmp_hex[:-2] + "00"

positive_test_numbers_small_names: List[str] = [
    "zero.txt",
    "one.txt",
    "rand_len_15.txt",
    "rand_len_100.txt",
    "rand_len_60x16.txt"
]


def get_positive_test_numbers() -> List[Tuple[str, str]]:
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
        (random_hex_string(100), "rand_len_100.txt"),
        (random_hex_string(60 * 16), "rand_len_60x16.txt"),
        (random_hex_string(60 * 16 + 1), "rand_len_60x16+1.txt"),
        (tmp_hex, "tmp_hex.txt"),
        (tmp_hex_near, "tmp_hex_near.txt"),
        (random_hex_string(500 * 16 + 1), "rand_len_500x16+1.txt"),
        (random_hex_string(2000 * 16), "rand_len_2000x16.txt"),
        (random_hex_string(2000 * 16 + 1), "rand_len_2000x16+1.txt"),
        (random_hex_string(720 * 16 - 1), "rand_len_720x16-1.txt"),
        (random_hex_string(1152 * 16 - 6), "rand_len_1152*16-6.txt"),
        (random_hex_string(10000 * 16 - 3), "rand_len_10000*16-3.txt"),
        ("1" + "0" * 16 * 15 + "0" * 5, "rand_len_1+0xaLot.txt")
    ]
    return positive_test_numbers


def get_positive_dec_test_numbers() -> List[Tuple[str, str]]:
    positive_dec_test_numbers: List[Tuple[str, str]] = [
        ("0", "dec_zero.txt"),
        ("1", "dec_one.txt"),
        (random_dec_string(1), "dec_randon_len_1.txt"),
        (random_dec_string(2), "dec_randon_len_2.txt"),
        (random_dec_string(15), "dec_randon_len_15.txt"),
        (random_dec_string(16), "dec_randon_len_16.txt"),
        (random_dec_string(17), "dec_randon_len_17.txt"),
        (random_dec_string(31), "dec_randon_len_31.txt"),
        (random_dec_string(32), "dec_randon_len_32.txt"),
        (random_dec_string(33), "dec_randon_len_33.txt"),
        (random_dec_string(63), "dec_randon_len_63.txt"),
        (random_dec_string(64), "dec_randon_len_64.txt"),
        (random_dec_string(65), "dec_randon_len_65.txt"),
        (random_dec_string(100), "dec_randon_len_100.txt"),
        (random_dec_string(60 * 16), "dec_randon_len_60x16.txt"),
        (random_dec_string(60 * 16 + 1), "dec_randon_len_60x16+1.txt"),
        (random_dec_string(500 * 16 + 1), "dec_randon_len_500x16+1.txt"),
        (random_dec_string(2000 * 16), "dec_randon_len_2000x16.txt"),
        (random_dec_string(2000 * 16 + 1), "dec_randon_len_2000x16+1.txt"),
        (random_dec_string(720 * 16 - 1), "dec_randon_len_720x16-1.txt"),
        (random_dec_string(1152 * 16 - 6), "dec_randon_len_1152x16-6.txt"),
        (random_dec_string(10000 * 16 - 3), "dec_randon_len_10000x16-3.txt"),
        ("1" + "0" * 16 * 15 + "0" * 5, "dec_rand_len_1+0xaLot.txt")
    ]
    return positive_dec_test_numbers


def get_negative_test_numbers():
    negative_test_numbers = [("-" + hexstr, "-" + name) for hexstr, name in get_positive_test_numbers()]
    return negative_test_numbers


def get_negative_dec_test_numbers():
    negative_dec_test_numbers = [("-" + hexstr, "-" + name) for hexstr, name in get_positive_dec_test_numbers()]
    return negative_dec_test_numbers


def get_all_test_numbers():
    all_test_numbers = get_positive_test_numbers() + get_negative_test_numbers()
    return all_test_numbers


def get_all_dec_test_numbers():
    all_dec_test_numbers = get_positive_dec_test_numbers() + get_negative_dec_test_numbers()
    return all_dec_test_numbers


negative_test_numbers_small_names = ["-" + s for s in positive_test_numbers_small_names]

all_test_numbers_names = [s[1] for s in get_all_test_numbers()]
all_dec_test_numbers_names = [s[1] for s in get_all_dec_test_numbers()]
all_test_numbers_small_names = positive_test_numbers_small_names + negative_test_numbers_small_names

all_test_number_pairs_names = [(a, b) for a in all_test_numbers_names for b in all_test_numbers_names]
all_test_number_pairs_small_names = [(a, b) for a in all_test_numbers_small_names for b in all_test_numbers_small_names]


# positive_test_number_pairs_names = [(a, b) for a in positive_test_numbers for b in positive_test_numbers]
# positive_test_number_pairs_small_names = [(a, b) for a in positive_test_numbers_small_names for b in positive_test_numbers_small_names]


def add_partial_big_int_to_filename(filename: str, under: int = 20, over: int = 20) -> str:
    delimiter: str = ":"
    return f"{filename}{delimiter}{under}{delimiter}{over}"
