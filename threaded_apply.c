#include "threaded_apply.h"

#include <stdlib.h>
#include <string.h>

#include "thpool.h"

typedef struct {
    int* input;
    int output;
    int begin;
    int end;
    BinaryFunction function;
} ApplyArgs;

static void* apply_serial(void* arg_v) {
    ApplyArgs* arg = (ApplyArgs*)arg_v;
    arg->output = arg->input[arg->begin];
    if (arg->begin + 1 >= arg->end) return NULL;
    for (int i = arg->begin + 1; i < arg->end; i++) {
        arg->output = arg->function(arg->output, arg->input[i]);
    }
    return NULL;
}

/**
 * @brief Computes the `apply` function threadedly, DOES NOT do the final
 * combining of results
 *
 * This function is called internally, as it only does simple (1 layer) chunking
 */
#define MIN(x, y) ((x) < (y) ? (x) : (y))
static int* apply_threaded_internal(ThreadPool* thpool, BinaryFunction function,
                                    int* input, int size, int chunk_size,
                                    int* num_chunks) {
    if (size % chunk_size == 0)
        *num_chunks = size / chunk_size;
    else
        *num_chunks = size / chunk_size + 1;
    ApplyArgs* tasks = (ApplyArgs*)malloc(*num_chunks * sizeof(ApplyArgs));
    int* output = (int*)malloc(*num_chunks * sizeof(int));
    for (int i = 0; i < *num_chunks; i++) {
        tasks[i].begin = i * chunk_size;
        tasks[i].end = MIN((i + 1) * chunk_size, size);
        tasks[i].function = function;
        tasks[i].input = input;
        tasks[i].output = 0;

        ThreadPool_Task* thpool_task =
            (ThreadPool_Task*)malloc(sizeof(ThreadPool_Task));
        thpool_task->function = apply_serial;
        thpool_task->arg = &tasks[i];
        ThreadPool_AddTask(thpool, thpool_task);
    }
    ThreadPool_Wait(thpool);
    for (int i = 0; i < *num_chunks; i++) output[i] = tasks[i].output;
    free(tasks);
    return output;
}

int apply_threaded(BinaryFunction function, int* input, int size) {
    if (size < 50) {
        ApplyArgs args = {.input = input,
                          .output = 0,
                          .function = function,
                          .begin = 0,
                          .end = size};
        apply_serial(&args);
        return args.output;
    }
    ThreadPool* thpool = ThreadPool_Init(THREAD_NUM);
    int *array, *array_temp;
    array = (int*)malloc(size * sizeof(int));
    memcpy(array, input, size * sizeof(int));
    while (size >= THREAD_NUM) {
        array_temp =
            apply_threaded_internal(thpool, function, array, size, 8, &size);
        free(array);
        array = array_temp;
    }
    int ans = array[0];
    for (int i = 1; i < size; i++) ans = function(ans, array[i]);
    ThreadPool_Destroy(thpool);
    return ans;
}