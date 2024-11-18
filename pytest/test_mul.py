import pytest
from big_int_helper import big_int_lib, get_all_test_number_pairs


# TODO Test start and end attributes

@pytest.mark.parametrize("hex_string_a, hex_string_b", get_all_test_number_pairs())
def test_mul_sign_and_different_size_numbers(hex_string_a: str, hex_string_b: str):
    big_int_instance_a = big_int_lib.hexStringToBigInt(hex_string_a.encode())
    big_int_instance_b = big_int_lib.hexStringToBigInt(hex_string_b.encode())
    big_int_instance_res = big_int_lib.mul(big_int_instance_a, big_int_instance_b)
    assert big_int_lib.isValidBigInt(big_int_instance_res)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance_res)

    python_a = int(hex_string_a, 16)
    python_b = int(hex_string_b, 16)
    python_res = python_a * python_b
    python_hex_string = hex(python_res).replace("0x", "").upper()
    assert python_hex_string == hex_str.decode()