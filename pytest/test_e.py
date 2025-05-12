import pytest
import math
import gmpy2

from pathlib import Path
from run_big_int import run_command_in_valgrind, run_command_non_valgrind
from big_int_helper import command_list_permuted, prepare_test_folder, compute_test_helper

n_list = [0, 1, 2, 5, 10, 42, 69, 102, 999, 10000]
n_list_non_valgrind = [1000000]
test_list = [(n, flags) for n in n_list for flags in command_list_permuted]
test_list_non_valgrind = [(n, flags) for n in n_list_non_valgrind for flags in command_list_permuted]


@pytest.mark.valgrind_test
@pytest.mark.parametrize("n, flags", test_list)
def test_e_correctness_valgrind(tmp_path: Path, n: int, flags: str):
    output_file = "out.txt"

    prepare_test_folder(tmp_path)
    command = f"./fib -o f -r d --output-filename {output_file} --compute e {n} {flags.strip()}"
    run_command_in_valgrind(command, tmp_path)

    compute_test_helper(tmp_path, output_file, command, reference_implementation_e, n)


@pytest.mark.parametrize("n, flags", test_list_non_valgrind)
def test_e_correctness(tmp_path: Path, n: int, flags: str):
    output_file = "out.txt"

    prepare_test_folder(tmp_path)
    command = f"./fib -o f -r d --output-filename {output_file} --compute e {n} {flags.strip()}"
    run_command_non_valgrind(command, tmp_path)

    compute_test_helper(tmp_path, output_file, command, reference_implementation_e, n)


def reference_implementation_e(n: int) -> str:
    if n < 0:
        raise ValueError("Number of digits must be non-negative")

    # MPFR precision is in bits: log2(10) ≈ 3.32193, add a safety margin
    bits = math.ceil((n + 5) * 3.32193)
    gmpy2.get_context().precision = bits

    e = gmpy2.exp(gmpy2.mpfr(1))

    e_str = str(e)
    res = e_str[:n + 2]
    if n == 0:
        res = res + "0"
    return res
