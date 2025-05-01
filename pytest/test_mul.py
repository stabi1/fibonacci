import shutil
from pathlib import Path

import pytest
from big_int_helper import all_test_number_pairs, FIB_DIR, add_partial_big_int_to_filename, all_test_number_pairs_small
from run_big_int import run_command_in_valgrind


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


@pytest.mark.valgrind_test
@pytest.mark.partialBigInt_test
@pytest.mark.parametrize("hex_string_a, hex_string_b", all_test_number_pairs_small)
def test_mul_partial_bigints_valgrind(tmp_path: Path, hex_string_a: str, hex_string_b: str):
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
    command = f"./fib -t mul,{add_partial_big_int_to_filename(input1_file)},{add_partial_big_int_to_filename(input2_file)},{output_file}"
    run_command_in_valgrind(command, tmp_path)

    with open(tmp_path.joinpath(output_file), 'r') as f:
        res_string = f.read()
    tmp_path.joinpath(output_file).unlink()

    python_a = int(hex_string_a, 16)
    python_b = int(hex_string_b, 16)
    python_res = python_a * python_b
    python_res_string = hex(python_res).replace("0x", "").upper()

    assert res_string == python_res_string.upper(), f"Command: {command}"
