#!/usr/bin/env python3
"""Benchmark Slim, Vim, and Neovim startup + quit times."""
import subprocess
import time
import os
import tempfile
import statistics

TEST_FILE = "/tmp/slim_bench.txt"
LINES = 1000

def make_test_file():
    with open(TEST_FILE, "w") as f:
        for i in range(LINES):
            f.write(f"Line {i + 1}: the quick brown fox jumps over the lazy dog\n")

def run(cmd, input_bytes=b"", rounds=10):
    times = []
    for _ in range(rounds):
        start = time.perf_counter()
        proc = subprocess.Popen(
            cmd,
            stdin=subprocess.PIPE,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        proc.communicate(input=input_bytes)
        elapsed = time.perf_counter() - start
        times.append(elapsed)
    return {
        "min": min(times),
        "max": max(times),
        "avg": statistics.mean(times),
        "median": statistics.median(times),
    }

def main():
    make_test_file()
    os.chdir(os.path.dirname(os.path.abspath(__file__)))

    # Ensure Slim binary exists
    if not os.path.exists("./slim"):
        print("Building Slim first...")
        subprocess.run(["./build.sh"], check=True)

    print("=" * 60)
    print(f"Startup + quit benchmark ({LINES}-line file, 10 rounds each)")
    print("=" * 60)

    results = []

    # Slim: open and immediately Ctrl+Q
    results.append(("Slim",    ["./slim", TEST_FILE], b"\x11"))

    # Vim: minimal config, open and quit
    results.append(("Vim",     ["vim", "-u", "NONE", "-es", "-c", "q", TEST_FILE], b""))

    # Neovim: minimal config, open and quit
    results.append(("Neovim",  ["nvim", "-u", "NONE", "--headless", "-c", "q", TEST_FILE], b""))

    for name, cmd, inp in results:
        try:
            res = run(cmd, inp)
            print(f"{name:10} avg={res['avg']*1000:7.3f} ms  median={res['median']*1000:7.3f} ms  min={res['min']*1000:7.3f} ms  max={res['max']*1000:7.3f} ms")
        except Exception as e:
            print(f"{name:10} FAILED: {e}")

    print("=" * 60)

if __name__ == "__main__":
    main()
