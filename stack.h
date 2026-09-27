#ifndef STACK_H
#define STACK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Node {
    void *value;
    struct Node *next;
} Node;

// Inserts a value on the top of the stack
void push(Node **top, void *value);

// Return and delete the vale on the top of the stack and return NULL if empty
void* pop(Node **top);

int is_empty(Node *top);

#ifdef __cplusplus
}
#endif

#endif