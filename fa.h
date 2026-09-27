#ifndef FA_H
#define FA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "LinkedList.h"

#define EPSILON '\0'

typedef struct transition {
    int start;
    int finish;
    char symbol;
} transition;

typedef struct fa {
    transition *transitions;
    int start;
    int *accept_states;
    int length_accept_states;
    int length;
    int states;
} fa;


/* Funciones del NFA */

void print_automata(fa *automata);

bool belongs(list *l, int value);

list *move(fa automata, int *T, int length_T, char a);

int epsilon_closure(
    const fa *automata,
    const int *current_states,
    int current_count,
    int *out_closure,
    int max_states
);


/* DFA */

typedef struct dfa_state {
    int *states;
    int count;
} dfa_state;


typedef struct dfa {
    dfa_state *states;
    int length_states;

    transition *transitions;
    int length_transitions;

    int start;

    int *accept_states;
    int length_accept_states;

} dfa;


dfa *nfa_to_dfa(
    const fa *nfa,
    const char *alphabet,
    int alphabet_size
);

void free_dfa(dfa *automata);

void print_dfa(const dfa *automata);

#ifdef __cplusplus
}
#endif

#endif