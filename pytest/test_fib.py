import shutil
import sys
from pathlib import Path
from typing import Tuple

import pytest

from run_big_int import run_command_in_valgrind
from big_int_helper import command_list_permuted, FIB_DIR

n_list = [0, 1, 2, 5, 10, 42, 69, 102, 999, 10000, 1000000]
test_list = [(n, flags) for n in n_list for flags in command_list_permuted]


@pytest.mark.valgrind_test
@pytest.mark.parametrize("n, flags", test_list)
def test_fib_correctness(tmp_path: Path, n: int, flags: str):
    output_file = "out.txt"
    sys.set_int_max_str_digits(1000000)

    src = FIB_DIR.joinpath("fib")
    dest = tmp_path.joinpath("fib")
    shutil.copy(src, dest)
    command = f"./fib -o f -r d --output-filename {output_file} --compute fibonacci {n} {flags.strip()}"
    run_command_in_valgrind(command, tmp_path)

    with open(tmp_path.joinpath(output_file), 'r') as f:
        res_string = f.read()
    tmp_path.joinpath(output_file).unlink()

    python_res_string = str(reference_implementation(n))

    assert res_string == python_res_string.upper(), f"Command: {command}"


# TODO: use gmpy2?
def reference_implementation(n: int) -> int:
    if n < 0:
        raise ValueError("n must be greater than 0")
    return _fib(n)[0]


def _fib(n) -> Tuple[int, int]:
    if n == 0:
        return 0, 1
    else:
        a, b = _fib(n // 2)
        c = a * (b * 2 - a)
        d = a * a + b * b
        if n % 2 == 0:
            return c, d
        else:
            return d, c + d
