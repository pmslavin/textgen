#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>


extern struct option long_options[];
extern const char *option_descs[][2], *optstring;

void full_usage(FILE *, const char *, const struct option *, const char *[][2], int);

#endif
