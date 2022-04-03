#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdbool.h>

typedef struct benchmark_state benchmark_state;
typedef void (*benchmark_func)(benchmark_state *);

bool benchmark_keep_running(benchmark_state *state);
int benchmark_main(const benchmark_func funcs[], char names[]);

// Pass names and functions to benchmark_main
#define BENCHMARK(...)                                                         \
  int main(void) {                                                             \
    return benchmark_main((const benchmark_func[]){__VA_ARGS__},               \
                          (char[]){#__VA_ARGS__});                             \
  }                                                                            \
  int main(void)

#endif
