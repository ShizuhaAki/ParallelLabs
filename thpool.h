#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <pthread.h>

#include "def.h"
#include "queue.h"

#define TASKQUEUE_MAXSIZE 100000000

typedef void *(*ThreadPool_Function)(void *);

typedef struct {
    ThreadPool_Function function;
    void *arg;
} ThreadPool_Task;

typedef struct {
    // This is set to true in the destroy function
    int destroyed;
    int num_threads;
    int num_pending_tasks;
    queue_t *task_queue;
    pthread_t *threads;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_cond_t all_tasks_done;
} ThreadPool;

/**
 * @brief Creates a Thread Pool
 *
 * This function creates the necessary data structure for a thread pool,
 * and spawns a main thread that maintains the pool
 *
 * @param num_threads The number of threads that this thread pool may use
 */
ThreadPool *ThreadPool_Init(int num_threads);

/**
 * @brief Adds a task to the thread pool
 *
 * This function should be the only way to create tasks, do not use
 * pthread_create manually, as this won't be managed by the thread pool
 *
 * @param[in] pool the thread pool object
 * @param[in] task the task itself
 * @returns 0 if successful, -1 if not
 */
int ThreadPool_AddTask(ThreadPool *pool, ThreadPool_Task *task);

/**
 * @brief Destroys the thread pool
 */
void ThreadPool_Destroy(ThreadPool *pool);

int ThreadPool_GetNumThreads(ThreadPool *pool);

void ThreadPool_Wait(ThreadPool *pool);
#endif