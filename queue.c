#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

#define element_next(node, member) \
    list_entry(node->member.next, typeof(*node), member)

#define element_prev(node, member) \
    list_entry(node->member.prev, typeof(*node), member)

static inline element_t *create_element(char *s)
{
    element_t *element = malloc(sizeof(element_t));
    if (!element)
        return NULL;
    element->value = strdup(s);
    // If string copy failed, free the element
    if (!element->value) {
        free(element);
        return NULL;
    }
    return element;
}

static inline struct list_head *_q_find_mid(struct list_head *left,
                                            struct list_head *right)
{
    while (left != right) {
        left = left->next;
        // When there are an even number of nodes, select next first
        if (right == left)
            break;
        right = right->prev;
    }

    return left;
}

static inline int _q_value_cmpare(char *a, char *b, bool descend)
{
    return descend ? strcmp(b, a) : strcmp(a, b);
}

void _q_merge(struct list_head *l_head, struct list_head *r_head, bool descend)
{
    element_t *l = list_first_entry(l_head, element_t, list);

    while (!list_empty(r_head)) {
        // Grab next r element
        element_t *r = list_first_entry(r_head, element_t, list);
        // Compare l and r element value,
        // if l element is in the correct position(dont need to swap),
        // skip until the element need to swap
        while (&l->list != l_head &&
               _q_value_cmpare(l->value, r->value, descend) <= 0)
            l = list_entry(l->list.next, element_t, list);
        // If all left list elements are in the correct position,
        // means that the entire list is sorted,
        // can directly append right list to left,
        // Increase sorting speed.
        if (&l->list == l_head) {
            // Append right list to left list
            l_head->prev->next = r_head->next;
            r_head->next->prev = l_head->prev;

            r_head->prev->next = l_head;
            l_head->prev = r_head->prev;

            INIT_LIST_HEAD(r_head);
            break;
        }

        // Insert r node before l node
        list_move_tail(r_head->next, &l->list);
    }
}

void _q_merge_sort(struct list_head *head, bool descend)
{
    if (list_is_singular(head))
        return;

    struct list_head *l = head, *r = &(struct list_head) {},
                     *mid = _q_find_mid(head->next, head->prev),
                     *l_end = mid->prev;

    // Split list from mid
    r->next = mid;
    mid->prev = r;
    r->prev = l->prev;  // Last node from list head
    l->prev->next = r;

    l->prev = l_end;
    l_end->next = l;

    _q_merge_sort(l, descend);
    _q_merge_sort(r, descend);

    _q_merge(l, r, descend);
}

