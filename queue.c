#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

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
    bool dup = false;
    list_for_each_entry_safe (curr, next, head, list) {
        // Check if pair is duplicate
        if (&next->list != head && strcmp(curr->value, next->value) == 0) {
            list_del(&curr->list);
            q_release_element(curr);
            dup = true;
        }
        // If last pair is duplicate string, remove the last duplicate
        else if (dup) {
            list_del(&curr->list);
            q_release_element(curr);
            dup = false;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *node, *next;
    list_for_each_safe (node, next, head) {
        if (next == head)
            break;
        list_move(node, next);
        next = node->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head)
        list_move(node, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend) {}

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
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
