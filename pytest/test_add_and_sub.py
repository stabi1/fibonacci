import ctypes
from typing import Tuple

import pytest
from big_int_helper import big_int_lib, get_all_test_number_pairs, get_partial_bigInts_pairs, BigInt


@pytest.mark.parametrize("hex_string_a, hex_string_b", get_all_test_number_pairs())
def test_add_sign_and_different_size_numbers(hex_string_a: str, hex_string_b: str):
    big_int_instance_a = big_int_lib.hexStringToBigInt(hex_string_a.encode())
    big_int_instance_b = big_int_lib.hexStringToBigInt(hex_string_b.encode())
    big_int_instance_res = big_int_lib.add(big_int_instance_a, big_int_instance_b)
    assert big_int_lib.isValidBigInt(big_int_instance_res)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance_res)

    python_a = int(hex_string_a, 16)
    python_b = int(hex_string_b, 16)
    python_res = python_a + python_b
    python_hex_string = hex(python_res)

    assert python_hex_string.replace("0x", "").upper() == hex_str.decode()


@pytest.mark.parametrize("tuple_a, tuple_b", get_partial_bigInts_pairs())
def test_add_partial_bigInts(tuple_a: Tuple[ctypes.POINTER(BigInt), str],
                             tuple_b: Tuple[ctypes.POINTER(BigInt), str]):
    big_int_instance_a, hex_string_a = tuple_a
    big_int_instance_b, hex_string_b = tuple_b

    big_int_instance_res = big_int_lib.add(big_int_instance_a, big_int_instance_b)
    assert big_int_lib.isValidBigInt(big_int_instance_res)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance_res)

    python_a = int(hex_string_a, 16)
    python_b = int(hex_string_b, 16)
    python_res = python_a + python_b
    python_hex_string = hex(python_res)

    assert python_hex_string.replace("0x", "").upper() == hex_str.decode()


@pytest.mark.parametrize("hex_string_a, hex_string_b", get_all_test_number_pairs())
def test_sub_sign_and_different_size_numbers(hex_string_a: str, hex_string_b: str):
    big_int_instance_a = big_int_lib.hexStringToBigInt(hex_string_a.encode())
    big_int_instance_b = big_int_lib.hexStringToBigInt(hex_string_b.encode())
    big_int_instance_res = big_int_lib.sub(big_int_instance_a, big_int_instance_b)
    assert big_int_lib.isValidBigInt(big_int_instance_res)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance_res)

    python_a = int(hex_string_a, 16)
    python_b = int(hex_string_b, 16)
    python_res = python_a - python_b
    python_hex_string = hex(python_res)
    assert python_hex_string.replace("0x", "").upper() == hex_str.decode()


@pytest.mark.parametrize("tuple_a, tuple_b", get_partial_bigInts_pairs())
def test_sub_partial_bigInts(tuple_a: Tuple[ctypes.POINTER(BigInt), str],
                             tuple_b: Tuple[ctypes.POINTER(BigInt), str]):
    big_int_instance_a, hex_string_a = tuple_a
    big_int_instance_b, hex_string_b = tuple_b

    big_int_instance_res = big_int_lib.sub(big_int_instance_a, big_int_instance_b)
    assert big_int_lib.isValidBigInt(big_int_instance_res)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance_res)

    python_a = int(hex_string_a, 16)
    python_b = int(hex_string_b, 16)
    python_res = python_a - python_b
    python_hex_string = hex(python_res)

    assert python_hex_string.replace("0x", "").upper() == hex_str.decode()
