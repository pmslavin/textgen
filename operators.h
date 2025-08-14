#ifndef _OPERATORS_H_
#define _OPERATORS_H_

typedef char * (*Operator_func)(char *);

typedef struct _operator{
    const char *name;
    Operator_func func;
}Operator;

extern Operator operators[];

int operator_name_to_idx(const char *);
Operator *get_operator_by_name(const char *);

#endif
