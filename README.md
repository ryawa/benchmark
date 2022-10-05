# Tool for benchmarking code execution time

### Summary

This is a library written in C that can be used to measure the execution time of pieces of C code which run in the nanosecond range. The program outputs how many iterations were run, the average run time for the piece of code, and the total time taken to run the benchmark.

Video demonstration: https://youtu.be/DWEnA68SNE4

## Motivation

My motivation for this project was CS50's speller problem, where the goal is to create a spell-checking program that runs as fast as possible. Students can compete for the quickest run time on the Big Board, which times how long the program takes. However, the Big Board only shows the run times for the functions in the program and it is very slow compared to running the program locally, and it is hard to know the exact time it takes for small snippets of code to run. Therefore, I created this library to measure the execution time of pieces of code. Using this library, it is possible to analyze the individual parts of the functions to know which pieces of code run slowly and need to be improved. This tool helped me to go from roughly rank #300 out of 5000 students who submitted to the Big Board to [rank #6](https://speller.cs50.net/cs50/problems/2020/x/challenges/speller#user/ryawa) after a few months of optimizing. It is also my final project for the course.

## Library structure

`benchmark.h` - header file which contains funciton prototypes and `BENCHMARK()` macro

`benchmark.c` - source file which contains the implementation of the benchmarking algorithm

## Algorithm

The most important part of the algorithm is finding the optimal number of iterations to run so that it does not take too much time and provides an accurate measurement of the average run time of the code to benchmark. Because there are many factors such as cache misses and CPU load which can influence the run time of a benchmark, it is necessary to run the benchmark for many iterations. First, the library runs one iteration of a benchmark to determine the number of iterations to run. Then, it keeps multiplying the number of iterations by 10 until the run time is at least 10% of the minimum time (0.5 seconds is used). Then, it multiplies the number of iterations by the minimum time divided by the run time. If enough iterations (10⁹ is used) are run or enough time passes (0.5 seconds), then the library reports the average run time.

## Example

Benchmark of a hash function:
```c
// Include the library
#include "benchmark.h"
#include <stdint.h>

// Function to benchmark
void hash(benchmark_state *state)
{
  // Code to benchmark is in the loop
  while (benchmark_keep_running(state)) {
    // FNV-1a hash function (http://www.isthe.com/chongo/tech/comp/fnv/)
    volatile uint64_t h = 14695981039346656037UL;
    volatile char word[] = "test";
    for (int i = 0; word[i] != '\0'; i++)
    {
        h = (h ^ (word[i] | 0x20)) * 1099511628211UL;
    }
  }
}

// Run the benchmark
BENCHMARK(hash);
```
Output:
```
hash: 3705080 iterations, avg 137.045371 ns, total 0.507764 s
```

## Further Development
* Print more statistics like standard deviation, etc.
* Allow passing arguments into benchmarks
* Provide macros for preventing optimization
