#include "regex.h"
#include "nfa.h"
#include "stack.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

//Used in Thompson for saving the start and finish of the nfa
typedef struct 
{
    int fst;
    int snd;
} pair;


nfa regex_to_nfa(regex r)
{   
    int size_regex = r.length;
    int total_transitions = 0;
    //Count the transitions and states of the future nfa
    for (int i = 0; i < size_regex; i++)
    {
        switch(r.regex_symbols[i])
        {
        case '*':
            total_transitions += 4;
            break;
        case '+':
            total_transitions += 3;   // <-- nuevo
            break;
        case '?':
            total_transitions += 3;   // <-- nuevo
            break;
        case '|':
            total_transitions += 4;
            break;
        case '.':
            total_transitions++;
            break;
        default:
            total_transitions++;
            break;
        }
    }
    
    //Implementation of Thompson's algorithm
    Node *stack = NULL;
    int avaiable_state = 0;
    transition *transitions = malloc(sizeof(transition) * total_transitions);
    int actual_transition = 0;

    for (int i = 0; i < size_regex; i++)
    {
        switch(r.regex_symbols[i])
        {
        case '*':
            {
            //take just one nfa from the stack
            pair *p1 = (pair *)pop(&stack);
            //values of the 2 new states
            int state1 = avaiable_state;
            int state2 = avaiable_state+1;
            avaiable_state = avaiable_state + 2;
            //transitions since state1 to first of p1 and second of p1 to state2 
            transition t1 = {state1,p1->fst,EPSILON};
            transitions[actual_transition] = t1; 
            actual_transition++;
            transition t2 = {p1->snd,state2,EPSILON};
            transitions[actual_transition] = t2; 
            actual_transition++;
            //transition since state1 to state2
            transition t3 = {state1,state2,EPSILON};
            transitions[actual_transition] = t3; 
            actual_transition++;
            //transition since second of p1 to first of p1 to let cycle
            transition t4 = {p1->snd,p1->fst,EPSILON};
            transitions[actual_transition] = t4; 
            actual_transition++;
            //take the new start and accept states and added to the stack
            pair *p = malloc(sizeof(pair));
            p->fst=state1;
            p->snd=state2;
            push(&stack,p);
            //free memory of the pair removed from the stack
            free(p1);
            break;
            }
        case '|':
            {
            //take the begin and finish of the nfas from the stack
            pair *p2 = (pair *)pop(&stack);
            pair *p1 = (pair *)pop(&stack);
            //values of the 2 new states
            int state1 = avaiable_state;
            int state2 = avaiable_state+1;
            avaiable_state = avaiable_state + 2;
            //transitions since state1 to first of p1 and first of p2
            transition t1 = {state1,p1->fst,EPSILON};
            transitions[actual_transition] = t1; 
            actual_transition++;
            transition t2 = {state1,p2->fst,EPSILON};
            transitions[actual_transition] = t2; 
            actual_transition++;
            //transitions since the second of p1 and second of p2 to state2
            transition t3 = {p1->snd,state2,EPSILON};
            transitions[actual_transition] = t3; 
            actual_transition++;
            transition t4 = {p2->snd,state2,EPSILON};
            transitions[actual_transition] = t4; 
            actual_transition++;
            //take the new start and accept states and added to the stack
            pair *p = malloc(sizeof(pair));
            p->fst=state1;
            p->snd=state2;
            push(&stack,p);
            //free memory of the pairs removed from the stack
            free(p1);
            free(p2);
            break;
            }
        case '.':
            {
            pair *p2 = (pair *)pop(&stack);
            pair *p1 = (pair *)pop(&stack);
            //transition of the second of p1 and the first of p2
            transition t = {p1->snd,p2->fst,EPSILON};
            transitions[actual_transition] = t; 
            actual_transition++;
            //take the new start and accept states and added to the stack
            pair *p = malloc(sizeof(pair));
            p->fst=p1->fst;
            p->snd=p2->snd;
            push(&stack,p);
            //free memory of the pairs removed from the stack
            free(p1);
            free(p2);
            break;
            }

            case '+':
            {
            pair *p1 = (pair *)pop(&stack);
            int state1 = avaiable_state;
            int state2 = avaiable_state + 1;
            avaiable_state += 2;
            transition t1 = {state1, p1->fst, EPSILON};
            transitions[actual_transition++] = t1;

            transition t2 = {p1->snd, p1->fst, EPSILON};
            transitions[actual_transition++] = t2;

            transition t3 = {p1->snd, state2, EPSILON};
            transitions[actual_transition++] = t3;

            pair *p = malloc(sizeof(pair));
            p->fst = state1;
            p->snd = state2;
            push(&stack, p);
            free(p1);
            break;
            }
        case '?':
            {
            pair *p1 = (pair *)pop(&stack);
            int state1 = avaiable_state;
            int state2 = avaiable_state + 1;
            avaiable_state += 2;

            transition t1 = {state1, p1->fst, EPSILON};
            transitions[actual_transition++] = t1;

            transition t2 = {state1, state2, EPSILON};
            transitions[actual_transition++] = t2;

            transition t3 = {p1->snd, state2, EPSILON};
            transitions[actual_transition++] = t3;

            pair *p = malloc(sizeof(pair));
            p->fst = state1;
            p->snd = state2;
            push(&stack, p);
            free(p1);
            break;
            }
        
        default:
            {
            int state1 = avaiable_state;
            int state2 = avaiable_state+1;
            avaiable_state = avaiable_state + 2;
            transition t = {state1,state2,r.regex_symbols[i]};
            transitions[actual_transition] = t;
            actual_transition++;
            //Insert the initial and final state of the nfa to the stack
            pair *p = malloc(sizeof(pair));
            p->fst=state1;
            p->snd=state2;
            push(&stack,p);
            break;
            }
        }
    }

    pair *p = (pair *)pop(&stack);
    nfa final_nfa = {transitions,p->fst,p->snd,total_transitions,actual_transition};
    free(p);
    return final_nfa;
}

