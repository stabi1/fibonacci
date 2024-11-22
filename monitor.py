#!/usr/bin/env python3
import os
from typing import List, Tuple
import matplotlib.pyplot as plt

import psutil
import time
import subprocess

data: List[Tuple[int, int, float]] = []


def calculate_plot(command: str):
    # Extracting the components from data
    memory_size = [item[0] for item in data]
    swap_size = [item[1] for item in data]
    timestamps = [item[2] for item in data]

    print(f"Max memory: {human_readable_size(max(memory_size))}; Max swap: {human_readable_size(max(swap_size))}")

    # Convert timestamps to a more readable format if needed (seconds -> hours:minutes:seconds)
    readable_time = [timestamp for timestamp in timestamps]  # Use as-is or convert if needed

    plt.figure(figsize=(10, 6))

    # Plot memory size
    plt.plot(readable_time, memory_size, label="Memory Size (KB)", color="blue", marker="o")
    # Plot swap size
    plt.plot(readable_time, swap_size, label="Swap Size (KB)", color="red", marker="x")

    plt.title("Memory and Swap Size Over Time")
    plt.suptitle(f"Command: {' '.join(command)}")
    plt.xlabel("Timestamp (seconds)")
    plt.ylabel("Size (KB)")
    plt.legend()
    plt.grid(True)

    plt.tight_layout()
    plt.savefig('memory_usage_plot.png')
    plt.show()


def human_readable_size(kb):
    units = ["KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"]
    size = kb
    unit_index = 0

    while size >= 1024 and unit_index < len(units) - 1:
        size /= 1024
        unit_index += 1

    return f"{size:.2f} {units[unit_index]}"


def get_swap_storage_size() -> str:
    script_dir = os.path.dirname(os.path.realpath(__file__))
    swap_storage_path = os.path.join(script_dir, 'swap_storage')

    # Get the size of the folder using du command, -k so the result is always in KB, -s for summary
    result = subprocess.run(['du', '-sk', swap_storage_path], stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                            text=True)
    if result.returncode == 0:
        return result.stdout.split()[0]  # Return the size from the du output
    else:
        raise Exception(f"Error retrieving value; stderr: {result.stderr}, stdout: {result.stdout}")


def monitor_process(pid) -> str:
    start_time = time.perf_counter()
    command: str = ""
    try:
        process = psutil.Process(pid)
        command = process.cmdline()
        while True:
            # Get the size of the swap_storage folder
            try:
                swap_size: int = int(get_swap_storage_size())
            except Exception as e:
                print(f"{str(e)}")
                swap_size = -1

            # get time
            now_time = time.perf_counter()
            timestamp = round(now_time - start_time, 2)

            # Get process details
            with process.oneshot():
                threads = process.threads()  # Get list of threads
                thread_ids = [t.id for t in threads]
                print(
                    f"PID: {process.pid} | TID: {thread_ids} | MEM%: {process.memory_percent():.2f}% | RSS: {human_readable_size(process.memory_info().rss // 1024)} | Swap: {human_readable_size(swap_size)} | Command: {process.cmdline()[0]} | Passed time: {timestamp}")

            # log data
            data.append((process.memory_info().rss // 1024, int(float(swap_size)), timestamp))

            time.sleep(0.05)
    except psutil.NoSuchProcess:
        print("Process not found.")
        data.append((0, 0, data[-1][2] + 1))
    except psutil.AccessDenied:
        print("Access denied to process information.")
    except Exception as ex:
        print(f"Exception occurred, terminating monitoring: {str(ex)}")
    return command


def get_PID(command: str) -> str|None:
    pid = None
    for proc in psutil.process_iter(attrs=['pid', 'cmdline']):
        if command in proc.info['cmdline']:
            pid = proc.info['pid']
            break
    return pid

if __name__ == '__main__':
    # Get the PID of the process running './fib'
    command_to_monitor = './fib'
    pid_to_monitor = get_PID(command_to_monitor)
    if pid_to_monitor is None:
        print(f"Process '{command_to_monitor}' not found, looping until found")

    while pid_to_monitor is None:
        pid_to_monitor = get_PID(command_to_monitor)

    print(f"Process '{command_to_monitor}' found, monitoring is starting")
    command_ret = monitor_process(pid_to_monitor)
    calculate_plot(command_ret)

