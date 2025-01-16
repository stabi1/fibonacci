import ctypes
import shutil
from pathlib import Path
from typing import Tuple

import pytest
from big_int_helper import big_int_lib, all_test_number_pairs, partial_bigInts_pairs, BigInt, FIB_DIR
from run_big_int import run_command_in_valgrind

@pytest.mark.parametrize("hex_string_a, hex_string_b", all_test_number_pairs)
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


@pytest.mark.valgrind_test
@pytest.mark.parametrize("hex_string_a, hex_string_b", all_test_number_pairs)
def test_mul_sign_and_different_size_numbers_valgrind(tmp_path: Path, hex_string_a: str, hex_string_b: str):
    output_file = "out.txt"
    input1_file = "input1.txt"
    input2_file = "input2.txt"

    src = FIB_DIR.joinpath("fib")
    dest = tmp_path.joinpath("fib")
    shutil.copy(src, dest)
    with open(tmp_path.joinpath(input1_file), 'w') as f:
        f.write(hex_string_a)
    with open(tmp_path.joinpath(input2_file), 'w') as f:
        f.write(hex_string_b)
    command = f"./fib -t mul,{input1_file},{input2_file},{output_file}"
    run_command_in_valgrind(command, tmp_path)

    with open(tmp_path.joinpath(output_file), 'r') as f:
        res_string = f.read()
    tmp_path.joinpath(output_file).unlink()

    python_a = int(hex_string_a, 16)
    python_b = int(hex_string_b, 16)
    python_res = python_a * python_b
    python_res_string = hex(python_res).replace("0x", "").upper()

    assert res_string == python_res_string.upper(), f"Command: {command}"


@pytest.mark.parametrize("tuple_a, tuple_b", partial_bigInts_pairs)
def test_mul_partial_bigints(tuple_a: Tuple[ctypes.POINTER(BigInt), str],
                             tuple_b: Tuple[ctypes.POINTER(BigInt), str]):
    big_int_instance_a, hex_string_a = tuple_a
    big_int_instance_b, hex_string_b = tuple_b

    big_int_instance_res = big_int_lib.mul(big_int_instance_a, big_int_instance_b)
    assert big_int_lib.isValidBigInt(big_int_instance_res)
    hex_str = big_int_lib.bigIntToHexString(big_int_instance_res)

    python_a = int(hex_string_a, 16)
    python_b = int(hex_string_b, 16)
    python_res = python_a * python_b
    python_hex_string = hex(python_res)

    assert python_hex_string.replace("0x", "").upper() == hex_str.decode()
