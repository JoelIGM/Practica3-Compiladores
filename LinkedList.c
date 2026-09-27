#include <stdlib.h>
#include "LinkedList.h"

list list_create(void) {
    list l;
    l.head = NULL;
    l.length = 0;
    return l;
}

/*
 * Interts at the begining of the list
 * data is a pointer already created with malloc
 */
void list_push(list *l, void *data) {
    node *new_node = malloc(sizeof(node));
    new_node->data = data;
    new_node->next = l->head;
    l->head = new_node; 
    l->length++;
}

/*
 * Free memory of the list
 */
void list_free(list *l) {
    node *actual = l->head;
    while (actual != NULL) {
        node *next_ = actual->next;
        free(actual->data);
        free(actual);
        actual = next_;
    }
    l->head = NULL;
    l->length = 0;
}
