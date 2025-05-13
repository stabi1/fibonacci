import pytest

from pathlib import Path

from big_int_helper import arithmetic_test_helper, OpsTestArithmetic, prepare_test_folder, all_test_number_pairs_names, add_partial_big_int_to_filename, \
    all_test_number_pairs_small_names
from run_big_int import run_command_in_valgrind


@pytest.mark.valgrind_test
@pytest.mark.parametrize("filename_a, filename_b", all_test_number_pairs_names)
def test_shift_add_sign_and_different_size_numbers_valgrind(tmp_path: Path, filename_a: str, filename_b: str):
    output_file = "out.txt"
    prepare_test_folder(tmp_path, filename_a, filename_b)

    with open(tmp_path.joinpath(filename_a)) as f:
        number_a: str = f.read()
        len_a = len(number_a) // 16 + 1
    with open(tmp_path.joinpath(filename_b)) as f:
        number_b: str = f.read()
        len_b = len(number_b) // 16 + 1
    numbers = [0, 1, len_a // 2, len_b // 2, len_a, len_b, len_a * 2, len_b * 2]

    for n in numbers:
        command = f"./fib -t shiftAdd,{filename_a},{filename_b},{n},{output_file}"
        program_error: bool = run_command_in_valgrind(command, tmp_path)

        arithmetic_test_helper(OpsTestArithmetic.SHIFT_ADD, program_error, tmp_path, filename_a, output_file, command, filename_b=filename_b, integer=n)


@pytest.mark.valgrind_test
@pytest.mark.partialBigInt_test
@pytest.mark.parametrize("filename_a, filename_b", all_test_number_pairs_small_names)
def test_shift_add_partial_bigints_valgrind(tmp_path: Path, filename_a: str, filename_b: str):
    output_file = "out.txt"
    prepare_test_folder(tmp_path, filename_a, filename_b)

    with open(tmp_path.joinpath(filename_a)) as f:
        number_a: str = f.read()
        len_a = len(number_a) // 16 + 1
    with open(tmp_path.joinpath(filename_b)) as f:
        number_b: str = f.read()
        len_b = len(number_b) // 16 + 1
    numbers = [0, 1, len_a // 2, len_b // 2, len_a, len_b, len_a * 2, len_b * 2]

    for n in numbers:
        command = f"./fib -t shiftAdd,{add_partial_big_int_to_filename(filename_a)},{add_partial_big_int_to_filename(filename_b)},{n},{output_file}"
        program_error: bool = run_command_in_valgrind(command, tmp_path)

        arithmetic_test_helper(OpsTestArithmetic.SHIFT_ADD, program_error, tmp_path, filename_a, output_file, command, filename_b=filename_b, integer=n)
