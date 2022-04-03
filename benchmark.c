#include "benchmark.h"

#if defined(_WIN32)
// Windows XP
#define NTDDI_VERSION 0x05010000
#define _WIN32_WINNT 0x0501

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
// Needed for clock_gettime
#define _POSIX_C_SOURCE 199309L
#include <time.h>
#endif

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Maximum number of iterations to run a benchmark
#define MAX_ITERS 1000000000

// Minimum time (ns) to run a benchmark
#define MIN_TIME 500000000

// State passed into function to benchmark
struct benchmark_state {
  char *name;
  uint64_t iters;
  uint64_t index;
  int64_t *ns;
};

// Get timestamp in nanoseconds
int64_t get_ns(void) {
#if defined(_WIN32)
  LARGE_INTEGER frequency;
  LARGE_INTEGER time;

  // QueryPerformanceFrequency and QueryPerformanceCounter always succeed on
  // Windows XP and later
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&time);

  return (int64_t)time.QuadPart * 1000000000 / frequency.QuadPart;
#else
  struct timespec ts;
  int ret;

  // Fall back to CLOCK_REALTIME if CLOCK_MONOTONIC is not supported
#if defined(CLOCK_MONOTONIC)
  if ((ret = clock_gettime(CLOCK_MONOTONIC, &ts)))
#endif
    ret = clock_gettime(CLOCK_REALTIME, &ts);

  if (ret) {
    perror("clock_gettime");
    exit(EXIT_FAILURE);
  }

  return (int64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
#endif
}

// Save the timestamp for the current iteration
bool benchmark_keep_running(struct benchmark_state *state) {
  state->ns[state->index] = get_ns();
  return state->index++ < state->iters;
}

// Determine if a run should be reported
static bool should_report(const struct benchmark_state *state) {
  return state->iters >= MAX_ITERS ||                        // Enough iterations
         state->ns[state->iters] - state->ns[0] >= MIN_TIME; // Enough time
}

static void report(struct benchmark_state *state) {
  int64_t total_ns = state->ns[state->iters] - state->ns[0];
  for (int i = 0; i < state->iters; i++) {
    state->ns[i] = state->ns[i + 1] - state->ns[i];
  }
  printf("%s: %" PRIu64 " iterations, avg %f ns, total %f s\n", state->name, state->iters, (double)total_ns / state->iters, total_ns / 1e9);
}

static void update(struct benchmark_state *state) {
  double multiplier = 0;
  int64_t total_ns = state->ns[state->iters] - state->ns[0];

  // Total time was at least 10% of the minimum time
  if (total_ns * 10 > MIN_TIME) {
    // Find the number of iterations needed to run for the minimum time
    multiplier = (double)MIN_TIME / total_ns;
  } else {
    // Increase iterations by 10 times
    multiplier = 10.0;
  }

  // Limit iterations to the maximum number of iterations
  uint64_t iters = state->iters * multiplier;
  if (iters > MAX_ITERS) {
    iters = MAX_ITERS;
  }

  // Increase the size of the timing array
  free(state->ns);
  // n + 1 timestamps for n iterations
  state->ns = malloc(sizeof(*state->ns) * (iters + 1));
  if (state->ns == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  // Reset the state
  state->iters = iters;
  state->index = 0;
}

int benchmark_main(const benchmark_func funcs[], char names[]) {
  char *name = NULL;
  size_t i = 0;
  struct benchmark_state *state = malloc(sizeof(*state));
  if (state == NULL) {
    perror("malloc");
    return EXIT_FAILURE;
  }

  for (name = strtok(names, ", "); name != NULL; name = strtok(NULL, ", "), i++) {
    state->name = name;
    state->iters = 1;
    state->index = 0;

    state->ns = malloc(sizeof(*state->ns) * 2);
    if (state->ns == NULL) {
      perror("malloc");
      return EXIT_FAILURE;
    }

    // Keep running until results should be reported
    while (true) {
      funcs[i](state);

      if (should_report(state)) {
        break;
      }

      update(state);
    }

    report(state);
    free(state->ns);
  }

  free(state);
  return 0;
}
