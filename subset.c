#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "fa.h"

//True if a set of states already exists in DFA
static bool same_set(
    const int *a,
    int count_a,
    const int *b,
    int count_b
) {
    if (count_a != count_b) {
        return false;
    }

    for (int i = 0; i < count_a; i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }

    return true;
}

// Search for set of states in Qd.
static int find_state_set(
    const dfa *automata,
    const int *states,
    int count
) {
    for (int i = 0; i < automata->length_states; i++) {

        if (same_set(
            automata->states[i].states,
            automata->states[i].count,
            states,
            count
        )) {
            return i;
        }
    }

    return -1;
}

// Each set of states from NFA is transformed into a state in DFA.
static int add_dfa_state(
    dfa *automata,
    const int *states,
    int count
) {
    int index = automata->length_states;

    automata->states = realloc(
        automata->states,
        sizeof(dfa_state) * (index + 1)
    );

    automata->states[index].states = malloc(sizeof(int) * count);

    for (int i = 0; i < count; i++) {
        automata->states[index].states[i] = states[i];
    }

    automata->states[index].count = count;

    automata->length_states++;

    return index;
}

static int compare_ints(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

// Definition of a queque for algorithm 3

typedef struct queue {
    int *items;
    int front;
    int rear;
    int capacity;
} queue;

static queue *queue_create(void) {
    queue *q = malloc(sizeof(queue));

    q->capacity = 10;
    q->items = malloc(sizeof(int) * q->capacity);

    q->front = 0;
    q->rear = 0;

    return q;
}

static bool queue_empty(queue *q) {
    return q->front == q->rear;
}

static void enqueue(queue *q, int value) {

    if (q->rear >= q->capacity) {
        q->capacity *= 2;

        q->items = realloc(
            q->items,
            sizeof(int) * q->capacity
        );
    }

    q->items[q->rear++] = value;
}

static int dequeue(queue *q) {
    return q->items[q->front++];
}

static void queue_free(queue *q) {
    free(q->items);
    free(q);
}

// Adds a transition to DFA
static void add_transition(
    dfa *automata,
    int start,
    int finish,
    char symbol
) {
    automata->transitions = realloc(
        automata->transitions,
        sizeof(transition) * (automata->length_transitions + 1)
    );

    int index = automata->length_transitions;

    automata->transitions[index].start = start;
    automata->transitions[index].finish = finish;
    automata->transitions[index].symbol = symbol;

    automata->length_transitions++;
}

dfa *nfa_to_dfa(
    const fa *nfa,
    const char *alphabet,
    int alphabet_size
) {
    if (nfa == NULL || alphabet == NULL || alphabet_size <= 0) {
        return NULL;
    }

    dfa *result = malloc(sizeof(dfa));

    result->states = NULL;
    result->length_states = 0;

    result->transitions = NULL;
    result->length_transitions = 0;

    result->accept_states = NULL;
    result->length_accept_states = 0;

    //s0 = epsilon-closure({q0})
    int *initial_set = malloc(sizeof(int) * nfa->states);

    int initial_count = epsilon_closure(
        nfa,
        &nfa->start,
        1,
        initial_set,
        nfa->states
    );

    if (initial_count <= 0) {
        free(initial_set);
        free(result);
        return NULL;
    }

    qsort(
        initial_set,
        initial_count,
        sizeof(int),
        compare_ints
    );
    
    //QD = {s0}
    int initial_index = add_dfa_state(
        result,
        initial_set,
        initial_count
    );

    result->start = initial_index;
    queue *q = queue_create();
    enqueue(q, initial_index);
    free(initial_set);

    while (!queue_empty(q)) {
        int current_index = dequeue(q);
        dfa_state current = result->states[current_index];

        //for all a ∈ Σ
        for (int i = 0; i < alphabet_size; i++) {
            char symbol = alphabet[i];

            //V = epsilon-closure(Move(U,a))
            list *move_result = move(
                *nfa,
                current.states,
                current.count,
                symbol
            );

            int *move_states = malloc(
                sizeof(int) * nfa->states
            );
            int move_count = 0;
            node *actual = move_result->head;

            while (actual != NULL) {
                int value = *((int *)actual->data);
                move_states[move_count++] = value;
                actual = actual->next;
            }

            if (move_count == 0) {
                list_free(move_result);
                free(move_result);
                free(move_states);
                continue;
            }


            // Calling epsilon-closure
            int *closure = malloc(
                sizeof(int) * nfa->states
            );

            int closure_count = epsilon_closure(
                nfa,
                move_states,
                move_count,
                closure,
                nfa->states
            );

            list_free(move_result);
            free(move_result);
            free(move_states);


            if (closure_count == 0) {
                free(closure);
                continue;
            }

            // Sorting sets so it is easier to compare them
            qsort(
                closure,
                closure_count,
                sizeof(int),
                compare_ints
            );

            //if V not in QD
            int destination_index = find_state_set(
                result,
                closure,
                closure_count
            );

            if (destination_index == -1) {
                destination_index = add_dfa_state(
                    result,
                    closure,
                    closure_count
                );
                enqueue(q, destination_index);
            }

            //D(U,a) = V
            add_transition(
                result,
                current_index,
                destination_index,
                symbol
            );
            free(closure);
        }
    }


    //Final states of DFA

    for (int i = 0; i < result->length_states; i++) {

        dfa_state state = result->states[i];

        bool is_accepting = false;


        for (int j = 0; j < state.count; j++) {

            int nfa_state = state.states[j];

            for (int k = 0; k < nfa->length_accept_states; k++) {

                if (nfa_state == nfa->accept_states[k]) {
                    is_accepting = true;
                    break;
                }
            }

            if (is_accepting) {
                break;
            }
        }


        if (is_accepting) {

            result->accept_states = realloc(
                result->accept_states,
                sizeof(int) *
                (result->length_accept_states + 1)
            );

            result->accept_states[
                result->length_accept_states++
            ] = i;
        }
    }


    queue_free(q);

    return result;
}

void free_dfa(dfa *automata) {

    if (automata == NULL) {
        return;
    }

    for (int i = 0; i < automata->length_states; i++) {
        free(automata->states[i].states);
    }

    free(automata->states);
    free(automata->transitions);
    free(automata->accept_states);

    free(automata);
}


void print_dfa(const dfa *automata) {

    if (automata == NULL) {
        return;
    }

    printf("\n========== DFA ==========\n");

    printf("Start state: %d\n", automata->start);

    printf("States:\n");

    for (int i = 0; i < automata->length_states; i++) {

        printf("  q%d = { ", i);

        for (int j = 0; j < automata->states[i].count; j++) {
            printf("%d", automata->states[i].states[j]);

            if (j + 1 < automata->states[i].count) {
                printf(", ");
            }
        }

        printf(" }\n");
    }


    printf("\nAccept states: ");

    for (int i = 0; i < automata->length_accept_states; i++) {
        printf("q%d ", automata->accept_states[i]);
    }


    printf("\n\nTransitions:\n");

    for (int i = 0; i < automata->length_transitions; i++) {

        transition t = automata->transitions[i];

        printf(
            "  q%d --(%c)--> q%d\n",
            t.start,
            t.symbol,
            t.finish
        );
    }

    printf("=========================\n");
}