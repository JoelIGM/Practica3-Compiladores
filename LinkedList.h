#ifndef LIST_H
#define LIST_H


#ifdef __cplusplus
extern "C" {
#endif

typedef struct node {
    void *data;
    struct node *next;
} node;

typedef struct list {
    node *head;
    int length;
} list;

list list_create(void);

void list_push(list *l, void *data);

void list_free(list *l);

#ifdef __cplusplus
}
#endif

#endif