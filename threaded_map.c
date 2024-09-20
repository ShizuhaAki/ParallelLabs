#include "threaded_map.h"

#include <pthread.h>

typedef struct ThreadArg {
    int *input, *output;
    size_t start_idx, end_idx;
    Function function;
} ThreadArg;

static void* worker(void* argument) {
    ThreadArg* arg = (ThreadArg*)argument;
    for (size_t i = arg->start_idx; i < arg->end_idx; i++) {
        arg->output[i] = arg->function(arg->input[i]);
    }
    return NULL;
}

void map_threaded(Function function, int* input, int* output, size_t size) {
    size_t chunk = size / THREAD_NUM;
    ThreadArg thread_arg[THREAD_NUM];
    pthread_t threads[THREAD_NUM];
    for (size_t i = 0; i < THREAD_NUM; i++) {
        thread_arg[i].input = input;
        thread_arg[i].output = output;
        thread_arg[i].function = function;
        thread_arg[i].start_idx = chunk * i;
        thread_arg[i].end_idx = (i == THREAD_NUM - 1) ? size : (i + 1) * chunk;

        pthread_create(&threads[i], NULL, worker, &thread_arg[i]);
    }
    for (size_t i = 0; i < THREAD_NUM; i++) {
        pthread_join(threads[i], NULL);
    }
}