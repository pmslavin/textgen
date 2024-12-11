#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <stdint.h>

#include <jansson.h>

#define linewidth 32
#define fnv1a32_basis ((uint32_t)0x811c9dc5)
#define fnv1a32_prime ((uint32_t)0x01000193)

typedef struct _symbol{
    char *symbol;
    size_t index;
    struct _symbol *next;
}symbol;

const char ***build_grammar(const char *const);
void free_grammar(const char ***);
uint32_t fnv1a32_hashstr(const char *, uint32_t);
size_t symbolmap_init(symbol ***);
void symbolmap_add(const char *symtxt, size_t symidx);
size_t symbolmap_getidx(const char *symtxt);
char **extract_tokens(const char *);

#endif
