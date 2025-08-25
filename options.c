#include "config.h"
#include "options.h"


const char *optstring = "g:v:S:s:n:G:Lh";

struct option long_options[] = {
    {"grammar-file",     required_argument, NULL, 'g'},
    {"validate-grammar", required_argument, NULL, 'v'},
    {"line-separator",   required_argument, NULL, 'S'},
    {"random-seed",      required_argument, NULL, 's'},
    {"number",           required_argument, NULL, 'n'},
    {"random-generator", required_argument, NULL, 'G'},
    {"list-generators",  no_argument,       NULL, 'L'},
    {"help",             no_argument,       NULL, 'h'},
    {NULL, 0, NULL, 0 }
};


const char *option_descs[][2] = {
   {"<filename>", "The grammar file to load"},
   {"<filename>", "The grammar file to validate"},
   {"<character>", "A character used to separate output lines"},
   {"<seed>", "A seed to initialise the random number generator"},
   {"<number>", "The number of texts to generate"},
   {"<generator>", "The random generator to use"},
   {"", "List the available random generators"},
   {"", "Print this help"}
};


void full_usage(FILE *fp, const char *pname, const struct option *longopts, const char *descs[][2], int exitcode){
    fprintf(fp, "%s, version %s\n", pname, PROJECT_VERSION);
    fprintf(fp, "Usage: %s [OPTIONS]\n", pname);
    for(size_t i=0; longopts[i].name ;i++){
        fprintf(fp, "    --%-18s-%-2c%-14s%-32s\n",
                longopts[i].name, longopts[i].val, descs[i][0], descs[i][1]);
    }
    exit(exitcode);
}