// Circular Queue structure for BFS
typedef struct {
    int *data;
    int front;
    int rear;
    int capacity;
} Queue;

static void queue_init(Queue *q, int *buffer, int capacity) {
    q->data = buffer;
    q->front = 0;
    q->rear = 0;
    q->capacity = capacity;
}

static inline bool queue_is_empty(const Queue *q) {
    return q->front == q->rear;
}

static inline bool queue_is_full(const Queue *q) {
    return ((q->rear + 1) % q->capacity) == q->front;
}

static inline bool queue_push(Queue *q, int state) {
    if (queue_is_full(q)) {
        return false;
    }
    q->data[q->rear] = state;
    q->rear = (q->rear + 1) % q->capacity;
    return true;
}

static inline int queue_pop(Queue *q) {
    if (queue_is_empty(q)) {
        return -1;
    }
    int state = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    return state;
}

//Helper to safely compute the upper bound of state IDs
static int get_max_states(const nfa *n) {
    int max_id = (n->start > n->accept) ? n->start : n->accept;
    for (int i = 0; i < n->length; i++) {
        if (n->transitions[i].start > max_id)  max_id = n->transitions[i].start;
        if (n->transitions[i].finish > max_id) max_id = n->transitions[i].finish;
    }
    if (n->states > max_id) {
        max_id = n->states - 1;
    }
    return max_id + 1;
}

