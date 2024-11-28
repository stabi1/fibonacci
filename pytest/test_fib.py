import sys
from pathlib import Path
from typing import Tuple

import pytest

from run_big_int import run_command_in_valgrind

n_list = [0, 1, 2, 5, 10, 42, 69, 102, 999, 10000, 1000000]

LOCAL_DIR = Path(__file__).parent


@pytest.mark.parametrize("n", n_list)
def test_fib_correctness(n: int):
    output_file = "out.txt"
    sys.set_int_max_str_digits(1000000)

    command = f"./fib -o f -r d --output-filename {output_file} -n {n}"
    run_command_in_valgrind(command)

    with open(LOCAL_DIR.joinpath(output_file), 'r') as f:
        res_string = f.read()
    LOCAL_DIR.joinpath(output_file).unlink()

    python_res_string = str(reference_implementation(n))

    assert res_string == python_res_string.upper(), f"Command: {command}"


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
