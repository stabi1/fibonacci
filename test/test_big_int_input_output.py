import pytest

from big_int_helper import big_int_lib, random_hex_string, random_dec_string, getPartialBigInt, trim_string, \
    get_all_test_numbers


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


@pytest.mark.parametrize("test_hex_string", get_all_test_numbers())
def test_hexStringToBigInt_and_bigIntToHexString(test_hex_string: str):
    big_int_instance = big_int_lib.hexStringToBigInt(test_hex_string.encode())
    assert big_int_lib.isValidBigInt(big_int_instance)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance)
    assert hex_str.decode() == test_hex_string


@pytest.mark.parametrize("test_dec_string", get_all_test_numbers())
def test_decStringToBigInt_and_bigIntToDecString(test_dec_string: str):
    return
    big_int_instance = big_int_lib.decStringToBigInt(test_dec_string.encode())
    assert big_int_lib.isValidBigInt(big_int_instance)
    dec_str = big_int_lib.bigIntToDecString(big_int_instance)
    assert dec_str.decode() == test_dec_string


def test_start_attribute_with_hexStringToBigInt():
    test_hex_string = random_hex_string(16 * 10)
    big_int_instance = big_int_lib.hexStringToBigInt(test_hex_string.encode())
    big_int_instance = getPartialBigInt(big_int_instance, 3, 6)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance)
    assert hex_str.decode() == trim_string(test_hex_string, 4 * 16, 3 * 16), f"org: {test_hex_string}"


def test_start_attribute_with_decStringToBigInt():
    return
    test_hex_string = random_dec_string(19 * 10)
    big_int_instance = big_int_lib.hexStringToBigInt(test_hex_string.encode())
    big_int_instance = getPartialBigInt(big_int_instance, 3, 6)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance)
    assert hex_str.decode() == trim_string(test_hex_string, 4 * 19, 3 * 19), f"org: {test_hex_string}"
