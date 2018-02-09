
#include "cline.h"
#include <stdio.h>
#include <string.h>
#include <math.h>


enum cline_status {
    CLINE_PARSE_BEGIN   = 0,
    CLINE_PARSE_OPT,
    CLINE_PARSE_ARG,
};

/* match end of string */
static int match_end(const char *str)
{
    return !*str ? 1 : 0;
}

/* character match function type */
typedef int f_match_char(char ch, const char *against);

/* f_match_char */
static int match_char_set(char ch, const char *charset)
{
    const char *c;
    for (c = charset; *c; c++) {
        if (*c == ch)
            return 1;
    }
    return 0;
}

/* f_match_char */
static int match_char_range(char ch, const char *range)
{
    const char *c;
    for (c = range; c[0] && c[1]; c += 2) {
        if (ch >= c[0] && ch <= c[1])
            return 1;
    }
    return 0;
}

static int match_chars(
    f_match_char *matcher,
    const char **val, const char **end, const char *str,
    const char *against,
    int minlen, int maxlen)
{
    const char *s;
    const char *c;
    int n;

    for (s = str; *s && s - str < maxlen; s++) {
        if (!(*matcher)(*s, against))
            break;
    }
    if (s - str >= minlen) {
        *val = str;
        *end = s;
        return *end - str;
    }
    else {
        *end = str;
        return 0;
    }
}

static int match_chars_set(
    const char **val, const char **end, const char *str,
    const char *charset,
    int minlen, int maxlen)
{
    return match_chars(&match_char_set, val, end, str, charset, minlen, maxlen);
}

static int match_chars_range(
    const char **val, const char **end, const char *str,
    const char *range,
    int minlen, int maxlen)
{
    return match_chars(&match_char_range, val, end, str, range, minlen, maxlen);
}

/* unsigned decimal integer */
static int read_udec(int *val, const char **end, const char *str)
{
    const char *s;

    if (match_chars_range(&s, end, str, "09", 1, 10)) {
        *val = 0;
        for (; s < *end; s++)
            *val = *val * 10 + (*s - '0');
        return *end - str;
    }
    return 0;
}

/* signed decimal integer */
static int read_sdec(int *val, const char **end, const char *str)
{
    const char *s = str, *sign;
    int negative;

    negative = match_chars_set(&sign, &s, s, "+-", 1, 1) && *sign == '-';
    if (read_udec(val, end, s)) {
        if (negative)
            *val = -*val;
        return *end - str;
    }
    return 0;
}

/* ISO date: yyyy-mm-dd */
static int read_date(struct cline_date *val, const char **end, const char *str)
{
    const char *s = str;
    const char *sep;

    return read_udec(&val->year, &s, s)
        && match_chars_set(&sep, &s, s, "-/", 1, 1)
        && read_udec(&val->month, &s, s)
        && match_chars_set(&sep, &s, s, "-/", 1, 1)
        && read_udec(&val->day, end, s)
        ? (*end - str) : 0;
}

static int pow10(int e)
{
    int x = 1;
    while (e-- > 0)
        x *= 10;
    return x;
}

static int read_geocoord_iso6709_degree(
    struct cline_degree *val,
    const char **end, const char *str)
{
    const char *s = str, *sign, *dot;
    int a, la, b, lb; /* `a.b` */

    *end = str;

    if (!match_chars_set(&sign, &s, s, "+-", 1, 1))
        return 0;
    if (!(la = read_udec(&a, &s, s)))
        return 0;
    if (match_chars_set(&dot, &s, s, ".", 1, 1)) {
        if (!(lb = read_udec(&b, &s, s)))
            return 0;
    }
    else {
        lb = 0;
        b = 0;
    }
    if (la < 1) {
        return 0;
    }
    else if (la < 4) {
        val->deg = a * 1000000 + b / pow10(lb - 6) * pow10(6 - lb);
        val->min = val->sec = 0;
    }
    else if (la < 6) {
        val->deg = a / 100 * 1000000;
        val->min = a % 100 * 1000000 + b / pow10(lb - 4) * pow10(4 - lb) * 100;
        val->sec = 0;
    }
    else if (la < 8) {
        val->deg = a / 10000 * 1000000;
        val->min = a % 10000 / 100 * 1000000;
        val->sec = a % 100 * 1000000 + b / pow10(lb - 2) * pow10(2 - lb) * 10000;
    }
    else {
        return 0;
    }

    if (!(val->min >= 0 && val->min < 60000000
        && val->sec >= 0 && val->sec < 60000000))
        return 0;

    if (*sign == '-') {
        val->deg = -val->deg;
        val->min = -val->min;
        val->sec = -val->sec;
    }

    *end = s;
    return *end - str;
}

