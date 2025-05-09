import os
import subprocess
import sys
from pathlib import Path
from typing import List

import pytest


def format_args(args: List[str]) -> str:
    args_str = f", program <{str(args[0]).split('/')[-1]}> was called with: "
    tmp_str = ""
    for i in range(1, len(args)):
        tmp_str += " " + args[i]
    args_str += tmp_str.strip() if tmp_str != "" else "No Arguments"
    return args_str.strip()


def run_command_in_valgrind(command: str, path: Path, timeout: int = 100000) -> bool:
    valgrind_error = 1000
    command = command.strip()
    opt = str(command).split(' ')

    opt = ["valgrind", "--leak-check=full", f"--error-exitcode={valgrind_error}"] + opt

    result_valgrind: subprocess.CompletedProcess | None = None
    try:
        minimal_env = {"PATH": os.environ["PATH"]}
        result_valgrind: subprocess.CompletedProcess = subprocess.run(opt, capture_output=True, cwd=path, timeout=timeout, env=minimal_env)
    except subprocess.TimeoutExpired:
        pytest.skip(f"Valgrind call timed out")
    except Exception as e:
        pytest.fail(f"Error occurred while trying to run valgrind on student submission; {repr(e)}{format_args(list(map(str, opt)))}")

    if result_valgrind.returncode < 0:
        pytest.fail(f"Valgrind CRASHED!!!{format_args(result_valgrind.args)}")

    if result_valgrind.returncode == valgrind_error:
        os.write(sys.stderr.fileno(), result_valgrind.stderr)
        os.write(sys.stdout.fileno(), result_valgrind.stdout)
        pytest.fail(f"Valgrind error!{format_args(result_valgrind.args)}")
    elif result_valgrind.returncode != 0:
        os.write(sys.stderr.fileno(), result_valgrind.stderr)
        os.write(sys.stdout.fileno(), result_valgrind.stdout)
        return True
    return False
