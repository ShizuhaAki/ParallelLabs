#include "thpool.h"

#include <stdio.h>
#include <stdlib.h>

static void* threadPool_Manager(void* arg_v) {
    ThreadPool* pool = (ThreadPool*)arg_v;

    while (1) {
        // Before Execute
        pthread_mutex_lock(&pool->mutex);
        while (queue_size(pool->task_queue) == 0 && !pool->destroyed) {
            // No tasks are pending, wait
            //    fprintf(stderr, "Thread Pool: Queue empty, waiting for
            //    tasks\n");
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }
        if (pool->destroyed) {
            //    fprintf(stderr, "Thread Pool: Self Destroy\n");
            pthread_mutex_unlock(&pool->mutex);
            pthread_exit(NULL);
        }
        void* current_task_v;
        //   fprintf(stderr, "Thread Pool: Found pending task, dequeueing\n");
        queue_remove_head(pool->task_queue, &current_task_v);
        ThreadPool_Task* current_task = (ThreadPool_Task*)current_task_v;
        pthread_mutex_unlock(&pool->mutex);
        //   fprintf(stderr, "Thread Pool: Dispatching task: %p\n",
        //           current_task->arg);
        // Execute
        current_task->function(current_task->arg);
        // After Execute
        pthread_mutex_lock(&pool->mutex);
        pool->num_pending_tasks--;  // Decrement the pending tasks counter
        if (pool->num_pending_tasks == 0) {
            pthread_cond_signal(
                &pool->all_tasks_done);  // Signal when all tasks are done
        }
        pthread_mutex_unlock(&pool->mutex);
    }

    return NULL;
}

ThreadPool* ThreadPool_Init(int num_threads) {
    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    if (pool == NULL) {
        return NULL;
    }
    pool->num_threads = num_threads;
    pool->num_pending_tasks = 0;
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->cond, NULL);
    pthread_cond_init(&pool->all_tasks_done, NULL);
    pool->task_queue = queue_init();
    if (pool->task_queue == NULL) {
        free(pool);
        return NULL;
    }
    pool->destroyed = 0;
    pool->threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&pool->threads[i], NULL, threadPool_Manager,
                           (void*)pool) != 0) {
            queue_free(pool->task_queue);
            free(pool);
            return NULL;
        }
    }
    //  fprintf(stderr, "Thread Pool: Created successfully\n");
    return pool;
}

int ThreadPool_AddTask(ThreadPool* pool, ThreadPool_Task* task) {
    // Locks the pool to avoid race condition
    pthread_mutex_lock(&pool->mutex);
    // The following code BLOCKS when the task queue is full
    while (queue_size(pool->task_queue) == TASKQUEUE_MAXSIZE) {
        //      fprintf(stderr,
        //              "Thread Pool: Task Queue full, waiting for free
        //              worker\n");
        pthread_cond_wait(&pool->cond, &pool->mutex);
    }
    // The following code DOES NOT block when task queue is full
    /*
        if (queue_size(pool->task_queue) == TASKQUEUE_MAXSIZE) {
            pthread_mutex_unlock(&pool->mutex);
            return -1;
        }
    */
    //  fprintf(stderr, "Thread Pool: Still room in task queue; enqueueing
    //  tesk\n");
    int enqueue_status = queue_insert_tail(pool->task_queue, task);
    if (enqueue_status == 0) {
        fprintf(stderr, "error: Thread Pool: Task enqueue failed\n");
        pthread_mutex_unlock(&pool->mutex);
        return -1;
    }
    pool->num_pending_tasks++;
    pthread_cond_signal(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);
    //  fprintf(stderr, "Thread Pool: Task submitted successfully\n");
    return 0;
}

void ThreadPool_Destroy(ThreadPool* pool) {
    pthread_mutex_lock(&pool->mutex);
    pool->destroyed = 1;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);

    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);
    queue_free(pool->task_queue);
    free(pool);
}

int ThreadPool_GetNumThreads(ThreadPool* pool) { return pool->num_threads; }
void ThreadPool_Wait(ThreadPool* pool) {
    pthread_mutex_lock(&pool->mutex);
    while (pool->num_pending_tasks > 0) {
        pthread_cond_wait(&pool->all_tasks_done,
                          &pool->mutex);  // Wait until all tasks are done
    }
    pthread_mutex_unlock(&pool->mutex);
}