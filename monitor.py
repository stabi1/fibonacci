#!/usr/bin/env python3
import argparse
import os
import sys
from typing import List, Tuple
import matplotlib.pyplot as plt

import psutil
import time
import subprocess

data: List[Tuple[int, int, float]] = []


def calculate_plot(command: str, output_filename: str, show_plot: bool):
    # Extracting the components from data
    memory_size = [item[0] for item in data]
    swap_size = [item[1] for item in data]
    timestamps = [item[2] for item in data]

    summary_string = f"Max memory: {human_readable_size(max(memory_size))}; Max swap: {human_readable_size(max(swap_size))}"
    print(summary_string)

    # Convert timestamps to a more readable format if needed (seconds -> hours:minutes:seconds)
    readable_time = [timestamp for timestamp in timestamps]  # Use as-is or convert if needed

    plt.figure(figsize=(10, 6))

    # Plot memory size
    plt.plot(readable_time, memory_size, label="Memory Size (KB)", color="blue", marker="o")
    # Plot swap size
    plt.plot(readable_time, swap_size, label="Swap Size (KB)", color="red", marker="x")

    plt.title("Memory and Swap Size Over Time")
    plt.suptitle(f"Command: {' '.join(command)}\nSummary: {summary_string}")
    plt.xlabel("Timestamp (seconds)")
    plt.ylabel("Size (KB)")
    plt.legend()
    plt.grid(True)

    plt.tight_layout()
    plt.savefig(output_filename)
    if show_plot:
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


def monitor_process(pid, monitor_interval: float) -> str:
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
                thread_ids = [t.id for t in threads]  # List of all thread Ids
                print(
                    f"PID: {process.pid} | NUM Threads: {len(thread_ids)} | MEM%: {process.memory_percent():.2f}% | RSS: {human_readable_size(process.memory_info().rss // 1024)} | Swap: {human_readable_size(swap_size)} | Command: {process.cmdline()[0]} | Passed time: {timestamp}")

            # log data
            data.append((process.memory_info().rss // 1024, int(float(swap_size)), timestamp))

            time.sleep(monitor_interval)
    except psutil.NoSuchProcess:
        print("Process not found.")
        data.append((0, 0, data[-1][2] + monitor_interval * 2))
    except psutil.AccessDenied:
        print("Access denied to process information.")
    except Exception as ex:
        print(f"Exception occurred, terminating monitoring: {str(ex)}")
    return command


def get_PID(command: str) -> str | None:
    pid = None
    for proc in psutil.process_iter(attrs=['pid', 'cmdline']):
        if command in proc.info['cmdline']:
            pid = proc.info['pid']
            break
    return pid


def execute_monitoring(monitor_interval: float, chart_filename: str, show_plot) -> None:
    # Get the PID of the process running './fib'
    command_to_monitor = './fib'
    pid_to_monitor = get_PID(command_to_monitor)
    if pid_to_monitor is None:
        print(f"Process '{command_to_monitor}' not found, looping until found")

    while pid_to_monitor is None:
        pid_to_monitor = get_PID(command_to_monitor)

    print(f"Process '{command_to_monitor}' found, monitoring is starting")
    command_ret = monitor_process(pid_to_monitor, monitor_interval)
    calculate_plot(command_ret, chart_filename, show_plot)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Monitor process")
    # Add arguments
    parser.add_argument('--monitor-interval', type=float, default=0.1, help='The time between measurements in seconds', dest='monitor_interval')
    parser.add_argument('--chart-filename', default='memory_usage_plot.png', type=str, help='Name of the output chart, must be a .png file', dest='chart_filename')
    parser.add_argument('--show-plot', action='store_true', help='If set show the created plot', dest='show_plot')

    args = parser.parse_args()
    if not args.chart_filename.endswith(".png"):
        print("--chart_filename must be a .png file")
        sys.exit(1)

    execute_monitoring(args.monitor_interval, args.chart_filename, args.show_plot)
    sys.exit(0)
