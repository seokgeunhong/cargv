#ifndef __cline_h__
#define __cline_h__


enum cline_result {
    CLINE_OK    = 0,
    CLINE_DONE,     /* no more arguments */

    CLINE_ERR_PARAM     = -1,       /* caller did something wrong */
    CLINE_ERR_INTERNAL  = -2,       /* internal structure corrupted */
    CLINE_ERR_VAL_MISSING   = -3,   /* value required but missing */
    CLINE_ERR_BAD_OPTION    = -4,   /* unknown option specified */
};

struct cline_opt {
    char *names;    /* double-zero-terminated string of possible names. */
    int val;        /* 1 if val needed, 0 otherwise. */
};

struct cline_parser {
    const struct cline_opt *opts, *optend;
    char **args, **argend;

    int section; /* current section */
};

#ifdef __cplusplus
extern "C"
#endif
enum cline_result
cline_init(
    struct cline_parser *parser,
    int optc, const struct cline_opt *opts,
    int argc, char *argv[]);

#ifdef __cplusplus
extern "C"
#endif
enum cline_result
cline_read(
    struct cline_parser *parser,
    const struct cline_opt **opt,
    const char **name,
    const char **val);


#endif /* __cline_h__ */
