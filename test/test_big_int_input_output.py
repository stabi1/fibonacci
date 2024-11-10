import pytest

from big_int_helper import big_int_lib, random_hex_string


def test_new_big_int_is_zero():
    big_int_instance = big_int_lib.newBigInt(1)
    big_int = big_int_instance.contents
    assert big_int.bigIntArray[0] == 0


def test_bigIntToHexString_for_zero():
    big_int_instance = big_int_lib.newBigInt(1)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance)
    assert hex_str.decode() == "0"


def test_bigIntToDecString_for_zero():
    big_int_instance = big_int_lib.newBigInt(1)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance)
    assert hex_str.decode() == "0"


@pytest.mark.parametrize("test_hex_string", [
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
    random_hex_string(1000),
    random_hex_string(10000),
])
def test_hexStringToBigInt_and_bigIntToHexString(test_hex_string: str):
    # test positive
    big_int_instance = big_int_lib.hexStringToBigInt(test_hex_string.encode())
    hex_str = big_int_lib.bigIntToHexString(big_int_instance)
    assert hex_str.decode() == test_hex_string

    # negative
    test_hex_string = "-" + test_hex_string
    big_int_instance = big_int_lib.hexStringToBigInt(test_hex_string.encode())
    hex_str = big_int_lib.bigIntToHexString(big_int_instance)
    assert hex_str.decode() == test_hex_string
