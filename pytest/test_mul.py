import shutil
from pathlib import Path

import pytest
from big_int_helper import all_test_number_pairs_names, FIB_DIR, add_partial_big_int_to_filename, all_test_number_pairs_small_names, NUMER_FILES_PATH
from run_big_int import run_command_in_valgrind


@pytest.mark.valgrind_test
@pytest.mark.parametrize("filename_a, filename_b", all_test_number_pairs_names)
def test_mul_sign_and_different_size_numbers_valgrind(tmp_path: Path, filename_a: str, filename_b: str):
    output_file = "out.txt"

    # copy binary
    src = FIB_DIR.joinpath("fib")
    dest = tmp_path.joinpath("fib")
    shutil.copy(src, dest)

    if not tmp_path.joinpath(filename_a).exists():
        shutil.copy(NUMER_FILES_PATH.joinpath(filename_a), tmp_path.joinpath(filename_a))
    if not tmp_path.joinpath(filename_b).exists():
        shutil.copy(NUMER_FILES_PATH.joinpath(filename_b), tmp_path.joinpath(filename_b))

    command = f"./fib -t mul,{filename_a},{filename_b},{output_file}"
    run_command_in_valgrind(command, tmp_path)

    with open(tmp_path.joinpath(output_file), 'r') as f:
        res_string = f.read()
    tmp_path.joinpath(output_file).unlink()

    with open(tmp_path.joinpath(filename_a), 'r') as f:
        hex_string_a = f.read()
    with open(tmp_path.joinpath(filename_b), 'r') as f:
        hex_string_b = f.read()

    python_a = int(hex_string_a, 16)
    python_b = int(hex_string_b, 16)
    python_res = python_a * python_b
    python_res_string = hex(python_res).replace("0x", "").upper()

    assert res_string == python_res_string.upper(), f"Command: {command}"


@pytest.mark.valgrind_test
@pytest.mark.partialBigInt_test
@pytest.mark.parametrize("filename_a, filename_b", all_test_number_pairs_small_names)
def test_mul_partial_bigints_valgrind(tmp_path: Path, filename_a: str, filename_b: str):
    output_file = "out.txt"

    # copy binary
    src = FIB_DIR.joinpath("fib")
    dest = tmp_path.joinpath("fib")
    shutil.copy(src, dest)

    if not tmp_path.joinpath(filename_a).exists():
        shutil.copy(NUMER_FILES_PATH.joinpath(filename_a), tmp_path.joinpath(filename_a))
    if not tmp_path.joinpath(filename_b).exists():
        shutil.copy(NUMER_FILES_PATH.joinpath(filename_b), tmp_path.joinpath(filename_b))

    command = f"./fib -t mul,{add_partial_big_int_to_filename(filename_a)},{add_partial_big_int_to_filename(filename_b)},{output_file}"
    run_command_in_valgrind(command, tmp_path)

    with open(tmp_path.joinpath(output_file), 'r') as f:
        res_string = f.read()
    tmp_path.joinpath(output_file).unlink()

    with open(tmp_path.joinpath(filename_a), 'r') as f:
        hex_string_a = f.read()
    with open(tmp_path.joinpath(filename_b), 'r') as f:
        hex_string_b = f.read()

    python_a = int(hex_string_a, 16)
    python_b = int(hex_string_b, 16)
    python_res = python_a * python_b
    python_res_string = hex(python_res).replace("0x", "").upper()

    assert res_string == python_res_string.upper(), f"Command: {command}"
