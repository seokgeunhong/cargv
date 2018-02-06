
#include "cline.h"
#include <stdio.h>
#include <string.h>


enum cline_section {
    CLINE_CMD           = 0,
    CLINE_OPTION,
    CLINE_NON_OPTION,
};


enum cline_result
cline_init(
    struct cline_parser *parser,
    int optc, const struct cline_opt *opts,
    int argc, char *argv[])
{
    if (!(parser && ((optc > 0 && opts) || optc == 0)
            && ((argc > 0 && argv) || argc == 0)))
        return CLINE_ERR_PARAM;

    parser->opts = opts;
    parser->optend = opts + optc;
    parser->args = argv;
    parser->argend = argv + argc;
    parser->section = CLINE_CMD;
    return CLINE_OK;
}

enum cline_result
cline_read(
    struct cline_parser *parser,
    const struct cline_opt **opt,
    const char **name,
    const char **val)
{
    const struct cline_opt *o;
    const char *n;

    if (!(parser && opt && name))
        return CLINE_ERR_PARAM;

    *opt = NULL;
    *name = "";
    if (val)
        *val = "";

    if (parser->section == CLINE_CMD) {
        parser->args++;
        parser->section = CLINE_OPTION;
    }

    if (!(parser->args < parser->argend))
        return CLINE_DONE;

    if (parser->section == CLINE_OPTION) {
        if (strcmp(*parser->args, "--") == 0) { /* section separator */
            parser->args++;                     /* from next, */
            parser->section = CLINE_NON_OPTION; /* non-option arguments come */
            return CLINE_DONE;
        }
        if ((*parser->args)[0] != '-') {        /* should begin with '-' */
            parser->section = CLINE_NON_OPTION; /* or non-option arguments */
            return CLINE_DONE;
        }
        for (o = parser->opts; o < parser->optend; o++) {   /* foreach opts */
            for (n = o->names; *n; n = n + strlen(n) + 1) { /* foreach names */
                if (strcmp(*parser->args, n) == 0) {
                    parser->args++;
                    *opt = o;
                    *name = n;
                    if (o->val > 0) {
                        if (!(parser->args < parser->argend))
                            return CLINE_ERR_VAL_MISSING;
                        if (!val)
                            return CLINE_ERR_PARAM;

                        *val = *parser->args;
                        parser->args++;
                    }
                    else {
                        *val = "";
                    }
                    return CLINE_OK;
                }
            }
        }
        return CLINE_ERR_BAD_OPTION;
    }
    else if (parser->section == CLINE_NON_OPTION) {
        if (!val)
            return CLINE_ERR_PARAM;

        *name = "";
        *val = *parser->args;
        parser->args++;
        return CLINE_OK;
    }
    else {
        return CLINE_ERR_INTERNAL;
    }
}
