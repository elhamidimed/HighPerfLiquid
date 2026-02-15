# Benchmark Results

This folder stores benchmark runs for reproducibility and regression tracking.

## How to run

Build with benchmarks enabled:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DHPL_ENABLE_BENCHMARKS=ON
cmake --build build