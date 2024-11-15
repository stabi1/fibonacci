from big_int_helper import big_int_lib, getPartialBigInt
import pytest


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
    assert blocks == length


def test_getOccupiedBlocks_partialBigInt():
    big_int_instance = big_int_lib.hexStringToBigInt((32 * 16 * "1").encode())
    big_int_instance = getPartialBigInt(big_int_instance, 3, 6)
    blocks = big_int_lib.getOccupiedBlocks(big_int_instance)
    assert blocks == 3


def test_getOccupiedBlocks_onlyZeroBlocks():
    big_int_instance = big_int_lib.newBigInt(10)
    blocks = big_int_lib.getOccupiedBlocks(big_int_instance)
    assert blocks == 1
