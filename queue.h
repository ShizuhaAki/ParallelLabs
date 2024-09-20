#include <stdbool.h>
#include <stddef.h>

/************** Data structure declarations ****************/

/**
 * @brief Linked list element containing a string.
 *
 * You shouldn't change this struct.
 */
typedef struct list_ele {
    void *value;
    struct list_ele *next;
} list_ele_t;

/**
 * @brief Queue structure representing a list of elements
 */
typedef struct {
    list_ele_t *head;
    list_ele_t *tail;
    size_t size;
} queue_t;

/************** Operations on queue ************************/

/* Create empty queue. */
queue_t *queue_init(void);

/* Free ALL storage used by queue. */
void queue_free(queue_t *q);

/* Attempt to insert element at tail of queue. */
bool queue_insert_tail(queue_t *q, const void *s);

/* Attempt to remove element from head of queue. */
bool queue_remove_head(queue_t *q, void **sp);

/* Return number of elements in queue. */
size_t queue_size(queue_t *q);
