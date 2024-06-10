#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define CACHE_LINE_SIZE 64

typedef enum {
  READ,
  WRITE,
  CAS
} operation_t;

typedef enum {
  SEQUENTIAL,
  RANDOM
} access_pattern_t;

typedef struct {
  int thread_id;
  int num_threads;
  int runtime;
  operation_t operation;
  access_pattern_t access_pattern;
  uint64_t *array;
  size_t array_size;
  uint64_t operations;
  unsigned int seed;
} thread_arg_t;

void pin_thread_to_core(int core_id) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

void *thread_func(void *arg) {
  thread_arg_t *targ = (thread_arg_t *)arg;
  pin_thread_to_core(targ->thread_id);

  struct timeval start, end;
  gettimeofday(&start, NULL);
  uint64_t ops = 0;

  while (1) {
    gettimeofday(&end, NULL);
    if ((end.tv_sec - start.tv_sec) >= targ->runtime) {
      break;
    }

    size_t index = (targ->access_pattern == RANDOM) ? rand_r(&targ->seed) % targ->array_size : (ops % targ->array_size);

    switch (targ->operation) {
      case READ:
        __atomic_load_n(&targ->array[index], __ATOMIC_RELAXED);
        break;
      case WRITE:
        __atomic_store_n(&targ->array[index], ops, __ATOMIC_RELAXED);
        break;
      case CAS: {
        uint64_t expected = targ->array[index];
        __atomic_compare_exchange_n(&targ->array[index], &expected, ops, 0, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
      } break;
    }
    ops++;
  }

  targ->operations = ops;
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc != 7) {
    fprintf(stderr, "Usage: %s <num_threads> <runtime> <operation> <access_pattern> <array_size> <seed>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int num_threads = atoi(argv[1]);
  int runtime = atoi(argv[2]);
  operation_t operation = (strcmp(argv[3], "READ") == 0) ? READ : (strcmp(argv[3], "WRITE") == 0) ? WRITE
                                                              : (strcmp(argv[3], "CAS") == 0)     ? CAS
                                                                                                  : -1;
  access_pattern_t access_pattern = (strcmp(argv[4], "SEQUENTIAL") == 0) ? SEQUENTIAL : (strcmp(argv[4], "RANDOM") == 0) ? RANDOM
                                                                                                                         : -1;
  size_t array_size = atol(argv[5]);
  unsigned int seed = (unsigned int)atoi(argv[6]);

  if (operation == -1 || access_pattern == -1) {
    fprintf(stderr, "Invalid operation or access pattern\n");
    exit(EXIT_FAILURE);
  }

  pthread_t threads[num_threads];
  thread_arg_t thread_args[num_threads];

  uint64_t *array;
  if (posix_memalign((void **)&array, CACHE_LINE_SIZE, array_size * sizeof(uint64_t)) != 0) {
    perror("posix_memalign");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < num_threads; i++) {
    thread_args[i].thread_id = i;
    thread_args[i].num_threads = num_threads;
    thread_args[i].runtime = runtime;
    thread_args[i].operation = operation;
    thread_args[i].access_pattern = access_pattern;
    thread_args[i].array = array;
    thread_args[i].array_size = array_size;
    thread_args[i].operations = 0;
    thread_args[i].seed = seed + i;  // Different seed for each thread
    pthread_create(&threads[i], NULL, thread_func, &thread_args[i]);
  }

  uint64_t total_operations = 0;
  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
    total_operations += thread_args[i].operations;
  }

  printf("Total operations: %lu\n", total_operations);
  printf("Throughput: %.2f million operations per second\n", total_operations / (double)runtime / 1e6);

  free(array);
  return 0;
}
