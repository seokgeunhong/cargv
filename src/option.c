#include "sunriset_option.h"
#include "sunriset_version.h"

#include "cline.h"
#include <stdio.h>
#include <memory.h>
#include <string.h>


void print_usage()
{
    static const char *usage[] = {
"",
"Calculate time of sunrise and sunset, and length of day of a date",
"at a position on earth.",
"",
"OPTIONS:",
"",
"  --version         Print version info. and exit.",
"  -h --help         Print this help message and exit.",
"",
"Original code snippet was written by Paul Schlyter<pausch@stjarnhimlen.se>,",
"all right reserved, 1989, 1992. And can be found at:",
"http://stjarnhimlen.se/comp/sunriset.c",
"",
0
    };
    const char **s;

    fprintf(stderr, "Usage: %s [OPTIONS] [--] [ARGS]\n", SUNRISET_NAME);
    for (s = usage; *s; s++)
        fprintf(stderr, "%s\n", *s);
}

int parse_args(struct sunriset_option_t *option, int argc, char *argv[])
{
    static const struct cline_opt _opts[] = {
        {"--help\0-h\0",       0},
        {"--version\0",        0},
        {"--verbose\0-v\0",    0},
        {"--longitude\0-x\0",  1},
        {"--latitude\0-y\0",   1},
        {"--altitude\0-z\0",   1},
    };

    struct cline_parser parser;
    int err;
    const struct cline_opt *opt;
    const char *name;
    const char *val;

    memset(option, 0, sizeof(*option));

    if (cline_init(&parser, sizeof(_opts)/sizeof(_opts[0]), _opts, argc, argv))
        return 1;

    while ((err = cline_read(&parser, &opt, &name, &val)) == CLINE_OK) {
        if (strcmp(opt->names, "--help") == 0)
            option->help = 1;
        else if (strcmp(opt->names, "--version") == 0)
            option->version = 1;
        else if (strcmp(opt->names, "--verbose") == 0)
            option->verbose = 1;
        else if (strcmp(opt->names, "--longitude") == 0)
            option->longitude = val;
        else if (strcmp(opt->names, "--latitude") == 0)
            option->latitude = val;
        else if (strcmp(opt->names, "--altitude") == 0)
            option->altitude = val;
        else {
            fprintf(stderr,
                "FATAL: parsed, but not recognized option `%s`\n", name);
            return CLINE_ERR_BAD_OPTION;
        }
    }
    if (err != CLINE_DONE) {
        fprintf(stderr, "Error: %d\n", err);
        return err;
    }
    return 0;
}
