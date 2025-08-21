/*  Definitions of random number generators.
 *
 *  Each generator must provide:
 *    -  An initialiser-constant name
 *    -  An int (*)(void) generator function
 *    -  An int (*)(const char *, const char *) seed function
 *    -  An initialiser-constant seedfmt string
 *    -  Any state storage required by the generator
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "randfuncs.h"

/*  Random generator 'constant'
 *
 *  Returns a constant value determined by its seed.
 */

int rg_constant_state = 0;

int rg_constant(void){
    return rg_constant_state;
}

int rg_constant_seedfn(const char *fmt, const char *args){
    int ret = sscanf(args, fmt, &rg_constant_state);
    if(ret != 1){
        fprintf(stderr, "Invalid seed format for 'constant': '%s' "
                "Specify seed as: '%s'\n", args, fmt);
        exit(EXIT_FAILURE);
    }
    return ret;
}

/*  Random generator 'cycle'
 *
 *  Repeatedly cycles from start_seed to end_seed-1.
 */

unsigned rg_cycle_start = 0, rg_cycle_mod = 8;
int rg_cycle(void){
    int ret = rg_cycle_start % rg_cycle_mod;
    //printf("cycle_start: %u  cycle_mod: %u  ret: %d\n", cycle_start, cycle_mod, ret);
    rg_cycle_start++;
    return ret;
}

int rg_cycle_seedfn(const char *fmt, const char *args){
    int ret = sscanf(args, fmt, &rg_cycle_start, &rg_cycle_mod);
    if(ret != 2){
        fprintf(stderr, "Invalid seed format for 'cycle': '%s' "
                "Specify seed as: '%s'\n", args, fmt);
        exit(EXIT_FAILURE);
    }
    return ret;
}

/*  Random generator 'rand'
 *
 *  This is a wrapper for stdlib's rand and passes
 *  seeds through to srand. As such, no separate
 *  rand func or state are required.
 */
int rg_rand_seedfn(const char *fmt, const char *args){
    unsigned seed = 1;
    int ret = sscanf(args, fmt, &seed);
    if(ret != 1){
        fprintf(stderr, "Invalid seed format for 'rand': '%s' "
                "Specify seed as: '%s'\n", args, fmt);
        exit(EXIT_FAILURE);
    }
    srand(seed);
    return ret;
}

/*  Random generator 'pisano16'
 */
unsigned rg_pisano16_w0 = 0x5A4A;
unsigned rg_pisano16_w1 = 0x0248;
unsigned rg_pisano16_w2 = 0xB753;
int rg_pisano16(void){
    unsigned wtemp = rg_pisano16_w0 + rg_pisano16_w1 + rg_pisano16_w2;
    wtemp &= (1 << 16) - 1;
    rg_pisano16_w0 = rg_pisano16_w1;
    rg_pisano16_w1 = rg_pisano16_w2;
    rg_pisano16_w2 = wtemp;

    return rg_pisano16_w2 >> 8;
}
int rg_pisano16_seedfn(const char *fmt, const char *args){
    int ret = sscanf(args, fmt, &rg_pisano16_w0, &rg_pisano16_w1, &rg_pisano16_w2);
    if(ret != 3){
        fprintf(stderr, "Invalid seed format for 'pisano16': '%s' "
                "Specify seed as: '%s'\n", args, fmt);
        exit(EXIT_FAILURE);
    }
    return ret;
}
