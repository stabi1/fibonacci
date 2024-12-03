import ctypes
from typing import Tuple
import pytest

from big_int_helper import big_int_lib, getPartialBigInt, partial_bigInts_pairs, BigInt, bigInt_to_python_hex_string, positive_test_number_pairs


def test_new_big_int_is_zero():
    big_int_instance = big_int_lib.newBigInt(1)
    big_int = big_int_instance.contents
    assert big_int.bigIntArray[0] == 0


@pytest.mark.parametrize("hex_str, length", [
    ("0", 1),
    ("1", 1),
    (16 * "1", 1),
    (16 * "1" + "1", 2),
    (32 * 16 * "1", 32),
])
def test_getOccupiedBlocks_normalBigInt(hex_str: str, length: int):
    big_int_instance = big_int_lib.hexStringToBigInt(hex_str.encode())
    blocks = big_int_lib.getOccupiedBlocks(big_int_instance)
    big_int_lib.freeBigInt(big_int_instance)
    assert blocks == length


def test_getOccupiedBlocks_partialBigInt():
    big_int_instance = big_int_lib.hexStringToBigInt((32 * 16 * "1").encode())
    big_int_instance = getPartialBigInt(big_int_instance, 3, 6)
    blocks = big_int_lib.getOccupiedBlocks(big_int_instance)
    big_int_lib.freeBigInt(big_int_instance)
    assert blocks == 3


def test_getOccupiedBlocks_onlyZeroBlocks():
    big_int_instance = big_int_lib.newBigInt(10)
    blocks = big_int_lib.getOccupiedBlocks(big_int_instance)
    big_int_lib.freeBigInt(big_int_instance)
    assert blocks == 1


@pytest.mark.parametrize("hex_string_a, hex_string_b", positive_test_number_pairs)
def test_shiftAdd_positive_numbers(hex_string_a: str, hex_string_b: str):
    len_a = len(hex_string_a) // 16 + 1
    len_b = len(hex_string_b) // 16 + 1
    numbers = [0, len_a // 2, len_b // 2, len_a, len_b, len_a * 2, len_b * 2]
    big_int_instance_a = big_int_lib.hexStringToBigInt(hex_string_a.encode())
    big_int_instance_b = big_int_lib.hexStringToBigInt(hex_string_b.encode())
    python_a = int(hex_string_a, 16)
    python_b = int(hex_string_b, 16)
    for n in numbers:
        big_int_instance_res = big_int_lib.shiftAdd(big_int_instance_a, big_int_instance_b, n)
        assert big_int_lib.isValidBigInt(big_int_instance_res)
        hex_str = big_int_lib.bigIntToHexString(big_int_instance_res)
        big_int_lib.freeBigInt(big_int_instance_res)

        python_res = python_a + (python_b << n * 64)
        python_hex_string = hex(python_res).replace("0x", "").upper()
        assert python_hex_string == hex_str.decode(), f"To shift was {n}"

    big_int_lib.freeBigInt(big_int_instance_a)
    big_int_lib.freeBigInt(big_int_instance_b)


@pytest.mark.parametrize("tuple_a, tuple_b", partial_bigInts_pairs)
def test_shiftAdd_partial_bigInts(tuple_a: Tuple[ctypes.POINTER(BigInt), str],
                                  tuple_b: Tuple[ctypes.POINTER(BigInt), str]):
    big_int_instance_a, hex_string_a = tuple_a
    big_int_instance_b, hex_string_b = tuple_b
    len_a = len(hex_string_a) // 16 + 1
    len_b = len(hex_string_b) // 16 + 1
    numbers = [0, len_a // 2, len_b // 2, len_a, len_b, len_a * 2, len_b * 2]
    python_a = int(hex_string_a, 16)
    python_b = int(hex_string_b, 16)
    for n in numbers:
        big_int_instance_res = big_int_lib.shiftAdd(big_int_instance_a, big_int_instance_b, n)
        assert big_int_lib.isValidBigInt(big_int_instance_res)
        hex_str = big_int_lib.bigIntToHexString(big_int_instance_res)
        big_int_lib.freeBigInt(big_int_instance_res)

        python_res = python_a + (python_b << n * 64)
        python_hex_string = hex(python_res).replace("0x", "").upper()
        assert python_hex_string == hex_str.decode(), f"{bigInt_to_python_hex_string(big_int_instance_a)} {bigInt_to_python_hex_string(big_int_instance_b)} toShift was {n}"
