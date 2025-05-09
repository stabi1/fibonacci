from pathlib import Path

import pytest
from big_int_helper import add_partial_big_int_to_filename, all_test_number_pairs_names, prepare_test_folder, arithmetic_test_helper, OpsTestArithmetic, \
    all_test_number_pairs_small_names
from run_big_int import run_command_in_valgrind


@pytest.mark.valgrind_test
@pytest.mark.parametrize("filename_a, filename_b", all_test_number_pairs_names)
def test_add_sign_and_different_size_numbers_valgrind(tmp_path: Path, filename_a: str, filename_b: str):
    output_file = "out.txt"
    prepare_test_folder(tmp_path, filename_a, filename_b)

    command = f"./fib -t add,{filename_a},{filename_b},{output_file}"
    program_error: bool = run_command_in_valgrind(command, tmp_path)

    arithmetic_test_helper(OpsTestArithmetic.ADD, program_error, tmp_path, filename_a, output_file, command, filename_b=filename_b)


@pytest.mark.valgrind_test
@pytest.mark.partialBigInt_test
@pytest.mark.parametrize("filename_a, filename_b", all_test_number_pairs_small_names)
def test_add_partial_bigints_valgrind(tmp_path: Path, filename_a: str, filename_b: str):
    output_file = "out.txt"
    prepare_test_folder(tmp_path, filename_a, filename_b)

    command = f"./fib -t add,{add_partial_big_int_to_filename(filename_a)},{add_partial_big_int_to_filename(filename_b)},{output_file}"
    program_error: bool = run_command_in_valgrind(command, tmp_path)

    arithmetic_test_helper(OpsTestArithmetic.ADD, program_error, tmp_path, filename_a, output_file, command, filename_b=filename_b)


@pytest.mark.valgrind_test
@pytest.mark.parametrize("filename_a, filename_b", all_test_number_pairs_names)
def test_sub_sign_and_different_size_numbers_valgrind(tmp_path: Path, filename_a: str, filename_b: str):
    output_file = "out.txt"
    prepare_test_folder(tmp_path, filename_a, filename_b)

    command = f"./fib -t sub,{filename_a},{filename_b},{output_file}"
    program_error: bool = run_command_in_valgrind(command, tmp_path)

    arithmetic_test_helper(OpsTestArithmetic.SUB, program_error, tmp_path, filename_a, output_file, command, filename_b=filename_b)


@pytest.mark.valgrind_test
@pytest.mark.partialBigInt_test
@pytest.mark.parametrize("filename_a, filename_b", all_test_number_pairs_small_names)
def test_sub_partial_bigints_valgrind(tmp_path: Path, filename_a: str, filename_b: str):
    output_file = "out.txt"
    prepare_test_folder(tmp_path, filename_a, filename_b)

    command = f"./fib -t sub,{add_partial_big_int_to_filename(filename_a)},{add_partial_big_int_to_filename(filename_b)},{output_file}"
    program_error: bool = run_command_in_valgrind(command, tmp_path)

    arithmetic_test_helper(OpsTestArithmetic.SUB, program_error, tmp_path, filename_a, output_file, command, filename_b=filename_b)
