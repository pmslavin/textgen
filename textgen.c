#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grammar.h"
#include "textgen.h"


extern symbol **symbolmap;
unsigned maptop = 0;

const char *textgen(const char *src, const char ***grammar){
    size_t idx = -1;
    size_t outidx = 0;
    size_t block_sz = 32;
    char *output = calloc(block_sz, sizeof(char));

    while(1){
        unsigned char c = src[++idx];
        if(!c){
            break;
        }
        if(c < mapbase){
            output[outidx++] = c;
            if(outidx == block_sz){
                block_sz *= 2;
                output = realloc(output, block_sz);
            }
            continue;
        }else if(c < maptop){
            size_t c_len = linelen(grammar[c]);
            unsigned lineidx = rand() % c_len;
            const char *out = textgen(grammar[(unsigned)c][lineidx], grammar);
            size_t out_sz = strlen(out);
            if(outidx + out_sz >= block_sz){
                block_sz = outidx + out_sz <= 2*block_sz ? 2*block_sz + 1 : outidx + out_sz + 1;
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

void set_maptop(const char ***grammar){
    size_t mapidx = mapbase;
    while(*grammar[mapidx++]);
    maptop = mapidx-1;
}


int main(int argc, char *argv[]){

    const char *inputfn;

    if(argc >= 2){
        inputfn = argv[1];
    }else{
        fputs("Input grammar required.\n", stderr);
        exit(1);
    }

    symbolmap_init(&symbolmap);
    const char ***grammar = build_grammar(inputfn);
    set_maptop(grammar);
    //print_grammar(grammar);

    srand(0xff);
    unsigned count = 0;
    if(argc > 2){
        count = atoi(argv[2]);
    }else{
        count = 1;
    }
    for(unsigned i=0; i<count; i++){
        const char *output = textgen("\x80.", grammar);
        puts(output);
        free((char *)output);
    }

    free_grammar(grammar);
    symbolmap_free(symbolmap);
    return 0;
}
