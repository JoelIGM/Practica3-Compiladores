#ifndef REGEX_H
#define REGEX_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char value;
} RegexItem;

typedef struct {
    RegexItem *items;
    char *regex_symbols;
    int length;
    int size;
} regex;

regex parse_regex(const char *infix);
void free_regex(regex *r);

#ifdef __cplusplus
}
#endif

#endif