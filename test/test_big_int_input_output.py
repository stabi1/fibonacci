import pytest

from big_int_helper import big_int_lib, random_hex_string, random_dec_string, getPartialBigInt, trim_string


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


@pytest.mark.parametrize("test_dec_string", [
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
    random_dec_string(1000),
    random_dec_string(10000),
])
def test_decStringToBigInt_and_bigIntToDecString(test_dec_string: str):
    # test positive
    big_int_instance = big_int_lib.decStringToBigInt(test_dec_string.encode())
    dec_str = big_int_lib.bigIntToDecString(big_int_instance)
    assert dec_str.decode() == test_dec_string

    # negative
    # TODO fix
    '''test_dec_string = "-" + test_dec_string
    big_int_instance = big_int_lib.decStringToBigInt(test_dec_string.encode())
    assert big_int_instance.contents.negative == True;
    dec_str = big_int_lib.bigIntToDecString(big_int_instance)
    assert dec_str.decode() == test_dec_string'''


def test_start_attribute_with_hexStringToBigInt():
    test_hex_string = random_hex_string(16 * 10)
    big_int_instance = big_int_lib.hexStringToBigInt(test_hex_string.encode())
    big_int_instance = getPartialBigInt(big_int_instance, 3, 6)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance)
    assert hex_str.decode() == trim_string(test_hex_string, 4 * 16, 3 * 16), f"org: {test_hex_string}"