static int read_geocoord_iso6709(
    struct cline_geocoord *val,
    const char **end, const char *str)
{
    const char *s = str;
    const char *solidus;

    if (read_geocoord_iso6709_degree(&val->latitude, &s, s)
        && val->latitude.deg >= -90000000 && val->latitude.deg <= 90000000
        && read_geocoord_iso6709_degree(&val->longitude, &s, s)
        && val->longitude.deg >= -180000000 && val->longitude.deg <= 180000000)
    {
        match_chars_set(&solidus, end, s, "/", 1, 1);  // optional `/`
        return *end - str;
    }
    return 0;
}

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
    parser->state = CLINE_PARSE_BEGIN;
    return CLINE_OK;
}

enum cline_result
cline_read(
    struct cline_parser *parser,
    const struct cline_opt **opt,
    const char **name,
    struct cline_value *val)
{
    const struct cline_opt *o;
    const char *n;
    const char *a;

    if (!(parser && opt && name))
        return CLINE_ERR_PARAM;

    *opt = NULL;
    *name = NULL;
    memset(val, 0, sizeof(*val));

    if (parser->state == CLINE_PARSE_BEGIN) {
        parser->args++;
        parser->state = CLINE_PARSE_OPT;
    }

    if (!(parser->args < parser->argend))
        return CLINE_END;

    if (parser->state == CLINE_PARSE_OPT) {
        if (strcmp(*parser->args, "--") == 0) { /* option separator */
            parser->args++;                     /* from next, */
            parser->state = CLINE_PARSE_ARG;    /* non-option arguments come */
        }
        else if ((*parser->args)[0] != '-') {   /* should begin with '-' */
            parser->state = CLINE_PARSE_ARG;    /* or non-option arguments */
        }
        else {
            for (o = parser->opts; o < parser->optend; o++) {   /* opts */
                for (n = o->names; *n; n = n + strlen(n) + 1) { /* names */
                    if (strcmp(*parser->args, n) == 0) {
                        *opt = o;
                        *name = n;
                        if (o->val != CLINE_BOOL) {
                            if (!(++parser->args < parser->argend))
                                return CLINE_ERR_VALUE_REQUIRED;
                            val->arg = *parser->args;
                        }
                        parser->args++;

                        switch (o->val) {
                        case CLINE_BOOL:
                            val->flag = 1;
                            return CLINE_OK;

                        case CLINE_TEXT:
                            val->text = val->arg;
                            return CLINE_OK;

                        case CLINE_INTEGER:
                            return read_sdec(&val->integer, &a, val->arg)
                                && match_end(a)
                                ? CLINE_OK : CLINE_ERR_VALUE;

                        case CLINE_DATE:
                            return read_date(&val->date, &a, val->arg)
                                && match_end(a)
                                ? CLINE_OK : CLINE_ERR_VALUE;

                        case CLINE_GEOCOORD:
                            return read_geocoord_iso6709(
                                &val->geocoord, &a, val->arg)
                                && match_end(a)
                                ? CLINE_OK : CLINE_ERR_VALUE;
                        }
                        return CLINE_ERR_VALUE;
                    }
                }
            }
            return CLINE_ERR_OPTION;
        }
    }
    if (parser->state == CLINE_PARSE_ARG) {
        return CLINE_END;
    }
    return CLINE_ERR_INTERNAL;
}

CLINE_EXPORT
double cline_get_degree(const struct cline_degree *val)
{
    return (double)val->deg / 1E+6
        + (double)val->min / 1E+6 / 60.0
        + (double)val->sec / 1E+6 / 3600.0;
}
