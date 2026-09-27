#include "regex.h"
#include "stack.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Adds items to regex
void regex_add_item(regex *r, char c) {
    r->regex_symbols = (char *)realloc(r->regex_symbols, sizeof(char) * (r->length + 1));
    r->regex_symbols[r->length] = c;
    r->length++;

    r->items = (RegexItem *)realloc(r->items, sizeof(RegexItem) * (r->size + 1));
    r->items[r->size].value = c;
    r->size++;
}

// Precedence definition
int get_precedence(char c) {
    switch(c) {
        case '*': return 4; // +
        case '+': return 4;
        case '?': return 4;
        case '.': return 3;
        case '|': return 2; // -
        default:  return 0;
    }
}


int is_operator(char c) {
    return c == '*' || c == '+' || c == '?' || c == '|' || c == '.';
}


regex parse_regex(const char *infix) {
    regex result;
    result.regex_symbols = NULL;
    result.length = 0;
    result.items = NULL;
    result.size = 0;
    
    Node *stack = NULL;  // Stack for operators
    
    // Concatenation variables
    char prev_char = '\0';
    int i = 0;
    
    while (infix[i] != '\0') {
        char c = infix[i];
        
        // Reads the implicit concatenation
        if (i > 0 && 
            ((isalnum(prev_char) || prev_char == ')' || prev_char == '*' || prev_char == '+' || prev_char == '?') && 
             (isalnum(c) || c == '('))) {
            // Inserts operator '.'
            char *dot = (char *)malloc(sizeof(char));
            *dot = '.';
            
            while (stack != NULL) {
                char *top_val = (char *)(stack->value);
                if (*top_val == '(') break;
                
                int prec_top = get_precedence(*top_val);
                int prec_dot = get_precedence('.');
                
                if (prec_top >= prec_dot) {
                    char *popped = (char *)pop(&stack);
                    regex_add_item(&result, *popped);
                    free(popped);
                } else {
                    break;
                }
            }
            
            push(&stack, dot);
        }
        
        if (isalnum(c)) {
            regex_add_item(&result, c);
        }
        else if (c == '(') {
            char *paren = (char *)malloc(sizeof(char));
            *paren = '(';
            push(&stack, paren);
        }
        else if (c == ')') {
            while (1) {
                char *top_val = (char *)pop(&stack);
                if (top_val == NULL) break;
                if (*top_val == '(') {
                    free(top_val);
                    break;
                }
                regex_add_item(&result, *top_val);
                free(top_val);
            }
        }
        else if (is_operator(c)) {
            // Checks if the operator is unary
            if (c == '*' || c == '+' || c == '?') {
                char *op = (char *)malloc(sizeof(char));
                *op = c;
                
                // Procesess the precedence if there are more operators in the stack
                while (stack != NULL) {
                    char *top_val = (char *)(stack->value);
                    if (*top_val == '(') break;
                    if (get_precedence(*top_val) >= get_precedence(c)) {
                        char *popped = (char *)pop(&stack);
                        regex_add_item(&result, *popped);
                        free(popped);
                    } else break;
                }
                push(&stack, op);
            } else {
                // For binary operators
                while (stack != NULL) {
                    char *top_val = (char *)(stack->value);
                    if (*top_val == '(') break;
                    
                    int prec_top = get_precedence(*top_val);
                    int prec_c = get_precedence(c);
                    
                    if (prec_top >= prec_c) {
                        char *popped = (char *)pop(&stack);
                        regex_add_item(&result, *popped);
                        free(popped);
                    } else {
                        break;
                    }
                }
                
                char *op = (char *)malloc(sizeof(char));
                *op = c;
                push(&stack, op);
            }
        }
        
        prev_char = c;
        i++;
    }
    
    // Empties the stack
    while (stack != NULL) {
        char *val = (char *)pop(&stack);
        if (val != NULL) {
            regex_add_item(&result, *val);
            free(val);
        }
    }
    
    return result;
}

// Frees resources from regex
void free_regex(regex *r) {
    if (r->regex_symbols) {
        free(r->regex_symbols);
        r->regex_symbols = NULL;
    }
    if (r->items) {
        free(r->items);
        r->items = NULL;
    }
    r->length = 0;
    r->size = 0;
}