#!/usr/bin/env python3
import os
import psutil
import time
import subprocess


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
    try:
        process = psutil.Process(pid)
        while True:
            # Get the size of the swap_storage folder
            swap_size = get_swap_storage_size()

            # Get process details
            with process.oneshot():
                threads = process.threads()  # Get list of threads
                thread_ids = [t.id for t in threads]
                print(
                    f"PID: {process.pid} | TID: {thread_ids} | MEM%: {process.memory_percent():.2f}% | RSS: {process.memory_info().rss // 1024} KB | Swap: {swap_size} | Command: {process.cmdline()[0]}")

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
    else:
        print("Process './fib' not found.")
