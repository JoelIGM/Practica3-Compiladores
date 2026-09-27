#include "stack.h"
#include <stdlib.h>

void push(Node **top, void *value)
{
    Node *new = (Node *)malloc(sizeof(Node));
    new->value = value;
    new->next = *top;
    *top = new;
}
void* pop(Node **top) 
{
    if (*top == NULL) return NULL;
    Node *temp = *top;
    void *value = temp->value;
    *top = temp->next;
    free(temp);
    return value;
}