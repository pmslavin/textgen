#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "operators.h"
#include "randfunc_ops.h"  /* rand_func */

unsigned seq_state = 0;

char *opfunc_Capitalise(char *text){
    text[0] = toupper(text[0]);
    return text;
}

char *opfunc_Article(char *text){
    const char *vowels = "aeiou";
    char init = tolower(text[0]);
    const char *art = (strchr(vowels, init)) ? "an " : "a ";
    char *joined = malloc(sizeof(char) * (strlen(art) + strlen(text) + 1));
    strcpy(joined, art);
    strcpy(joined+strlen(art), text);
    return joined;
}

char *opfunc_Range(char *text){
    char *init_text = strdup(text);  /* For err reports */
    ++text;  /* Advance past initial '$' */
    int start = 0, end = 0;
    char *endptr = NULL;
    char *tok = strtok(text, ",");
    errno = 0;
    start = strtol(tok, &endptr, 10);
    if(errno != 0 || *endptr || endptr == tok){
        fprintf(stderr, "Invalid argument to range operator: %s\n", init_text);
        free(init_text);
        return NULL;
    }
    errno = 0;
    tok = strtok(NULL, ",");
    end = strtol(tok, &endptr, 10);
    if(errno != 0 || *endptr || endptr == tok){
        fprintf(stderr, "Invalid argument to range operator: %s\n", init_text);
        free(init_text);
        return NULL;
    }
    free(init_text);  /* No longer required in subsequent err prints */
    if(start == end){
        fprintf(stderr, "Range must have non-zero length, not $%d,%d\n", end, end);
        return NULL;
    }
    if(start > end){
        int temp = end;
        end = start;
        start = temp;
    }
    int v = start + rand_func() % (end - start);
    /* Allocate sufficient space for sign digit width and null term */
    int d = 0;
    for(int b=1; v/b; b*=10, d++);
    if(v == 0) ++d;
    int has_sign = v<0 ? 1 : 0;
    char *out = malloc((d+has_sign+1)*sizeof(char));
    if(!out){
        fprintf(stderr, "Unable to allocate size %d for Range operator\n", d+has_sign+1);
        return NULL;
    }
    snprintf(out, d+has_sign+1, "%d", v);
    return out;
}

char *opfunc_Date(char *text){

    static int mdays[2][13] = {
        {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };
    int sy=0, sm=0, sd=0, ey=0, em=0, ed=0;
    int count = sscanf(text, "$%d-%d-%d,%d-%d-%d", &sy, &sm, &sd, &ey, &em, &ed);
    if(count != 6){
        fprintf(stderr, "Invalid argument to Date operator: <%s>\n", text);
        return NULL;
    }

    int y = sy + rand_func() % (ey - sy + 1);
    int m;
    if(y == sy && sy == ey){
        m = sm + rand_func() % (em - sm);
    }else if(y == sy){
        m = sm + rand_func() % (12 - sm);
    }else if(y == ey){
        m = 1 + rand_func() % em;
    }else{
        m = 1 + rand_func() % 12;
    }

    int is_leap = (!(y%4) && y%100) || !(y%400) ? 1 : 0;
    int d;
    if(y == ey && sy == ey && sm == em){
        d = sd + rand_func() % (ed - sd);
    }else if(y == sy && m == sm){
        d = sd + rand_func() % (1 + mdays[is_leap][m] - sd);
    }else if(y == ey && m == em){
        d = 1 + rand_func() % ed;
    }else{
        d = 1 + rand_func() % mdays[is_leap][m];
    }

    int ys=0, ms=0, ds=0, b;
    for(b=1; y/b; ys++, b*=10);
    for(b=1; m/b; ms++, b*=10);
    for(b=1; d/b; ds++, b*=10);
    char *out = malloc((ys+ms+ds+3)*sizeof(char));
    if(!out){
        fprintf(stderr, "Unable to allocate size %d for Date operator\n", ys+ms+ds+3);
        return NULL;
    }
    snprintf(out, ys+ms+ds+3, "%d-%d-%d", y, m, d);

    return out;
}


char *opfunc_Seq(char *fmt){
    int width = -1;
    char hex = '!';
    int count = sscanf(fmt, "$%d%c", &width, &hex);
    if(count < 1 || width < 1){
        fprintf(stderr, "Invalid argument to seq operator: %s\n", fmt);
        return NULL;
    }

    int mod = 1, wc = width;
    for(; wc>0; wc--, mod*=10);

    char *out = malloc((width+1)*sizeof(char));
    if(!out){
        fprintf(stderr, "Unable to allocate size %d for seq operator\n", width+1);
        return NULL;
    }
    char *outfmt = NULL;
    if(hex == 'x'){
        outfmt = "%0*x";
    }else if(hex == 'X'){
        outfmt = "%0*X";
    }else{
        outfmt = "%0*d";
    }

    snprintf(out, width+1, outfmt, width, seq_state++ % mod);
    return out;
}


Operator operators[] = {
    {"capitalise", opfunc_Capitalise},
    {"article", opfunc_Article},
    {"range", opfunc_Range},
    {"date", opfunc_Date},
    {"seq", opfunc_Seq}
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
