#ifndef _RANDFUNCS_H_
#define _RANDFUNCS_H_

/* constant */
#define rg_constant_name "constant"
#define rg_constant_seedfmt "%x"
#define rg_constant_desc "Returns a constant value determined by its seed."
extern int rg_constant_state;
int rg_constant(void);
int rg_constant_seedfn(const char *, const char *);

/* cycle */
#define rg_cycle_name "cycle"
#define rg_cycle_seedfmt "%x:%x"
#define rg_cycle_desc "Repeatedly cycles from start_seed to end_seed-1."
extern unsigned rg_cycle_start;
extern unsigned rg_cycle_mod;
int rg_cycle(void);
int rg_cycle_seedfn(const char *, const char *);

/* rand */
#define rg_rand_name "rand"
#define rg_rand_seedfmt "%x"
#define rg_rand_desc "The stdlib rand() function, initialised by srand()."
int rg_rand_seedfn(const char *, const char *);

/* pisano16 */
#define rg_pisano16_name "pisano16"
#define rg_pisano16_seedfmt "%x:%x:%x"
#define rg_pisano16_desc "A 16-bit Pisano period generalised Fibonacci generator"
extern unsigned rg_pisano16_s0;
extern unsigned rg_pisano16_s1;
extern unsigned rg_pisano16_s2;
int rg_pisano16(void);
int rg_pisano16_seedfn(const char *, const char *);

#endif
