# Memory Benchmark

The `memory_benchmark` program is a multithreaded C application designed to measure the READ, WRITE, and CAS (Compare-And-Swap) throughput of memory. It calculates how many millions of operations can be performed per second. The program allows for setting the number of threads and runtime via command-line parameters, and each thread is bound to a distinct CPU core. Sequential and random accesses can be tested separately.

## Requirements

- GCC (GNU Compiler Collection)
- POSIX-compliant operating system (e.g., Linux)
- POSIX threads library

## Compilation

To compile the program, use the following command:

```sh
gcc -o memory_benchmark memory_benchmark.c -lpthread
```

## Usage

The program can be run with the following parameters:

```sh
./memory_benchmark <num_threads> <runtime> <operation> <access_pattern> <array_size> <seed>
```

### Parameters

- `num_threads`: Number of threads to use for the measurement.
- `runtime`: Program runtime in seconds.
- `operation`: Type of operation to measure (`READ`, `WRITE`, `CAS`).
- `access_pattern`: Access pattern (`SEQUENTIAL`, `RANDOM`).
- `array_size`: Size of the array to be accessed (number of elements).
- `seed`: Seed for random number generation.

### Example

To run the program with 4 threads for 10 seconds, measuring READ throughput with sequential access on an array of 1,000,000 elements, and using the seed 42 for random number generation, use the following command:

```sh
./memory_benchmark 4 10 READ SEQUENTIAL 1000000 42
```

### Output

The program will output the total number of operations performed and the throughput in millions of operations per second.

```sh
Total operations: <total_operations>
Throughput: <throughput> million operations per second
```

