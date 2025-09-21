from pathlib import Path
import pytest
import sys

from big_int_helper import prepare_test_folder, all_test_numbers_names, all_dec_test_numbers_names
from run_big_int import run_command_in_valgrind


@pytest.mark.valgrind_test
@pytest.mark.parametrize("filename_a_dec", all_dec_test_numbers_names)
def test_dec_string_conversion_valgrind(tmp_path: Path, filename_a_dec: str):
    output_file = "out.txt"

    prepare_test_folder(tmp_path, filename_a_dec)

    command: str = f"./fib --compute convert-number --input-filename {filename_a_dec} --output-filename {output_file} --input-radix d --output-radix d --output-format f"
    run_command_in_valgrind(command, tmp_path)

    with open(tmp_path.joinpath(output_file), 'r') as f:
        res_string = f.read()
    with open(tmp_path.joinpath(filename_a_dec), 'r') as f:
        test_dec_string = f.read()

    if test_dec_string == "-0":
        test_dec_string = "0"

    assert res_string == test_dec_string, f"Command: {command}"


@pytest.mark.valgrind_test
@pytest.mark.parametrize("filename_a", all_test_numbers_names)
def test_hex_string_conversion_valgrind(tmp_path: Path, filename_a: str):
    output_file = "out.txt"

    prepare_test_folder(tmp_path, filename_a)
    command: str = f"./fib --compute convert-number --input-filename {filename_a} --output-filename {output_file} --input-radix h --output-radix h --output-format f"
    run_command_in_valgrind(command, tmp_path)

    with open(tmp_path.joinpath(output_file), 'r') as f:
        res_string = f.read()
    with open(tmp_path.joinpath(filename_a), 'r') as f:
        test_string = f.read()

    if test_string == "-0":
        test_string = "0"

    assert res_string == test_string, f"Command: {command}"


@pytest.mark.valgrind_test
@pytest.mark.parametrize("filename_a", all_test_numbers_names)
def test_dec_string_partial_bigint_valgrind(tmp_path: Path, filename_a: str):
    output_file = "out.txt"

    prepare_test_folder(tmp_path, filename_a)

    command = f"./fib -t printDec,{filename_a},{output_file}"
    run_command_in_valgrind(command, tmp_path)

    with open(tmp_path.joinpath(output_file), 'r') as f:
        res_string = f.read()
    with open(tmp_path.joinpath(filename_a), 'r') as f:
        test_hex_string = f.read()

    if test_hex_string == "-0":
        test_hex_string = "0"

    sys.set_int_max_str_digits(1000000)
    assert res_string == str(int(test_hex_string, 16)), f"Command: {command}"


@pytest.mark.valgrind_test
@pytest.mark.parametrize("filename_a", all_test_numbers_names)
def test_hex_string_partial_bigint_valgrind(tmp_path: Path, filename_a: str):
    output_file = "out.txt"

    prepare_test_folder(tmp_path, filename_a)
    command = f"./fib -t printHex,{filename_a},{output_file}"
    run_command_in_valgrind(command, tmp_path)

    with open(tmp_path.joinpath(output_file), 'r') as f:
        res_string = f.read()
    with open(tmp_path.joinpath(filename_a), 'r') as f:
        test_string = f.read()

    if test_string == "-0":
        test_string = "0"

    assert res_string == test_string, f"Command: {command}"
