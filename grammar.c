#include <stdint.h>
#include <string.h>

#include "grammar.h"


#define mapsize 256

symbol **symbolmap = NULL;


const char ***build_grammar(const char *const filename){
    const char ***map = calloc(256, sizeof(char**));
    const unsigned mapbase = 0x80;
    const size_t max_textlen = 512;

    json_error_t json_err = {0};
    json_t *root = json_load_file(filename, 0, &json_err);
    size_t nkeys = json_object_size(root);
    (void)nkeys;  // unused

    for(size_t i=0; i<256; i++){
        map[i] = calloc(linewidth, sizeof(char*));
    }

    const char *key;
    json_t *value = NULL;
    unsigned mapidx = mapbase;
    json_object_foreach(root, key, value){
        json_t *data = NULL;
        //printf("%s:\n", key);
        //size_t keyidx = (size_t)atoi(key);
        //(void)keyidx;
        size_t linesz = json_array_size(value);
        for(size_t i=0; i<linesz; i++){
            data = json_array_get(value, i);
            //printf("  %s\n", json_string_value(data));
            map[mapidx][i] = strndup(json_string_value(data), max_textlen);
        }
        mapidx++;
    }

    json_decref(root);
    return map;
}

void free_grammar(const char ***grammar){
    for(size_t midx=0; midx<256; midx++){
        for(size_t lidx=0; lidx<linewidth; lidx++){
            free((char *)grammar[midx][lidx]);
        }
    }
    for(size_t midx=0; midx<256; midx++){
        free(grammar[midx]);
    }
    free(grammar);
}

uint32_t fnv1a32_hashstr(const char *str, uint32_t hash){
    uint8_t *oct = (uint8_t *)str;
    while(*oct){
        hash ^= (uint32_t)*oct++;
        hash *= fnv1a32_prime;
    }
    return hash;
}

size_t symbolmap_init(symbol ***map){
    *map = calloc(mapsize, sizeof(symbol **));
    return mapsize;
}

void symbolmap_add(const char *symtxt, size_t symidx){
    symbol *s = calloc(1, sizeof(symbol));
    s->symbol = strndup(symtxt, 64);  // tmplit
    s->index = symidx;
    s->next = NULL;

    uint32_t hash = fnv1a32_hashstr(s->symbol, fnv1a32_basis);
    uint8_t bucket = hash & 0xFF;

    symbol **list = &symbolmap[bucket];
    while(*list){
        //if(strcmp(symtxt, *list->symbol) == 0){
        //    ; // err already in list
        //}
        list = &(*list)->next;
    }
    *list = s;
}

size_t symbolmap_getidx(const char *symtxt){
    uint32_t hash = fnv1a32_hashstr(symtxt, fnv1a32_basis);
    uint8_t bucket = hash & 0xFF;

    symbol *list = symbolmap[bucket];
    while(list){
        if(strcmp(symtxt, list->symbol) == 0){
            return list->index;
        }
        list = list->next;
    }
    return 0;  // Only __start__ can have a zero symbol-index
}

char **extract_tokens(const char *str){
    const char* start = "$(";
    const char* end = ")";
    size_t bufsz = 32;
    char *tokstr = strdup(str), *tok = NULL;
    char **tokens = calloc(32, sizeof(char *));
    size_t idx = 0, endidx=0;

    tok = strtok(tokstr, start);
    if(strlen(tok) == strlen(tokstr)){
        free(tokens);
        return NULL;
    }
    endidx = strcspn(tok, end);
    tokens[idx++] = strndup(tok, endidx);
    while((tok = strtok(NULL, start))){
        endidx = strcspn(tok, end);
        tokens[idx++] = strndup(tok, endidx);
        if(idx == bufsz){
            bufsz *= 2;
            tokens = realloc(tokens, bufsz*sizeof(char *));
        }
    }
    tokens[idx++] = NULL;
    // Shrink the buffer
    tokens = realloc(tokens, idx*sizeof(char *));
    free(tokstr);
    return tokens;
}
