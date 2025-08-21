#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "randfuncs.h"
#include "randfunc_ops.h"


Random_Generator rand_generators[] = {
  {rg_rand_name, rand, rg_rand_seedfmt, rg_rand_seedfn, rg_rand_desc},
  {rg_constant_name, rg_constant, rg_constant_seedfmt, rg_constant_seedfn, rg_constant_desc},
  {rg_cycle_name, rg_cycle, rg_cycle_seedfmt, rg_cycle_seedfn, rg_cycle_desc},
  {rg_pisano16_name, rg_pisano16, rg_pisano16_seedfmt, rg_pisano16_seedfn, rg_pisano16_desc}
};

Rand_func rand_func = rand;
char *rand_func_name = "rand";

int randgen_name_to_idx(const char *name){
    size_t nrand = sizeof(rand_generators)/sizeof(rand_generators[0]);
    for(size_t idx=0; idx<nrand; idx++){
        if(!strcmp(name, rand_generators[idx].name)){
            return idx;
        }
    }
    return -1;
}

Random_Generator *get_randgen_by_name(const char *name){
    int idx = randgen_name_to_idx(name);
    if(idx == -1){
        return NULL;
    }
    return &rand_generators[idx];
}

int set_random_func(const char *name){
    int ret = -1;
    Random_Generator *rg = get_randgen_by_name(name);
    if(rg){
       rand_func_name = (char *)name;
       rand_func = rg->func;
       ret = 0;
    }
    return ret;
}

int seed_random_func(const char *optarg){
    Random_Generator *rg = get_randgen_by_name(rand_func_name);
    return rg->seedfunc(rg->seedfmt, optarg);
}


char **list_random_generators(void){
    const char *hname = "NAME";
    const char *hfmt = "SEED";
    const char *hdesc = "DESCRIPTION";
    size_t nrand = sizeof(rand_generators)/sizeof(rand_generators[0]);
    char **gen_names = calloc(nrand + 2, sizeof(char *));  /* Includes header line and NULL term*/
    size_t idx, max_name_len = 0, max_seedfmt_len = 0, wspad = 4;
    size_t namelen = 0, desclen = 0, fmtlen = 0;

    /* First calculate required column widths */
    for(idx=0; idx<nrand; idx++){
        namelen = strlen(rand_generators[idx].name);
        fmtlen = strlen(rand_generators[idx].seedfmt);
        max_name_len = namelen > max_name_len ? namelen : max_name_len;
        max_seedfmt_len = fmtlen > max_seedfmt_len ? fmtlen : max_seedfmt_len;
    }
    /* Make header line */
    char *h0 = malloc((max_name_len + max_seedfmt_len + 2*wspad + strlen(hdesc) + 1) * sizeof(char));
    sprintf(h0, "%-*s%-*s%s", (int)(max_name_len + wspad), hname, (int)(max_seedfmt_len + wspad), hfmt, hdesc);
    gen_names[0] = h0;

    /* Make padded columns */
    for(idx=0; idx<nrand; idx++){
        namelen = strlen(rand_generators[idx].name);
        desclen = strlen(rand_generators[idx].desc);
        fmtlen = strlen(rand_generators[idx].seedfmt);
        char *s = malloc((max_name_len + max_seedfmt_len + 2*wspad + desclen + 1) * sizeof(char));
        sprintf(s, "%-*s%-*s%s",
                (int)(max_name_len + wspad),
                rand_generators[idx].name,
                (int)(max_seedfmt_len + wspad),
                rand_generators[idx].seedfmt,
                rand_generators[idx].desc);
        gen_names[idx+1] = s;  /* Begin insertions after header row */
    }
    gen_names[idx+1] = NULL;  /* NULL-terminate name list */
    return gen_names;
}
