import pytest
import gmpy2

from pathlib import Path

from run_big_int import run_command_in_valgrind
from big_int_helper import command_list_permuted, prepare_test_folder, compute_test_helper

n_list = [0, 1, 2, 5, 10, 42, 69, 102, 999, 10000, 1000000]
test_list = [(n, flags) for n in n_list for flags in command_list_permuted]


@pytest.mark.valgrind_test
@pytest.mark.parametrize("n, flags", test_list)
def test_fib_correctness(tmp_path: Path, n: int, flags: str):
    output_file = "out.txt"

    prepare_test_folder(tmp_path)
    command = f"./fib -o f -r d --output-filename {output_file} --compute fibonacci {n} {flags.strip()}"
    run_command_in_valgrind(command, tmp_path)

    compute_test_helper(tmp_path, output_file, command, reference_implementation_fib, n)


# TODO: use gmpy2?
def reference_implementation_fib(n: int) -> str:
    if n < 0:
        raise ValueError("n must be greater than 0")
    return str(gmpy2.fib(n))
