
#include "queue.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief Allocates a new queue
 * @return The new queue, or NULL if memory allocation failed
 */
queue_t *queue_init(void) {
    queue_t *q = malloc(sizeof(queue_t));
    if (q == NULL) {
        return NULL;
    }
    q->head = NULL;
    q->size = 0;
    return q;
}
/**
 * @brief Frees all memory used by a queue
 * @param[in] q The queue to free
 */
void queue_free(queue_t *q) {
    if (q != NULL) {
        list_ele_t *cur = q->head;
        list_ele_t *tmp;
        while (cur != NULL) {
            tmp = cur;
            cur = cur->next;
            free(tmp->value);
            free(tmp);
        }
    }
    free(q);
    q = NULL;
}

/**
 * @brief Attempts to insert an element at tail of a queue
 *
 * @param[in] q The queue to insert into
 * @param[in] s Value to be copied and inserted into the queue
 *
 * @return true if insertion was successful
 * @return false if q is NULL, or memory allocation failed
 */
bool queue_insert_tail(queue_t *q, const void *s) {
    if (q == NULL) {
        return false;
    }
    list_ele_t *newt;
    newt = malloc(sizeof(list_ele_t));
    if (newt == NULL) {
        return false;
    }
    newt->value = s;
    newt->next = NULL;
    if (q->head == NULL) {
        q->head = q->tail = newt;
    } else {
        q->tail->next = newt;
        q->tail = newt;
    }
    q->size++;
    return true;
}

/**
 * @brief Attempts to remove an element from head of a queue
 *
 * @param[in]  q       The queue to remove from
 * @param[out] buf     Output buffer to write the head value into
 *
 * @returns true if removal succeeded,
 * @returns false if q is NULL or empty
 */
bool queue_remove_head(queue_t *q, void **buf) {
    if (q == NULL || q->head == NULL) {
        return false;
    }
    *buf = q->head->value;
    list_ele_t *head_to_free = q->head;
    q->head = q->head->next;
    free(head_to_free);
    q->size--;
    return true;
}

/**
 * @brief Returns the number of elements in a queue
 *
 * This function runs in O(1) time.
 *
 * @param[in] q The queue to examine
 *
 * @return the number of elements in the queue, or
 *         0 if q is NULL or empty
 */
size_t queue_size(queue_t *q) {
    if (q == NULL) {
        return 0;
    }
    return q->size;
}