static inline void _q_remove_not_in_order(struct list_head *head, bool descend)
{
    element_t *last = list_first_entry(head, element_t, list), *curr, *tmp;

    list_for_each_entry (curr, head, list) {
        // Remove all element start from end that not in order
        while (&last->list != head &&
               _q_value_cmpare(curr->value, last->value, descend) < 0) {
            tmp = element_prev(last, list);
            list_del(&last->list);
            q_release_element(last);
            last = tmp;
        }
        last = curr;
    }
}

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;

    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *entry, *safe = NULL;
    list_for_each_entry_safe (entry, safe, head, list)
        q_release_element(entry);
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *entry = create_element(s);
    if (!entry)
        return false;

    list_add(&entry->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *entry = create_element(s);
    if (!entry)
        return false;

    list_add_tail(&entry->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *entry = list_first_entry(head, element_t, list);
    if (sp && bufsize) {
        strncpy(sp, entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    list_del_init(&entry->list);
    return entry;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *entry = list_last_entry(head, element_t, list);
    if (sp && bufsize) {
        strncpy(sp, entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    list_del_init(&entry->list);
    return entry;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    struct list_head *mid = _q_find_mid(head->next, head->prev);
    list_del(mid);
    q_release_element(list_entry(mid, element_t, list));

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    element_t *curr, *next = NULL;
    struct list_head *cache;
    bool dup = false;
    list_for_each_entry_safe (curr, next, head, list) {
        // Find every duplicate element
        while (&next->list != head && strcmp(curr->value, next->value) == 0) {
            next = element_next(next, list);
            dup = true;
        }
        // Delete all duplicate
        if (dup) {
            dup = false;
            // Connect not duplicate element
            cache = curr->list.prev;
            cache->next = &next->list;
            next->list.prev = cache;
            // Delete each duplicate element
            for (; &curr->list != &next->list;
                 curr = list_entry(cache, element_t, list)) {
                cache = curr->list.next;
                q_release_element(curr);
            }
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *node = head->next, *next = node->next, *cache;
    for (; node != head && next != head; node = cache, next = cache->next) {
        cache = next->next;

        struct list_head *node_prev = node->prev, *node_next = next->next;
        // Update neighbor node
        node_prev->next = next;
        node_next->prev = node;
        // Move node position to next
        node->prev = next;
        node->next = node_next;
        // Move next position to prev
        next->next = node;
        next->prev = node_prev;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    // Reverse the connection of all nodes
    // Example.-----------.      .-----------.      .-----------.
    //        | *n2 | *n1 |  ->  | *n0 | *n2 |  ->  | *n1 | *n0 |
    //        |  n0(head) |  <-  |     n1    |  <-  |     n2    |
    //        *-----------*      *-----------*      *-----------*
    // 1.     .-----------.   *node-||-------.   *next-------||-.
    //        | *n2 | *n1 |  ->  | *n2 | *n2 |  ->  | *n1 | *n1 |
    //        |  n0(head) |  <-  |     n1    |  <-  |     n2    |
    //        *-----------*      *-----------*      *-----------*
    // 2.  *next-------||-.      .-----------.   *node-||-------.
    //        | *n2 | *n2 |  ->  | *n2 | *n2 |  ->  | *n0 | *n1 |
    //        |  n0(head) |  <-  |     n1    |  <-  |     n2    |
    //        *-----------*      *-----------*      *-----------*
    // 3.  *node-||-------.   *next-------||-.      .-----------.
    //        | *n1 | *n2 |  ->  | *n2 | *n0 |  ->  | *n0 | *n1 |
    //        |  n0(head) |  <-  |     n1    |  <-  |     n2    |
    //        *-----------*      *-----------*      *-----------*
    // 3.     .-----------.   *node-------!!-.   *next----------.
    //        | *n1 | *n2 |  ->  | *n2 | *n0 |  ->  | *n0 | *n1 |
    //        |  n0(head) |  <-  |     n1    |  <-  |     n2    |
    //        *-----------*      *-----------*      *-----------*
    // 4. Because `node->next == head`, loop break.

    struct list_head *node = head->next, *next = node->next, *cache;
    for (; node->next != head; node = next, next = cache) {
        cache = next->next;
        node->prev = next;
        next->next = node;
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head) || k <= 1)
        return;

    struct list_head *next_gp = head->next, *curr, *node, *next, *cache;
    while (true) {
        curr = next_gp;
        // Find next group start
        int count = 0;
        while (count++ != k) {
            if (next_gp == head)
                return;
            next_gp = next_gp->next;
        }
        // Connect [current-group last-node].prev to [last-group last-node]
        next_gp->prev->prev = curr->prev;
        // Connect [last-group last-node].next to [current-group last-node]
        curr->prev->next = next_gp->prev;
        // Connect [next-group first-node].prev to [current-group first-node]
        next_gp->prev = curr;
        // Reverse connection
        for (node = curr, next = node->next; next != next_gp;
             node = next, next = cache) {
            cache = next->next;
            node->prev = next;
            next->next = node;
        }
        // Connect [current-group first-node].next to [next-group first-node]
        curr->next = next_gp;
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return;
    _q_merge_sort(head, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    _q_remove_not_in_order(head, false);

    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    _q_remove_not_in_order(head, true);

    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    queue_contex_t *out = list_first_entry(head, queue_contex_t, chain), *node;

    for (node = element_next(out, chain); &node->chain != head;
         node = element_next(node, chain)) {
        _q_merge(out->q, node->q, descend);
    }

    return q_size(out->q);
}
