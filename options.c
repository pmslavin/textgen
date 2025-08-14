#include "config.h"
#include "options.h"


const char *optstring = "g:r:n:h";

struct option long_options[] = {
    {"grammar-file", required_argument, NULL, 'g'},
    {"random-seed",  required_argument, NULL, 'r'},
    {"number",       required_argument, NULL, 'n'},
    {"help",         no_argument,       NULL, 'h'},
    {NULL, 0, NULL, 0 }
};


const char *option_descs[][2] = {
   {"<filename>", "The grammar file to load"},
   {"<seed>", "A seed to initialise the random number generator"},
   {"<number>", "The number of texts to generate"},
   {"", "Print this help"}
};


void full_usage(FILE *fp, const char *pname, const struct option *longopts, const char *descs[][2], int exitcode){
    fprintf(fp, "%s, version %s\n", pname, PROJECT_VERSION);
    fprintf(fp, "Usage: %s [OPTIONS]\n", pname);
    for(size_t i=0; longopts[i].name ;i++){
        fprintf(fp, "    --%-16s-%-2c%-12s%-32s\n",
                longopts[i].name, longopts[i].val, descs[i][0], descs[i][1]);
    }
    exit(exitcode);
}

