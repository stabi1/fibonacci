from pathlib import Path

import pytest
from big_int_helper import all_test_number_pairs_names, add_partial_big_int_to_filename, all_test_number_pairs_small_names, prepare_test_folder, \
    OpsTestArithmetic, arithmetic_test_helper
from run_big_int import run_command_in_valgrind


@pytest.mark.valgrind_test
@pytest.mark.parametrize("filename_a, filename_b", all_test_number_pairs_names)
def test_div_sign_and_different_size_numbers_valgrind(tmp_path: Path, filename_a: str, filename_b: str):
    output_file = "out.txt"
    prepare_test_folder(tmp_path, filename_a, filename_b)

    command = f"./fib -t div,{filename_a},{filename_b},{output_file}"
    program_error: bool = run_command_in_valgrind(command, tmp_path)

    arithmetic_test_helper(OpsTestArithmetic.DIV, program_error, tmp_path, filename_a, output_file, command, filename_b=filename_b)


@pytest.mark.valgrind_test
@pytest.mark.partialBigInt_test
@pytest.mark.parametrize("filename_a, filename_b", all_test_number_pairs_small_names)
def test_div_partial_bigints_valgrind(tmp_path: Path, filename_a: str, filename_b: str):
    output_file = "out.txt"
    prepare_test_folder(tmp_path, filename_a, filename_b)

    command = f"./fib -t div,{add_partial_big_int_to_filename(filename_a)},{add_partial_big_int_to_filename(filename_b)},{output_file}"
    program_error: bool = run_command_in_valgrind(command, tmp_path)

    arithmetic_test_helper(OpsTestArithmetic.DIV, program_error, tmp_path, filename_a, output_file, command, filename_b=filename_b)


@pytest.mark.valgrind_test
@pytest.mark.parametrize("filename_a, filename_b", all_test_number_pairs_names)
def test_div_mod_sign_and_different_size_numbers_valgrind(tmp_path: Path, filename_a: str, filename_b: str):
    output_file = "div.txt"
    output_file_2 = "mod.txt"
    prepare_test_folder(tmp_path, filename_a, filename_b)

    command = f"./fib -t divMod,{filename_a},{filename_b},{output_file},{output_file_2}"
    program_error: bool = run_command_in_valgrind(command, tmp_path)

    arithmetic_test_helper(OpsTestArithmetic.DIV_MOD, program_error, tmp_path, filename_a, output_file, command, filename_b=filename_b, output_file_2=output_file_2)


@pytest.mark.valgrind_test
@pytest.mark.partialBigInt_test
@pytest.mark.parametrize("filename_a, filename_b", all_test_number_pairs_small_names)
def test_div_mod_partial_bigints_valgrind(tmp_path: Path, filename_a: str, filename_b: str):
    output_file = "div.txt"
    output_file_2 = "mod.txt"
    prepare_test_folder(tmp_path, filename_a, filename_b)

    command = f"./fib -t divMod,{add_partial_big_int_to_filename(filename_a)},{add_partial_big_int_to_filename(filename_b)},{output_file},{output_file_2}"
    program_error: bool = run_command_in_valgrind(command, tmp_path)

    arithmetic_test_helper(OpsTestArithmetic.DIV_MOD, program_error, tmp_path, filename_a, output_file, command, filename_b=filename_b, output_file_2=output_file_2)
