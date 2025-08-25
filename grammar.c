#include "config.h"
#define _POSIX_C_SOURCE 200809L  // strndup
#ifdef USE_ASPRINTF
  #define _GNU_SOURCE            // asprintf
#else
  #include <stdarg.h>
#endif
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include "grammar.h"
#include "operators.h"


symbol **symbolmap = NULL;
const char *start_key = "__start__";
const char *metadata_key = "__metadata__";
const char *min_version_key = "min_version";


const char ***build_grammar(json_t *root){
    /*  Builds the grammar represented by the json <root> arg.
     *
     *  Note that all routes call json_decref and invalidate
     *  the <root> argument.
     */
    const char ***map = calloc(mapsize, sizeof(char**));
    if(!map){
        fputs("Insufficient memory for grammar.\n", stderr);
        exit(1);
    }

    for(size_t i=mapbase; i<mapsize; i++){
        map[i] = calloc(linewidth, sizeof(char*));
        if(!map[i]){
            fputs("Insufficient memory for grammar.\n", stderr);
            exit(1);
        }
    }

    const char *key;
    json_t *value = NULL;
    unsigned mapidx = mapbase;
    size_t symidx = 1, line_len = 0;
    json_object_foreach(root, key, value){
        if(strcmp(key, metadata_key) == 0){
            /* Process metadata */
            continue;
        }
        json_t *data = NULL;
        if(!json_is_array(value)){
            fprintf(stderr, "Invalid definition of '%s' value must be an array, not: %s\n",
                    key, json_string_value(value));
            goto gram_tidy_exit;
        }
        size_t linesz = json_array_size(value);
        if(linesz == 0){
            fprintf(stderr, "Invalid definition of '%s': array may not be empty\n", key);
            goto gram_tidy_exit;
        }
        if(strcmp(key, start_key) == 0){
            for(size_t i=0; i<linesz; i++){
                data = json_array_get(value, i);
                if(!json_is_string(data)){
                    fprintf(stderr, "Invalid value in '%s' at index %zu\n", start_key, i);
                    goto gram_tidy_exit;
                }
                map[mapbase][i] = strndup(json_string_value(data), max_textlen);
            }
        }else{
            symbolmap_add(key, symidx++);
            for(size_t i=0; i<linesz; i++){
                data = json_array_get(value, i);
                if(!json_is_string(data)){
                    fprintf(stderr, "Invalid value in '%s' at index %zu\n", key, i);
                    goto gram_tidy_exit;
                }
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
gram_tidy_exit:
    json_decref(root);
    free_grammar(map);
    symbolmap_free(symbolmap);
    exit(EXIT_FAILURE);
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

size_t symbolmap_init(symbol ***map){
    *map = calloc(mapsize, sizeof(symbol **));
    if(!map){
        fputs("Insufficient memory for symbolmap.\n", stderr);
        exit(1);
    }
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
    if(!s){
        fputs("Insufficient memory for symbol.\n", stderr);
        exit(1);
    }
    s->symbol = strndup(symtxt, max_symbollen);
    s->index = symidx;
    s->next = NULL;

    uint32_t hash = fnv1a32_hashstr(s->symbol);
    uint8_t bucket = hash & 0xFF;

    symbol **list = &symbolmap[bucket];
    while(*list){
        if(strcmp(symtxt, (*list)->symbol) == 0){
            return;  /* err already in list */
        }
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
    return 0;  /* Only __start__ can have a zero symbol-index */
}

char **extract_symbols(const char *str){
    const char *start = "{";
    const char *end = "}";
    size_t bufsz = 32;
    if(strlen(str) == 0){
        return NULL;
    }
    char *tokstr = strdup(str), *tok = NULL;
    char **tokens = calloc(bufsz, sizeof(char *));
    if(!tokens){
        fputs("Insufficient memory for symbol buffer.\n", stderr);
        exit(1);
    }
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
            char **np = realloc(tokens, bufsz*sizeof(char *));
            if(!np){
                fputs("Insufficient memory to grow symbol buffer.\n", stderr);
                free(tokens);
                exit(1);
            }
            tokens = np;
        }
    }
    if(idx == 0){
        goto tok_tidy_exit;
    }
    tokens[idx++] = NULL;
    /* Shrink the buffer */
    char **np = realloc(tokens, idx*sizeof(char *));
    if(!np){
        fputs("Insufficient memory to shrink symbol buffer.\n", stderr);
        free(tokens);
        exit(1);
    }
    tokens = np;
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
    /*  Replaces symbols in a grammar with the array
     *  offset of their memory locations.
     *
     *  Operators prefixing symbols are identified and
     *  inserted into the grammar to be applied on
     *  extraction.
     */
    char **symbols = extract_symbols(text);
    if(!symbols){
        return strdup(text);
    }
    size_t symidx = 0;
    const char op_delim = ':';
    size_t txtlen = strlen(text);
    char *patched = calloc(txtlen, sizeof(char));
    if(!patched){
        fputs("Insufficient memory for address buffer.\n", stderr);
        exit(1);
    }
    char *pnext = patched;
    char *str = (char *)text;
    /*  Note that symbols are extracted above in
     *  the order and multiplicity they are encountered
     *  in *text. As such, we're certain to encounter
     *  them again here in the same order.
     */
    while(symbols[symidx]){
        char *delim_idx = NULL;
        if((delim_idx = strchr(symbols[symidx], op_delim))){
            char *opname = strndup(symbols[symidx], delim_idx - symbols[symidx]);
            char *symbol = strdup(delim_idx + 1);
            /* Copy str up to symbol into pnext */
            char *delim_opname = malloc((strlen(opname)+2)*sizeof(char));
            snprintf(delim_opname, strlen(opname)+2, "%c%s", '{', opname);
            char *sidx = strstr(str, delim_opname);
            free(delim_opname);
            strncpy(pnext, str, sidx-str);
            pnext += sidx-str;
            int idx = operator_name_to_idx(opname);
            *pnext = (char)(idx+1);
            pnext++;
            /* Increment str up to symbol start */
            str += sidx - str + 1 + strlen(opname);
            str[0] = '{';

            free(opname);
            free(symbols[symidx]);
            symbols[symidx] = symbol;
        }
        if(symbols[symidx][0] == '$'){
            symidx++;
            continue;
        }
        size_t slen = strlen(str);
        size_t symlen = strlen(symbols[symidx]);
        size_t mapidx = symbolmap_getidx(symbols[symidx]);
        (void)mapidx;  /* May be unused */
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

json_t *load_json_file(const char *const filename){
    /*  Returns a json_t* representing a valid json
     *  object defined in the <filename> arg, or NULL on
     *  file access or json parse errors.
     */
    json_error_t json_err = {0};
    json_t *root = json_load_file(filename, 0, &json_err);
    if(!root){
        enum json_error_code err_code = json_error_code(&json_err);
        if(err_code == json_error_cannot_open_file){
            fprintf(stderr, "Error loading grammar from file: %s\n", json_err.text);
        }else{
            fprintf(
                stderr,
                "Error loading grammar from file '%s' line %d, column %d: %s\n",
                filename, json_err.line, json_err.column, json_err.text
            );
        }
        return NULL;
    }
    return root;
}

/* qsort and bsearch in validation */
static int cmp_strcmp(const void *lhs, const void *rhs){
    return strcmp(*(char * const *)lhs, *(char * const *)rhs);
}

#ifndef USE_ASPRINTF
static char *entry_asprintf(const char *fmt, ...){
    /*  An allocating snprintf replacement.
     *  Used by default unless the GNU/BSD extension asprintf
     *  is preferred
     */
    int n = 0;
    size_t size = 0;
    char *p = NULL;
    va_list ap;

    va_start(ap, fmt);
    n = vsnprintf(p, size, fmt, ap);
    va_end(ap);

    if(n<0){  /* glibc < 2.0.6*/
        return NULL;
    }

    size = (size_t)n + 1;
    p = malloc(size);
    va_start(ap, fmt);
    n = vsnprintf(p, size, fmt, ap);
    va_end(ap);

    if(n<0){
        free(p);
        return NULL;
    }
    return p;
}
#endif

char **process_metadata(json_t *metadata){
    /*  Processes metadata and records validation issues.
     *
     *  Only the "min_version" key of metadata is validated
     *  to ensure that the version of textgen required by
     *  the grammar is available.
     *
     *  A NULL-terminated array of strings is returned, or
     *  NULL if no validation issues are raised.
     */
    size_t bufsz = 32, entidx = 0;
    char **entries = calloc(bufsz, sizeof(char *));
    if(!entries){
        fprintf(stderr, "Unable to allocate metadata entry buffer");
        exit(1);
    }

    json_t *minverk = json_object_get(metadata, min_version_key);
    if(!minverk){
        free(entries);
        return NULL;  /* Other metadata is intrinsically 'valid' */
    }
    char *minver = strdup(json_string_value(minverk));
    if(!minver){
#ifdef USE_ASPRINTF
        asprintf(&entries[entidx++], "Invalid %s in metadata", min_version_key);
#else
        entries[entidx++] = entry_asprintf("Invalid %s in metadata", min_version_key);
#endif
        goto shrink_and_ret;
    }
    /* Pseudo quasi partial semver support... */
    int semver[3], idx = 0;
    char *tok = strtok(minver, ".");
    if(!tok){
#ifdef USE_ASPRINTF
        asprintf(&entries[entidx++], "Invalid %s in metadata", min_version_key);
#else
        entries[entidx++] = entry_asprintf("Invalid %s in metadata", min_version_key);
#endif
        goto shrink_and_ret;
    }
    errno = 0;
    char *endptr = NULL;
    semver[idx++] = strtol(tok, &endptr, 10);
    if(errno != 0 ||  // Variously ERANGE on o/uflow or EINVAL
            *endptr || endptr == tok){
        goto report_and_cont;
    }
    while((tok = strtok(NULL, "."))){
        semver[idx++] = strtol(tok, &endptr, 10);
        if(errno != 0 ||
                *endptr || endptr == tok ||
                (semver[0] == 0 && semver[1] == 0 && semver[2] == 0)){
            goto report_and_cont;
        }
    }
    if(semver[0] == 0 && semver[1] == 0 && semver[2] == 0){
        goto report_and_cont;
    }
    goto process_semver;
report_and_cont:
#ifdef USE_ASPRINTF
    asprintf(&entries[entidx++],
         "Metadata %s defines invalid version: %s",
         min_version_key, json_string_value(minverk));
#else
    entries[entidx++] = entry_asprintf("Metadata %s defines invalid version: %s",
                                       min_version_key, json_string_value(minverk));
#endif
    goto shrink_and_ret;
process_semver:
    if(semver[0] > PROJECT_MAJOR ||
        (semver[0] == PROJECT_MAJOR && semver[1] > PROJECT_MINOR) ||
        (semver[0] == PROJECT_MAJOR && (semver[1] == PROJECT_MINOR) && (semver[2] > PROJECT_MICRO))){
#ifdef USE_ASPRINTF
        asprintf(&entries[entidx++],
           "Grammar requires version %s but textgen is %s",
            json_string_value(minverk), PROJECT_VERSION);
#else
        entries[entidx++] = entry_asprintf("Grammar requires version %s but textgen is %s",
                                           json_string_value(minverk), PROJECT_VERSION);
#endif
    }
shrink_and_ret:
    free(minver);
    if(entidx == 0){
        free(entries);
        entries = NULL;
    }else{
        entries[entidx++] = NULL;
        /* Shrink the entry buffer for return*/
        char **np = realloc(entries, entidx*sizeof(char *));
        if(!np){
            fputs("Insufficient memory to shrink metadata entries buffer.\n", stderr);
            free(entries);
            exit(1);
        }
        entries = np;
    }
    return entries;
}


char **lexical_validate_grammar(json_t *json){
    /*  This function performs a lexical validation of its json
     *  argument; that is, it considers the grammar specified in the
     *  file at the level of strings, without constructing a grammar
     *  instance or the symbol hash table. This duplicates some costs
     *  and saves on others, but importantly allows the validation to
     *  be run independently of grammar construction.
     *
     *  It checks that:
     *   - Grammars are valid json, implied by load_json_file()
     *   - A valid __start__ symbol is defined
     *   - Every symbol used in a grammar has a definition
     *   - Every operator used in a grammar has a definition
     *   - TBA The total number of symbols does not exceed a limit
     *   - TBA The number of alternates per symbols does not exceed a limit
     *
     *   Separate validation of any metadata min_version is
     *   performed by process_metadata.
     *
     *   A null-terminated array of strings is returned, or NULL
     *   if the grammar validates correctly.
     */
    size_t key_count = json_object_size(json);
    if(key_count == 0){
        return NULL;
    }
    char **keys = malloc(key_count * sizeof(char *));
    if(!keys){
        fprintf(stderr, "Unable to allocate for %zu keys\n", key_count);
        exit(1);
    }
    size_t bufsz = 32, entidx = 0;
    char **entries = calloc(bufsz, sizeof(char *));
    if(!entries){
        fprintf(stderr, "Unable to allocate entry buffer\n");
        for(size_t idx=0; idx<key_count; idx++){
            free(keys[idx]);
        }
        free(keys);
        exit(1);
    }

    const char *key = NULL;
    json_t *value = NULL;
    size_t idx = 0;

    json_object_foreach(json, key, value){
        keys[idx++] = strdup((char *)key);  /* keys freed prior to return */
    }

    qsort(keys, key_count, sizeof(char *), cmp_strcmp);

    if(!bsearch(&start_key, keys, key_count, sizeof(char *), cmp_strcmp)){
#ifdef USE_ASPRINTF
        asprintf(&entries[entidx++], "Grammar has no entrypoint. Please define a %s key.", start_key);
#else
        entries[entidx++] = entry_asprintf("Grammar has no entrypoint. Please define a %s key.", start_key);
#endif
    }
    const char op_delim = ':';

    json_object_foreach(json, key, value){
        if(!strcmp(key, metadata_key)){
            char **md_entries = process_metadata(value);
            if(md_entries){
                for(size_t idx=0; md_entries[idx]; idx++){
                    entries[entidx++] = md_entries[idx];
                    /* Handle bufsz */
                }
                free(md_entries);
            }
            continue;  /* No further processing of metadata */
        }
        if(!json_is_array(value)){
#ifdef USE_ASPRINTF
            asprintf(&entries[entidx++], "Invalid definition of '%s' value must be an array, not: %s",
                    key, json_string_value(value));
#else
            entries[entidx++] = entry_asprintf("Invalid definition of '%s' value must be an array, not: %s",
                    key, json_string_value(value));
#endif
            continue;
        }
        size_t linesz = json_array_size(value);
        json_t *data = NULL;
        for(size_t i=0; i<linesz; i++){
            data = json_array_get(value, i);
            if(!json_is_string(data)){
#ifdef USE_ASPRINTF
                asprintf(&entries[entidx++], "Invalid value in '%s' at index %zu", key, i);
#else
                entries[entidx++] = entry_asprintf("Invalid value in '%s' at index %zu", key, i);
#endif
                continue;
            }
            char **symbols = extract_symbols(json_string_value(data));
            if(!symbols){
                continue;
            }
            size_t symidx = 0;

            while(symbols[symidx]){
                char *symbol = NULL;
                char *delim = NULL, *sym_txt = NULL, *op_txt = NULL;
                /* Detect {op:sym} constructs and parse */
                if((delim = strchr(symbols[symidx], op_delim))){
                    size_t symlen = strlen(symbols[symidx]);
                    *delim = 0;  /* Null-terminate what becomes op_txt */
                    op_txt = symbols[symidx];
                    /* The symbol itself is a new alloc... */
                    sym_txt = strndup(delim+1, symlen-(delim-op_txt));
                    int op_idx = operator_name_to_idx(op_txt);
                    if(op_idx == -1){
#ifdef USE_ASPRINTF
                        asprintf(&entries[entidx++], "Invalid operator: %s used in {%s}", op_txt, key);
#else
                        entries[entidx++] = entry_asprintf("Invalid operator: %s used in {%s}", op_txt, key);
#endif
                    }
                    /* ...which replaces the composite {op:sym} */
                    symbol = sym_txt;
                }else{
                    symbol = symbols[symidx];
                }
                if(!(symbol[0] == '$')){
                    if(!bsearch(&symbol, keys, key_count, sizeof(char *), cmp_strcmp)){
#ifdef USE_ASPRINTF
                        asprintf(&entries[entidx++], "Undefined symbol {%s} used in {%s}", symbol, key);
#else
                        entries[entidx++] = entry_asprintf("Undefined symbol {%s} used in {%s}", symbol, key);
#endif
                    }
                }
                if(sym_txt){
                    free(sym_txt);  /* Otherwise freed with symbols prior to return */
                }
                if(entidx >= bufsz-5){
                    bufsz *= 2;
                    char **nent = realloc(entries, bufsz*sizeof(char *));
                    if(!nent){
                        fputs("Insufficient memory to grow validation entry buffer.", stderr);
                        free(entries);
                        exit(1);
                    }
                    entries = nent;
                }
                symidx++;
            }
            for(size_t idx=0; symbols[idx]; idx++){
                free(symbols[idx]);
            }
            free(symbols);
        }
    }
    for(size_t idx=0; idx<key_count; idx++){
        free(keys[idx]);
    }
    free(keys);

    if(entidx == 0){
        free(entries);
        entries = NULL;
    }else{
        entries[entidx++] = NULL;
        /* Shrink the entry buffer for return*/
        char **np = realloc(entries, entidx*sizeof(char *));
        if(!np){
            fputs("Insufficient memory to shrink validation entries buffer.\n", stderr);
            free(entries);
            exit(1);
        }
        entries = np;
    }
    return entries;
}
