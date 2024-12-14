#include <stdint.h>
#include <string.h>

#include "grammar.h"


symbol **symbolmap = NULL;
const char* start_key = "__start__";


const char ***build_grammar(const char *const filename){
    const char ***map = calloc(mapsize, sizeof(char**));
    const size_t max_textlen = 1024;

    json_error_t json_err = {0};
    json_t *root = json_load_file(filename, 0, &json_err);

    for(size_t i=mapbase; i<mapsize; i++){
        map[i] = calloc(linewidth, sizeof(char*));
    }

    const char *key;
    json_t *value = NULL;
    unsigned mapidx = mapbase;
    size_t symidx = 1, line_len = 0;
    json_object_foreach(root, key, value){
        json_t *data = NULL;
        size_t linesz = json_array_size(value);
        if(strcmp(key, start_key) == 0){
            for(size_t i=0; i<linesz; i++){
                data = json_array_get(value, i);
                map[mapbase][i] = strndup(json_string_value(data), max_textlen);
            }
        }else{
            symbolmap_add(key, symidx++);
            for(size_t i=0; i<linesz; i++){
                data = json_array_get(value, i);
                map[mapidx][i] = strndup(json_string_value(data), max_textlen);
            }
        }
        mapidx++;
    }
    json_decref(root);
    for(unsigned mapidx=mapbase; mapidx<mapsize; mapidx++){
        line_len = linelen(map[mapidx]);
        if(line_len == 0){
            continue;
        }
        for(size_t idx=0; idx<line_len; idx++){
            char *addrdata = patch_symbol_addresses(map[mapidx][idx]);
            free((char*)map[mapidx][idx]);
            map[mapidx][idx] = addrdata;
        }
    }
    return map;
}

void print_grammar(const char ***grammar){
    size_t c_len;
    for(unsigned mapidx=mapbase; mapidx<mapsize; mapidx++){
        c_len = linelen(grammar[mapidx]);
        if(c_len == 0){
            continue;
        }
        printf("%x:\n", mapidx);
        for(size_t idx=0; idx<c_len; idx++){
            printf("  %s\n", grammar[mapidx][idx]);
        }
    }
}

void free_grammar(const char ***grammar){
    for(size_t midx=mapbase; midx<mapsize; midx++){
        for(size_t lidx=0; lidx<linewidth; lidx++){
            free((char*)grammar[midx][lidx]);
        }
    }
    for(size_t midx=mapbase; midx<mapsize; midx++){
        free(grammar[midx]);
    }
    free(grammar);
}

uint32_t fnv1a32_hashstr(const char *str){
    uint32_t hash = fnv1a32_basis;
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

void symbolmap_free(symbol **symbolmap){
    for(size_t idx=0; idx<mapsize; idx++){
        symbol *list = symbolmap[idx], *prev = NULL;
        while(list){
            prev = list;
            list = list->next;
            free(prev->symbol);
            prev->index = 0;
            prev->next = NULL;
            free(prev);
        }
    }
    free(symbolmap);
}

void symbolmap_add(const char *symtxt, size_t symidx){
    symbol *s = calloc(1, sizeof(symbol));
    s->symbol = strndup(symtxt, 64);  // tmplit
    s->index = symidx;
    s->next = NULL;

    uint32_t hash = fnv1a32_hashstr(s->symbol);
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
    uint32_t hash = fnv1a32_hashstr(symtxt);
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

char **extract_symbols(const char *str){
    const char* start = "{";
    const char* end = "}";
    size_t bufsz = 32;
    if(strlen(str) == 0){
        return NULL;
    }
    char *tokstr = strdup(str), *tok = NULL;
    char **tokens = calloc(bufsz, sizeof(char *));
    size_t idx = 0, endidx=0;

    tok = strtok(tokstr, start);
    if((strlen(tok) == strlen(str)) || !tok){
        goto tok_tidy_exit;
    }
    endidx = strcspn(tok, end);
    if(endidx < strlen(tok)){
        tokens[idx++] = strndup(tok, endidx);
    }
    while((tok = strtok(NULL, start))){
        endidx = strcspn(tok, end);
        if(endidx < strlen(tok)){
            tokens[idx++] = strndup(tok, endidx);
        }
        if(idx == bufsz){
            bufsz *= 2;
            tokens = realloc(tokens, bufsz*sizeof(char *));
        }
    }
    if(idx == 0){
        goto tok_tidy_exit;
    }
    tokens[idx++] = NULL;
    // Shrink the buffer
    tokens = realloc(tokens, idx*sizeof(char *));
    free(tokstr);
    return tokens;
tok_tidy_exit:
    free(tokstr);
    free(tokens);
    return NULL;
}

size_t linelen(const char **line){
    size_t len = 0;
    while(line[len++]){
        if(len == linewidth){
            return linewidth;
        }
    }
    return len-1;
}

char *patch_symbol_addresses(const char *text){
    char **symbols = extract_symbols(text);
    if(!symbols){
        return strdup(text);
    }
    size_t symidx = 0;
    size_t txtlen = strlen(text);
    char *patched = calloc(txtlen, sizeof(char));
    char *pnext = patched;
    char *str = (char *)text;
    /*  Note that symbols are extracted above in
     *  the order and multiplicity they are encountered
     *  in *text. As such, we're certain to encounter
     *  them again here in the same order.
     */
    while(symbols[symidx]){
        size_t slen = strlen(str);
        size_t symlen = strlen(symbols[symidx]);
        size_t mapidx = symbolmap_getidx(symbols[symidx]);
        (void)mapidx;
        char *sidx = strstr(str, symbols[symidx]);
        if(!sidx){
            continue;
        }
        if(sidx - str > 0 && sidx + symlen - str <= (long)slen){
            if(sidx[-1] == '{' && sidx[symlen] == '}'){
                strncpy(pnext, str, sidx-str-1);
                pnext += (sidx-str-1);
                *pnext = (char)(mapbase + mapidx);
                pnext++;
            }
        }
        str = sidx + symlen + 1;
        symidx++;
    }
    strcpy(pnext, str);
    for(size_t idx=0; symbols[idx]; idx++){
        free(symbols[idx]);
    }
    free(symbols);
    return patched;
}
