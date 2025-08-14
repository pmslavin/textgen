#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "operators.h"


char *opfunc_Capitalise(char *text){
    text[0] = toupper(text[0]);
    return text;
}

Operator operators[] = {
    {"capitalise", opfunc_Capitalise}
};

int operator_name_to_idx(const char *name){
    for(size_t idx=0; idx < sizeof(operators)/sizeof(Operator); idx++){
        if(!strcmp(operators[idx].name, name)){
            return idx;
        }
    }
    return -1;
}

Operator *get_operator_by_name(const char *name){
    int idx = operator_name_to_idx(name);
    if(idx == -1){
        return NULL;
    }

    return &(operators[idx]);
}
