#include "threaded_apply.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>

#include "thpool.h"

typedef struct Task {
    int* input;
    int output;
    // If length is smaller than this threshold, we compute serially
    size_t serial_threshold;
    size_t start_idx, end_idx;
    BinaryFunction function;
    ThreadPool* pool;
    int computed;
    pthread_cond_t task_cond;
    pthread_mutex_t task_mutex;
} Task;

/**
 * @brief Serially computes apply on small segments
 *
 */
static void* serial(Task* task) {
    int result = task->input[task->start_idx];
    for (size_t i = task->start_idx + 1; i <= task->end_idx; i++) {
        result = task->function(result, task->input[i]);
    }
    task->output = result;
    return NULL;
}

/**
 * @brief Performs binary divide-and-conquer merging algorithm
 *
 * @todo: This asks for (theoretically) infinite number of threads, which is not
 *        desirable. Can we modify it to use a finite number of threads?
 */
static void* merge(void* task_v) {
    Task* task = (Task*)task_v;
    size_t start = task->start_idx, end = task->end_idx;
    fprintf(stderr, "Apply %p: Received task %ld, %ld\n", task, start, end);
    if (start == end) {
        // no need to merge, return straightaway
        fprintf(stderr, "Apply %p: Empty range, falling back\n", task);
        fprintf(stderr, "Apply %p: Job complete (empty)\n", task);
        task->output = task->input[start];
        pthread_mutex_lock(&task->task_mutex);
        task->computed = 1;
        pthread_cond_signal(&task->task_cond);
        pthread_mutex_unlock(&task->task_mutex);
        return NULL;
    }
    assert(end >= start);
    if (end - start + 1 <= task->serial_threshold) {
        // use serial computation instead
        fprintf(stderr, "Apply %p: Falling back to serial computation\n", task);
        fprintf(stderr, "Apply %p: Job complete (serial)\n", task);
        serial(task);
        pthread_mutex_lock(&task->task_mutex);
        task->computed = 1;
        pthread_cond_signal(&task->task_cond);
        pthread_mutex_unlock(&task->task_mutex);
        return NULL;
    }
    size_t mid = (start + end) / 2;
    Task left_subtask = {.input = task->input,
                         .output = 0,
                         .function = task->function,
                         .start_idx = task->start_idx,
                         .end_idx = mid,
                         .serial_threshold = task->serial_threshold,
                         .pool = task->pool,
                         .computed = 0};

    pthread_mutex_init(&left_subtask.task_mutex, NULL);
    pthread_cond_init(&left_subtask.task_cond, NULL);

    Task right_subtask = {.input = task->input,
                          .output = 0,
                          .function = task->function,
                          .start_idx = mid + 1,
                          .end_idx = task->end_idx,
                          .serial_threshold = task->serial_threshold,
                          .pool = task->pool,
                          .computed = 0};

    pthread_mutex_init(&right_subtask.task_mutex, NULL);
    pthread_cond_init(&right_subtask.task_cond, NULL);

    ThreadPool_Task left_subtask_thread = {.arg = &left_subtask,
                                           .function = merge};
    ThreadPool_Task right_subtask_thread = {.arg = &right_subtask,
                                            .function = merge};
    fprintf(stderr,
            "Apply %p: Submitted task %lu, %lu; %lu, %lu; addr is %p, %p\n",
            task, left_subtask.start_idx, left_subtask.end_idx,
            right_subtask.start_idx, right_subtask.end_idx,
            &left_subtask_thread, &right_subtask_thread);
    fprintf(stderr, "Apply %p: waiting left\n", task);
    // Wait for the left subtask to complete
    pthread_mutex_lock(&left_subtask.task_mutex);
    ThreadPool_AddTask(task->pool, &left_subtask_thread);
    while (!left_subtask.computed)
        pthread_cond_wait(&left_subtask.task_cond, &left_subtask.task_mutex);
    pthread_mutex_unlock(&left_subtask.task_mutex);
    fprintf(stderr, "Apply %p: received left, waiting right\n", task);
    // Wait for the right subtask to complete
    pthread_mutex_lock(&right_subtask.task_mutex);
    ThreadPool_AddTask(task->pool, &right_subtask_thread);
    while (!right_subtask.computed)
        pthread_cond_wait(&right_subtask.task_cond, &right_subtask.task_mutex);
    pthread_mutex_unlock(&right_subtask.task_mutex);
    fprintf(stderr, "Apply %p: received right\n", task);
    task->output = task->function(left_subtask.output, right_subtask.output);
    pthread_mutex_lock(&task->task_mutex);
    task->computed = 1;
    pthread_cond_signal(&task->task_cond);
    pthread_mutex_unlock(&task->task_mutex);
    fprintf(stderr, "Apply %p: Job complete\n", task);
    return NULL;
}

int apply_threaded(BinaryFunction function, int* input, size_t size) {
    Task task = {.input = input,
                 .start_idx = 0,
                 .end_idx = size - 1,
                 .serial_threshold = 10,  // Modify this for best result
                 .function = function,
                 .output = 0,
                 .pool = ThreadPool_Init(8),
                 .computed = 0};
    merge(&task);
    pthread_mutex_lock(&task.task_mutex);
    while (!task.computed) {
        pthread_cond_wait(&task.task_cond, &task.task_mutex);
    }
    pthread_mutex_unlock(&task.task_mutex);

    // Clean up resources
    pthread_mutex_destroy(&task.task_mutex);
    pthread_cond_destroy(&task.task_cond);
    ThreadPool_Destroy(task.pool);
    return task.output;
}