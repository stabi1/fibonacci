""" Tries to find the best values for the program"""
import argparse
import sys
import pathlib
import random
import subprocess
import shutil

from pathlib import Path

random.seed(42)
current_dir: Path = pathlib.Path(__file__).parent
uint64_max_value: int = 18446744073709551615


def random_hex_string(length: int) -> str:
    """Generate a random hex string of a specified length"""
    # First character should not be '0'
    hex_string = random.choice('123456789ABCDEF')
    hex_string += ''.join(random.choice('0123456789ABCDEF') for _ in range(length - 1))
    return hex_string


def run_program(size: int, k_values: list[int] = None, mul_thresholds=None) -> float:
    delimiter = ","

    if k_values is not None:
        args_fib = ["./fib", "-b", "-t", f"ssa-small{delimiter}{"file1.txt"}{delimiter}{"file2.txt"}{delimiter}{"res.txt"}", "--k-values",
                    str(k_values).removeprefix("[").removesuffix("]")]
    elif mul_thresholds is not None:
        args_fib = ["./fib", "-b", "-t", f"mul{delimiter}{"file1.txt"}{delimiter}{"file2.txt"}{delimiter}{"res.txt"}", "--mul-thresholds",
                    str(mul_thresholds).removeprefix("[").removesuffix("]")]
    else:
        print("Either k_values or mul_thresholds must be set")
        exit(-1)

    print(args_fib)
    result = subprocess.run(args_fib, capture_output=True, text=True, cwd=str(current_dir))
    if result.returncode != 0:
        print(f"Program returned with error code {result.returncode}\nstdout: {result.stdout}\nstderr: {result.stderr}")
        exit(-1)
    time: float = -1
    for line in result.stdout.splitlines():
        if line.startswith("Time:"):
            time = float(line.removeprefix("Time: "))
    if time == -1:
        print(f"No time found: {result.stdout}")
        exit(-1)

    print(result.stdout)
    current_dir.joinpath("res.txt").unlink()
    return time


def find_best_k_values():
    k_limit = 20
    current_k_index = 0

    k_values: list[int] = [1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20]

    shutil.copy(current_dir.parent.joinpath("fib"), current_dir)

    current_dir.joinpath("file1.txt").unlink(missing_ok=True)
    current_dir.joinpath("file2.txt").unlink(missing_ok=True)
    current_dir.joinpath("file1.txt").touch()
    current_dir.joinpath("file2.txt").touch()

    for start, stop, step in [
        (10, 1000, 10),
        (1000, 10000, 100),
        (10000, 100000, 1000),
        (100000, 1000000, 10000),
        (1000000, 10000000, 1000000),
        (10000000, 100000000, 10000000),
    ]:
        for size in range(start, stop, step):
            to_add = step
            if size == 1000 or size == 10000 or size == 100000:
                to_add = step // 100
            elif size == 1000000 or size == 10000000:
                to_add = step // 1000

            with (current_dir / "file1.txt").open("a") as f:
                f.write(random_hex_string(to_add * 16))
            with (current_dir / "file2.txt").open("a") as f:
                f.write(random_hex_string(to_add * 16))
            with (current_dir / "file2.txt").open("r") as f:
                print(len(f.read()) / 16)

            k_copy = k_values.copy()

            k_copy[current_k_index] = size
            time1 = run_program(size, k_values=k_copy)
            k_copy[current_k_index] = 0

            k_copy[current_k_index + 1] = size
            time2 = run_program(size, k_values=k_copy)
            k_copy[current_k_index + 1] = 0

            if time1 > time2:
                print(f"{16 * '-'}")
                k_values[current_k_index] = size
                current_k_index += 1
                if current_k_index + 1 >= len(k_values):
                    print("k limit reached")
                    current_dir.joinpath("file1.txt").unlink()
                    current_dir.joinpath("file2.txt").unlink()
                    current_dir.joinpath("fib").unlink()
                    print(k_values)
                    return

    print("Size limit reached")
    current_dir.joinpath("file1.txt").unlink()
    current_dir.joinpath("file2.txt").unlink()
    current_dir.joinpath("fib").unlink()
    print(k_values)
    return


def find_best_mul_thresholds():
    shutil.copy(current_dir.parent.joinpath("fib"), current_dir)

    current_dir.joinpath("file1.txt").unlink(missing_ok=True)
    current_dir.joinpath("file2.txt").unlink(missing_ok=True)
    current_dir.joinpath("file1.txt").write_text(random_hex_string(100000 * 16))
    current_dir.joinpath("file2.txt").write_text(random_hex_string(100000 * 16))

    current_mul_index = 0
    mul_thresholds: list[int] = [uint64_max_value, uint64_max_value, uint64_max_value, uint64_max_value]
    for start, stop, step in [
        (10, 1000, 10),
        (1000, 10000, 100),
        (10000, 100000, 1000),
    ]:
        for size in range(start, stop, step):
            mul_thresholds_copy = mul_thresholds.copy()

            mul_thresholds_copy[current_mul_index] = size
            time1 = run_program(size, mul_thresholds=mul_thresholds_copy)

            mul_thresholds_copy[current_mul_index] = size + step
            time2 = run_program(size, mul_thresholds=mul_thresholds_copy)

            if time1 < time2:
                print(f"{16 * '-'}")
                mul_thresholds[current_mul_index] = size
                current_mul_index += 1
                if current_mul_index >= len(mul_thresholds):
                    print("k limit reached")
                    current_dir.joinpath("file1.txt").unlink()
                    current_dir.joinpath("file2.txt").unlink()
                    current_dir.joinpath("fib").unlink()
                    print(mul_thresholds)
                    return

    print("Size limit reached")
    current_dir.joinpath("file1.txt").unlink()
    current_dir.joinpath("file2.txt").unlink()
    current_dir.joinpath("fib").unlink()
    print(mul_thresholds)
    return


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Monitor process")
    # Add arguments
    parser.add_argument('--k-values', action='store_true', help='If set calculate k values', dest='k_values')
    parser.add_argument('--mul-thresholds', action='store_true', help='If set calculate mul thresholds', dest='mul_thresholds')

    args = parser.parse_args()
    if args.k_values:
        find_best_k_values()
    elif args.mul_thresholds:
        find_best_mul_thresholds()

    sys.exit(0)
