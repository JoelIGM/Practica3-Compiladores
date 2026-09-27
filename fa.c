#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "fa.h"


void print_automata(fa *automata) {
    printf("========== NFA ==========\n");
    printf(" Start state: %d\n", automata->start);

    printf("\n Accept states: \n");

    for (int i = 0; i < automata->length_accept_states; i++) {
        printf("%d ", automata->accept_states[i]);
    }

    printf("\nTransitions:\n");

    for (int i = 0; i < automata->length; i++) {
        transition t = automata->transitions[i];

        if (t.symbol == EPSILON) {
            printf("  %d --(eps)--> %d\n", t.start, t.finish);
        } else {
            printf("  %d --(%c)--> %d\n", t.start, t.symbol, t.finish);
        }
    }
    printf("=========================\n");
}


bool belongs(list *l, int value) {
    node *actual = l->head;

    while (actual != NULL) {
        if (*(int *)actual->data == value) {
            return true;
        }

        actual = actual->next;
    }

    return false;
}


list *move(fa automata, int *T, int length_T, char a) {

    list *R = malloc(sizeof(list));
    *R = list_create();

    for (int i = 0; i < length_T; i++) {

        int s = T[i];

        for (int j = 0; j < automata.length; j++) {

            transition t = automata.transitions[j];

            if (t.start == s && t.symbol == a) {

                if (!belongs(R, t.finish)) {

                    int *new_state = malloc(sizeof(int));

                    *new_state = t.finish;

                    list_push(R, new_state);
                }
            }
        }
    }

    return R;
}


int epsilon_closure(
    const fa *automata,
    const int *current_states,
    int current_count,
    int *out_closure,
    int max_states
) {
    if (automata == NULL ||
        current_states == NULL ||
        current_count <= 0 ||
        out_closure == NULL ||
        max_states <= 0) {

        return 0;
    }

    bool *visited = calloc(max_states, sizeof(bool));

    if (visited == NULL) {
        return 0;
    }

    int *stack = malloc(sizeof(int) * max_states);

    if (stack == NULL) {
        free(visited);
        return 0;
    }

    int top = -1;
    int closure_count = 0;

    for (int i = 0; i < current_count; i++) {

        int s = current_states[i];

        if (s >= 0 && s < max_states && !visited[s]) {

            visited[s] = true;
            out_closure[closure_count++] = s;
            stack[++top] = s;
        }
    }

    while (top >= 0) {

        int state = stack[top--];

        for (int i = 0; i < automata->length; i++) {

            transition t = automata->transitions[i];

            if (t.start == state && t.symbol == EPSILON) {

                int next = t.finish;

                if (next >= 0 &&
                    next < max_states &&
                    !visited[next]) {

                    visited[next] = true;
                    out_closure[closure_count++] = next;
                    stack[++top] = next;
                }
            }
        }
    }

    free(stack);
    free(visited);

    return closure_count;
}