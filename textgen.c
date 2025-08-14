#define _POSIX_C_SOURCE 200809L  // strnlen
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grammar.h"
#include "options.h"
#include "operators.h"
#include "textgen.h"


extern symbol **symbolmap;
unsigned maptop = 0;

const char *textgen(const char *src, const char ***grammar){
    size_t idx = -1;
    size_t outidx = 0;
    size_t block_sz = 32;
    Operator *op = NULL;
    const char* alloc_errtxt = "Insufficient memory for output.\n";
    char *output = calloc(block_sz, sizeof(char));
    if(!output){
        fputs(alloc_errtxt, stderr);
        exit(1);
    }

    while(1){
        unsigned char c = src[++idx];
        if(!c){
            break;
        }
        if(c < optop){
            --c;  /* Stored opidx is +1 to avoid NULL */
            op = &(operators[c]);
            continue;
        }else if(c < mapbase){
            output[outidx++] = c;
            if(outidx == block_sz){
                block_sz *= 2;
                char *np = realloc(output, block_sz);
                if(!np){
                    fputs(alloc_errtxt, stderr);
                    free(output);
                    exit(1);
                }
                output = np;
            }
            continue;
        }else if(c < maptop){
            size_t c_len = linelen(grammar[c]);
            unsigned lineidx = rand() % c_len;
            char *out = (char *)textgen(grammar[(unsigned)c][lineidx], grammar);
            if(op){
                char *op_in = strdup(out);
                free(out);
                char *op_out = (*op->func)(op_in);
                op = NULL;
                out = op_out;
            }
            size_t out_sz = strnlen(out, max_textlen);
            if(outidx + out_sz >= block_sz){
                block_sz = outidx + out_sz <= 2*block_sz ? 2*block_sz + 1 : outidx + out_sz + 1;
                char *np = realloc(output, block_sz);
                if(!np){
                    fputs(alloc_errtxt, stderr);
                    free(output);
                    exit(1);
                }
                output = np;
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

    char *inputfn = NULL;
    unsigned seed = 0xff;
    unsigned count = 1;
    int opt, optidx = 0;

    char *pmname;
	(pmname = strrchr(argv[0], '/')) ? pmname++ : (pmname = argv[0]);
    if(argc == 1){
        full_usage(stderr, pmname, long_options, option_descs, EXIT_FAILURE);
	}

    while((opt = getopt_long(argc, argv, optstring, long_options, &optidx)) != -1){
        switch(opt){
            case 'g':
                inputfn = optarg;
                break;
            case 'r':
                seed = atoi(optarg);
                break;
            case 'n':
                count = atoi(optarg) > 0 ? atoi(optarg) : 1;
                break;
            case 'h':
                full_usage(stdout, pmname, long_options, option_descs, EXIT_SUCCESS);
                break;
            default:
                full_usage(stderr, pmname, long_options, option_descs, EXIT_FAILURE);
                break;
        }
    }

    symbolmap_init(&symbolmap);
    const char ***grammar = build_grammar(inputfn);
    set_maptop(grammar);
    //print_grammar(grammar);

    srand(seed);
    for(unsigned i=0; i<count; i++){
        const char *output = textgen("\x80.", grammar);
        puts(output);
        free((char *)output);
    }

    free_grammar(grammar);
    symbolmap_free(symbolmap);
    return 0;
}
