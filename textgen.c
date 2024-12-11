#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grammar.h"
#include "textgen.h"

extern symbol **symbolmap;

const char *map[255][linewidth] = {
    [0x80] = {"\x81 are \x82 that \x83 are \x84"}
};

const unsigned maptop = 0x86;

size_t linelen(const char **line){
    size_t len = 0;
    while(line[len++]){
        if(len == linewidth){
            return linewidth;
        }
    }
    return len-1;
}

const char *textgen(const char *src){
    size_t idx = -1;
    size_t outidx = 0;
    size_t block_sz = 32;
    char *output = calloc(block_sz, sizeof(char));

    while(1){
        unsigned char c = src[++idx];
        if(!c){
            break;
        }
        if(c < 0x80){
            output[outidx++] = c;
            if(outidx == block_sz){
                block_sz *= 2;
                output = realloc(output, block_sz);
            }
            continue;
        }else if(c < maptop){
            size_t c_len = linelen(map[c]);
            unsigned lineidx = rand() % c_len;
            const char *out = textgen(map[(unsigned)c][lineidx]);
            size_t out_sz = strlen(out);
            if(outidx + out_sz >= block_sz){
                //printf("block_sz: %lu  out_sz: %lu outidx: %lu ", block_sz, out_sz, outidx);
                block_sz = outidx + out_sz <= 2*block_sz ? 2*block_sz + 1 : outidx + out_sz + 1;
                //printf("new block_sz: %lu\n", block_sz);
                output = realloc(output, block_sz);
            }
            strncpy(output+outidx, out, out_sz);
            outidx += out_sz;
            free((char *)out);
            continue;
        }
    }
    output[outidx] = '\0';
    return output;
}

void print_grammar(const char ***grammar){
    unsigned mapbase = 0x80;
    size_t c_len;
    for(unsigned mapidx=mapbase; mapidx<255; mapidx++){
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

int main(int argc, char *argv[]){

    const char *inputfn;

    if(argc >= 2){
        inputfn = argv[1];
    }else{
        fputs("Input grammar required.\n", stderr);
        exit(1);
    }

    // WIP tests
    const char ***grammar = build_grammar(inputfn);
    print_grammar(grammar);
    char **tokens = extract_tokens("$(people) are $(request) that $(item) are $(rule)");
    if(!tokens){
        ; // string contains no tokens
    }
    free_grammar(grammar);

    symbolmap_init(&symbolmap);
    symbolmap_add("ocelots", 77);
    symbolmap_add("ocelots", 88);
    size_t sidx = symbolmap_getidx("ocelots");
    printf("sidx: %lu\n", sidx);

    srand(0xff);
    unsigned count = 0;
    if(argc > 2){
        count = atoi(argv[2]);
    }else{
        count = 1;
    }
    for(unsigned i=0; i<count; i++){
        const char *output = textgen("\x80.");
        puts(output);
        free((char *)output);
    }

    return 0;
}
