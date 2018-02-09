#ifndef __cline_h__
#define __cline_h__


#ifdef __cplusplus
  #define CLINE_EXPORT extern "C"
#else
  #define CLINE_EXPORT
#endif


enum cline_result {
    CLINE_OK    = 0,
    CLINE_END,     /* no more arguments */

    CLINE_ERR_PARAM     = -1,   /* somethong wrong in passed parameters */
    CLINE_ERR_INTERNAL  = -2,   /* internal structure corrupted */
    CLINE_ERR_OPTION    = -3,   /* unknown option specified */
    CLINE_ERR_VALUE_REQUIRED    = -4,   /* value required but missing */
    CLINE_ERR_VALUE     = -5,           /* wrong value */
};

enum cline_type {
    CLINE_BOOL,
    CLINE_TEXT,
    CLINE_INTEGER,
    CLINE_DATE,
    CLINE_GEOCOORD,
};

struct cline_opt {
    char *names;    /* double-zero-terminated string of possible names. */
    enum cline_type val;
};

struct cline_date {
    int year, month, day;
};

struct cline_degree {
    int deg;    /* ddd.dddddd */
    int min;    /* mm.mmmmmm */
    int sec;    /* ss.ssssss */
};

struct cline_geocoord {
    struct cline_degree latitude, longitude;
    /* int altitude; */
};

struct cline_value {
    const char *arg;
    union {
        int flag;
        int integer;
        const char *text;
        struct cline_date date;
        struct cline_geocoord geocoord;
    };
};

struct cline_parser {
    const struct cline_opt *opts, *optend;
    char **args, **argend;

    int state;
};

CLINE_EXPORT
enum cline_result
cline_init(
    struct cline_parser *parser,
    int optc, const struct cline_opt *opts,
    int argc, char *argv[]);

/*

return values:

    CLINE_OK                    An option is successfully read.
                                `name` is the string in `argv`.
                                `val.arg` is the string in `argv`, if given.
                                `val.`<value> is set with value, if given.

    CLINE_ERR_OPTION            An unrecognized option is found.
                                Output arguments are undefined.

    CLINE_ERR_VALUE_REQUIRED    An option is given, while requirs a value but
                                not given.
                                `name` is the string in `argv`.
                                `val.arg` is null.
                                `val.`<value> is set to 0.

    CLINE_ERR_VALUE             A value is given but invalid.
                                `name` is the string in `argv`.
                                `val.arg` is given string in `argv`.
                                `val.`<value> is undefined.
*/
CLINE_EXPORT
enum cline_result
cline_read(
    struct cline_parser *parser,
    const struct cline_opt **opt,
    const char **name,
    struct cline_value *val);


CLINE_EXPORT
double cline_get_degree(const struct cline_degree *val);


#endif /* __cline_h__ */
