from pathlib import Path

import pytest
from big_int_helper import all_test_numbers_names, add_partial_big_int_to_filename, all_test_numbers_small_names, prepare_test_folder, \
    OpsTestArithmetic, arithmetic_test_helper
from run_big_int import run_command_in_valgrind

shift_test_list = [0, 1, 2, 3, 7, 8] #, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 511, 512, 513, 1000, 10000]

@pytest.mark.valgrind_test
@pytest.mark.parametrize("filename_a", all_test_numbers_names)
def test_leftshift_sign_and_different_size_numbers_valgrind(tmp_path: Path, filename_a: str):
    output_file = "out.txt"
    prepare_test_folder(tmp_path, filename_a)

    for i in shift_test_list:
        command = f"./fib -t shiftLeft,{filename_a},{i},{output_file}"
        program_error: bool = run_command_in_valgrind(command, tmp_path)

        arithmetic_test_helper(OpsTestArithmetic.SHIFT_LEFT, program_error, tmp_path, filename_a, output_file, command, integer=i)


@pytest.mark.valgrind_test
@pytest.mark.partialBigInt_test
@pytest.mark.parametrize("filename_a", all_test_numbers_small_names)
def test_leftshift_partial_bigints_valgrind(tmp_path: Path, filename_a: str):
    output_file = "out.txt"
    prepare_test_folder(tmp_path, filename_a)

    for i in shift_test_list:
        command = f"./fib -t shiftLeft,{add_partial_big_int_to_filename(filename_a)},{i},{output_file}"
        program_error: bool = run_command_in_valgrind(command, tmp_path)

        arithmetic_test_helper(OpsTestArithmetic.SHIFT_LEFT, program_error, tmp_path, filename_a, output_file, command, integer=i)

@pytest.mark.valgrind_test
@pytest.mark.parametrize("filename_a", all_test_numbers_names)
def test_rightshift_sign_and_different_size_numbers_valgrind(tmp_path: Path, filename_a: str):
    output_file = "out.txt"
    prepare_test_folder(tmp_path, filename_a)

    for i in shift_test_list:
        command = f"./fib -t shiftRight,{filename_a},{i},{output_file}"
        program_error: bool = run_command_in_valgrind(command, tmp_path)

        arithmetic_test_helper(OpsTestArithmetic.SHIFT_RIGHT, program_error, tmp_path, filename_a, output_file, command, integer=i)


@pytest.mark.valgrind_test
@pytest.mark.partialBigInt_test
@pytest.mark.parametrize("filename_a", all_test_numbers_small_names)
def test_rightshift_partial_bigints_valgrind(tmp_path: Path, filename_a: str):
    output_file = "out.txt"
    prepare_test_folder(tmp_path, filename_a)

    for i in shift_test_list:
        command = f"./fib -t shiftRight,{add_partial_big_int_to_filename(filename_a)},{i},{output_file}"
        program_error: bool = run_command_in_valgrind(command, tmp_path)

        arithmetic_test_helper(OpsTestArithmetic.SHIFT_RIGHT, program_error, tmp_path, filename_a, output_file, command, integer=i)