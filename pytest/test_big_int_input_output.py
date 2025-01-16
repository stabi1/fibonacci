import ctypes
import shutil
from pathlib import Path
from typing import Tuple

import pytest

from big_int_helper import big_int_lib, random_dec_string, all_test_numbers, partial_bigInts, BigInt, FIB_DIR
from run_big_int import run_command_in_valgrind

DEC_ZERO = zero = ["0"]
DEC_LIST_POSITIVE_NO_ZERO = [
    "1",
    random_dec_string(1),
    random_dec_string(2),
    random_dec_string(15),
    random_dec_string(16),
    random_dec_string(17),
    random_dec_string(31),
    random_dec_string(32),
    random_dec_string(33),
    random_dec_string(63),
    random_dec_string(64),
    random_dec_string(65),
    random_dec_string(100),
    random_dec_string(60 * 16),
    random_dec_string(60 * 16 + 1),
    random_dec_string(500 * 16 + 1),
    random_dec_string(2000 * 16),
    random_dec_string(2000 * 16 + 1),
    random_dec_string(720 * 16 - 1),
    random_dec_string(1152 * 16 - 6),
    random_dec_string(10000 * 16 - 3),
    "1" + "0" * 16 * 15 + "0" * 5
]

DEC_LIST_POSITIVE = DEC_LIST_POSITIVE_NO_ZERO + DEC_ZERO

DEC_LIST_NEGATIVE = ["-" + s for s in DEC_LIST_POSITIVE_NO_ZERO]

DEC_LIST_ALL_NUMBERS = DEC_LIST_POSITIVE + DEC_LIST_NEGATIVE

def test_bigIntToHexString_for_zero():
    big_int_instance = big_int_lib.newBigInt(1)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance)
    big_int_lib.freeBigInt(big_int_instance)
    assert hex_str.decode() == "0"


def test_bigIntToDecString_for_zero():
    big_int_instance = big_int_lib.newBigInt(1)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance)
    big_int_lib.freeBigInt(big_int_instance)
    assert hex_str.decode() == "0"


@pytest.mark.parametrize("test_hex_string", all_test_numbers)
def test_hexStringToBigInt_and_bigIntToHexString(test_hex_string: str):
    big_int_instance = big_int_lib.hexStringToBigInt(test_hex_string.encode())
    assert big_int_lib.isValidBigInt(big_int_instance)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance)
    big_int_lib.freeBigInt(big_int_instance)
    assert hex_str.decode() == test_hex_string


@pytest.mark.parametrize("test_dec_string", DEC_LIST_ALL_NUMBERS)
def test_decStringToBigInt_and_bigIntToDecString(test_dec_string: str):
    big_int_instance = big_int_lib.decStringToBigInt(test_dec_string.encode())
    assert big_int_lib.isValidBigInt(big_int_instance)
    dec_str = big_int_lib.bigIntToDecString(big_int_instance)
    big_int_lib.freeBigInt(big_int_instance)
    assert dec_str.decode() == test_dec_string


@pytest.mark.valgrind_test
@pytest.mark.parametrize("test_dec_string", DEC_LIST_ALL_NUMBERS)
def test_decStringToBigInt_and_bigIntToDecString_valgrind(tmp_path: Path, test_dec_string: str):
    output_file = "out.txt"
    input_file = "input.txt"

    src = FIB_DIR.joinpath("fib")
    dest = tmp_path.joinpath("fib")
    shutil.copy(src, dest)
    with open(tmp_path.joinpath(input_file), 'w') as f:
        f.write(test_dec_string)
    command: str = f"./fib --convert-number --input-filename {input_file} --output-filename {output_file} --input-radix d --output-radix d --output-format f"
    run_command_in_valgrind(command, tmp_path)

    with open(tmp_path.joinpath(output_file), 'r') as f:
        res_string = f.read()

    assert res_string == test_dec_string, f"Command: {command}"


@pytest.mark.parametrize("tuple_a", partial_bigInts)
def test_partial_bigInt_hexStringToBigInt(tuple_a: Tuple[ctypes.POINTER(BigInt), str]):
    big_int_instance_a, hex_string_a = tuple_a
    hex_str = big_int_lib.bigIntToHexString(big_int_instance_a)
    assert hex_str.decode() == hex_string_a


@pytest.mark.parametrize("tuple_a", partial_bigInts)
def test_partial_bigInt_with_decStringToBigInt(tuple_a: Tuple[ctypes.POINTER(BigInt), str]):
    big_int_instance_a, hex_string_a = tuple_a
    dec_str = big_int_lib.bigIntToDecString(big_int_instance_a)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance_a)
    assert dec_str.decode() == str(int(hex_str.decode(), 16))
