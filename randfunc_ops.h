#ifndef _RANDFUNC_OPS_H_
#define _RANDFUNC_OPS_H_

typedef int (*Rand_func)(void);

typedef struct _randgen{
    const char *name;
    Rand_func func;
    const char *seedfmt;
    int (*seedfunc)(const char *, const char *);
    const char *desc;
}Random_Generator;

extern Random_Generator random_generators[];
extern Rand_func rand_func;

int randgen_name_to_idx(const char *);
Random_Generator *get_randgen_by_name(const char *);
int set_random_func(const char *);
int seed_random_func(const char *);
char **list_random_generators(void);

#endif