//Compute the epsilon closure of a set of states using BFS with a circular queue
int epsilon_closure_(const nfa *n, const int *current_states, int current_count, 
                    int *out_closure, int max_states) {
    if (n == NULL || current_states == NULL || current_count <= 0 || 
        out_closure == NULL || max_states <= 0) {
        return 0;
    }

    //Visited array to avoid infinite loops from epsilon cycles
    bool *visited = (bool *)calloc((size_t)max_states, sizeof(bool));
    if (visited == NULL) {
        return 0;
    }

    //Queue capacity: max_states + 1 allows holding up to max_states elements
    //with the condition ((rear + 1) % capacity == front)
    int queue_capacity = max_states + 1;
    int *queue_buffer = (int *)malloc((size_t)queue_capacity * sizeof(int));
    if (queue_buffer == NULL) {
        free(visited);
        return 0;
    }

    Queue q;
    queue_init(&q, queue_buffer, queue_capacity);

    int closure_count = 0;

    //Enqueue initial states, record in closure and mark them as visited
    for (int i = 0; i < current_count; i++) {
        int s = current_states[i];
        if (s >= 0 && s < max_states && !visited[s]) {
            visited[s] = true;
            out_closure[closure_count++] = s;
            queue_push(&q, s);
        }
    }

    //Breadth-First Search
    while (!queue_is_empty(&q)) {
        int u = queue_pop(&q);

        for (int i = 0; i < n->length; i++) {
            if (n->transitions[i].start == u && n->transitions[i].symbol == EPSILON) {
                int v = n->transitions[i].finish;
                if (v >= 0 && v < max_states && !visited[v]) {
                    visited[v] = true;
                    out_closure[closure_count++] = v;
                    queue_push(&q, v);
                }
            }
        }
    }

    //Free allocated resources
    free(queue_buffer);
    free(visited);
    return closure_count;
}

//Simulate NFA on input buffer: returns 1 if accepted, 0 otherwise
int match_nfa(nfa n, char *buf, int len) {
    if (n.transitions == NULL || n.length < 0 || len < 0 || (len > 0 && buf == NULL)) {
        return 0;
    }

    int max_states = get_max_states(&n);
    if (max_states <= 0) {
        return 0;
    }

    //Allocate working buffers
    int *closure_states = (int *)malloc((size_t)max_states * sizeof(int));
    int *next_states    = (int *)malloc((size_t)max_states * sizeof(int));
    bool *in_next       = (bool *)calloc((size_t)max_states, sizeof(bool));

    if (!closure_states || !next_states || !in_next) {
        free(closure_states);
        free(next_states);
        free(in_next);
        return 0;
    }

    //Start with epsilon closure of start state
    int initial_state = n.start;
    int closure_count = epsilon_closure_(&n, &initial_state, 1, closure_states, max_states);

    //Consume each symbol of the string
    for (int i = 0; i < len; i++) {
        char c = buf[i];
        int next_count = 0;
        memset(in_next, 0, (size_t)max_states * sizeof(bool));

        for (int j = 0; j < closure_count; j++) {
            int u = closure_states[j];

            for (int k = 0; k < n.length; k++) {
                if (n.transitions[k].start == u && n.transitions[k].symbol == c) {
                    int v = n.transitions[k].finish;
                    if (v >= 0 && v < max_states && !in_next[v]) {
                        in_next[v] = true;
                        next_states[next_count++] = v;
                    }
                }
            }
        }

        //Early exit if no transitions were possible with current symbol
        if (next_count == 0) {
            closure_count = 0;
            break;
        }

        //Compute epsilon closure of next reachable states
        closure_count = epsilon_closure_(&n, next_states, next_count, closure_states, max_states);
    }

    //Check if accept state was reached in final closure
    int accepted = 0;
    for (int i = 0; i < closure_count; i++) {
        if (closure_states[i] == n.accept) {
            accepted = 1;
            break;
        }
    }

    //Free all allocated memory to prevent memory leaks
    free(closure_states);
    free(next_states);
    free(in_next);

    return accepted;
}

//Free NFA transitions memory
void free_nfa(nfa *n) {
    if (n != NULL && n->transitions != NULL) {
        free(n->transitions);
        n->transitions = NULL;
        n->length = 0;
        n->states = 0;
    }
}

//Serialize NFA to output file 
bool save_nfa(const nfa *n, const char *output_path) {
    if (n == NULL || output_path == NULL) {
        return false;
    }

    FILE *f = fopen(output_path, "w");
    if (f == NULL) {
        return false;
    }

    fprintf(f, "%d %d %d %d\n", n->start, n->accept, n->length, n->states);
    for (int i = 0; i < n->length; i++) {
        fprintf(f, "%d %d %c\n", n->transitions[i].start, n->transitions[i].finish,
                n->transitions[i].symbol == EPSILON ? 'e' : n->transitions[i].symbol);
    }

    fclose(f);
    return true;
}


