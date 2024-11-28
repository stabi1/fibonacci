import ctypes
from typing import Tuple

import pytest

from big_int_helper import big_int_lib, random_hex_string, random_dec_string, getPartialBigInt, trim_string, \
    get_all_test_numbers, get_partial_bigInts, BigInt


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


@pytest.mark.parametrize("test_hex_string", get_all_test_numbers())
def test_hexStringToBigInt_and_bigIntToHexString(test_hex_string: str):
    big_int_instance = big_int_lib.hexStringToBigInt(test_hex_string.encode())
    assert big_int_lib.isValidBigInt(big_int_instance)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance)
    big_int_lib.freeBigInt(big_int_instance)
    assert hex_str.decode() == test_hex_string


@pytest.mark.parametrize("test_dec_string", [
    "0",
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
])
def test_decStringToBigInt_and_bigIntToDecString(test_dec_string: str):
    big_int_instance = big_int_lib.decStringToBigInt(test_dec_string.encode())
    assert big_int_lib.isValidBigInt(big_int_instance)
    dec_str = big_int_lib.bigIntToDecString(big_int_instance)
    big_int_lib.freeBigInt(big_int_instance)
    assert dec_str.decode() == test_dec_string

    # negative numbers
    test_dec_string = "-" + test_dec_string
    big_int_instance_2 = big_int_lib.decStringToBigInt(test_dec_string.encode())
    assert big_int_lib.isValidBigInt(big_int_instance_2)
    dec_str = big_int_lib.bigIntToDecString(big_int_instance_2)
    big_int_lib.freeBigInt(big_int_instance_2)
    assert dec_str.decode() == test_dec_string


@pytest.mark.parametrize("tuple_a", get_partial_bigInts())
def test_partial_bigInt_hexStringToBigInt(tuple_a: Tuple[ctypes.POINTER(BigInt), str]):
    big_int_instance_a, hex_string_a = tuple_a
    hex_str = big_int_lib.bigIntToHexString(big_int_instance_a)
    assert hex_str.decode() == hex_string_a


@pytest.mark.parametrize("tuple_a", get_partial_bigInts())
def test_partial_bigInt_with_decStringToBigInt(tuple_a: Tuple[ctypes.POINTER(BigInt), str]):
    big_int_instance_a, hex_string_a = tuple_a
    dec_str = big_int_lib.bigIntToDecString(big_int_instance_a)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance_a)
    assert dec_str.decode() == str(int(hex_str.decode(), 16))
