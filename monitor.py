#!/usr/bin/env python3
import os
from typing import List, Tuple
import matplotlib.pyplot as plt

import psutil
import time
import subprocess

data: List[Tuple[int, int, float]] = []

def calculate_plot():
    # Extracting the components from data
    memory_size = [item[0] for item in data]
    swap_size = [item[1] for item in data]
    timestamps = [item[2] for item in data]

    # Convert timestamps to a more readable format if needed (seconds -> hours:minutes:seconds)
    readable_time = [timestamp for timestamp in timestamps]  # Use as-is or convert if needed

    plt.figure(figsize=(10, 6))

    # Plot memory size
    plt.plot(readable_time, memory_size, label="Memory Size (KB)", color="blue", marker="o")
    # Plot swap size
    plt.plot(readable_time, swap_size, label="Swap Size (KB)", color="red", marker="x")

    plt.title("Memory and Swap Size Over Time")
    plt.xlabel("Timestamp (seconds)")
    plt.ylabel("Size (KB)")
    plt.legend()
    plt.grid(True)

    plt.tight_layout()
    plt.savefig('memory_usage_plot.png')
    plt.show()

def get_swap_storage_size():
    script_dir = os.path.dirname(os.path.realpath(__file__))
    swap_storage_path = os.path.join(script_dir, 'swap_storage')

    # Get the size of the folder using du command
    try:
        result = subprocess.run(['du', '-sh', swap_storage_path], stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                text=True)
        if result.returncode == 0:
            return result.stdout.split()[0]  # Return the size from the du output
        else:
            return "Error retrieving size"
    except Exception as e:
        return f"Error: {str(e)}"


def monitor_process(pid):
    start_time = time.perf_counter()
    try:
        process = psutil.Process(pid)
        while True:
            # Get the size of the swap_storage folder
            swap_size = get_swap_storage_size()

            # get time
            now_time = time.perf_counter()
            timestamp = round(now_time - start_time, 2)

            # Get process details
            with process.oneshot():
                threads = process.threads()  # Get list of threads
                thread_ids = [t.id for t in threads]
                print(
                    f"PID: {process.pid} | TID: {thread_ids} | MEM%: {process.memory_percent():.2f}% | RSS: {process.memory_info().rss // 1024} KB | Swap: {swap_size} | Command: {process.cmdline()[0]} | Passed time: {timestamp}")

            # log data
            data.append((process.memory_info().rss  // 1024, int(float(swap_size.removesuffix("K"))), timestamp))

            time.sleep(1)
    except psutil.NoSuchProcess:
        print("Process not found.")
    except psutil.AccessDenied:
        print("Access denied to process information.")


if __name__ == '__main__':
    # Get the PID of the process running './fib'
    pid_to_monitor = None
    for proc in psutil.process_iter(attrs=['pid', 'cmdline']):
        if './fib' in proc.info['cmdline']:
            pid_to_monitor = proc.info['pid']
            break

    if pid_to_monitor:
        monitor_process(pid_to_monitor)
        calculate_plot()
    else:
        print("Process './fib' not found.")
