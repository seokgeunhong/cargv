
#include "cargv.h"
#include "cargv_version.h"

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>


cargv_version_num_t cargv_version(struct cargv_version_t *ver)
{
    if (ver) {
        ver->major = CARGV_VERSION_MAJOR;
        ver->minor = CARGV_VERSION_MINOR;
        ver->patch = CARGV_VERSION_PATCH;
        ver->state = CARGV_VERSION_STATE_NUMBER;
    }
    return CARGV_VERSION;
}

const char *cargv_version_string()
{
    return CARGV_VERSION_STRING;
}


typedef const char             *_str;
typedef cargv_len_t             _len;
typedef cargv_int_t             _sint;
typedef cargv_uint_t            _uint;
typedef struct cargv_date_t     _date;
typedef struct cargv_time_t     _time;
typedef struct cargv_degree_t   _degree;
typedef struct cargv_geocoord_t _geocoord;

#define _SINT_MIN   CARGV_SINT_MIN
#define _SINT_MAX   CARGV_SINT_MAX
#define _UINT_MAX   CARGV_UINT_MAX


static _str opt_short_prefix = "-";
static _len opt_short_prefix_len = 1;
static _str opt_long_prefix  = "--";
static _len opt_long_prefix_len = 2;
static _str opt_wildcard = "*";
static _len opt_wildcard_len = 1;


/* See if text is empty, only when entire is nonzero.

[out] return: 1 if empty, else 0.
[in] text, textlen: Text to match.
[in] entire: If nonzero, text should end, or fail.
*/
static int match_end(_str text, _len textlen, int entire)
{
    return !(entire && textlen > 0);
}

/* See if the start of a text is matched to a pattern.

[out] return: Length of matched text. 0 if no match found.
[out] end: Points end of matched text. If no match found, points start of text.
[in]  text, textlen: Text to search.
[in]  pattern, patternlen: Pattern to search for.
*/
static int match_str(
    _str *end,
    _str text, _len textlen,
    _str pattern, _len patternlen)
{
    if (textlen >= patternlen && memcmp(text, pattern, patternlen) == 0) {
        *end = text + patternlen;
        return patternlen;
    }
    else {
        *end = text;
        return 0;
    }
}

/* See how many characters from the start of a text are not matched to a
   pattern.

[out] return: Length of unmatch. Whole length of the text if no match found.
[out] end: Points end of unmatch. It is start of the first match, or end of
           the text if no match found.
[in]  text, textlen: Text to search.
[in]  pattern, patternlen: Pattern to search for.
*/
static int unmatch_str(
    _str *end,
    _str text, _len textlen,
    _str pattern, _len patternlen)
{
    _str t;
    _len tlen;

    for (t = text, tlen = textlen; tlen >= patternlen; t++, tlen--) {
        if (memcmp(t, pattern, patternlen) == 0) {
            *end = t;
            return t - text;
        }
    }
    *end = text + textlen;
    return textlen;
}

/* Character matching function type */
typedef int f_match_char(char ch, _str pattern, _len patternlen);

/* See if a character is in a charset.

[out] return: 1 if match, else 0.
[in]  ch: A character to search for in the pattern.
[in]  pattern, patternlen: A character set to search.
*/
static int match_char_set(char ch, _str pattern, _len patternlen)
{
    _str p, e;

    for (p = pattern, e = pattern + patternlen; p < e; p++) {
        if (ch == *p)
            return 1;
    }
    return 0;
}

/* See if a character is in ranges.

[out] return: 1 if match, else 0.
[in]  ch: A character to search for in the pattern.
[in]  pattern, patternlen:  Character pair list. Each character pair in the
                            string defines a range.

        "az"        lowercase alphabet.
        "90afAF"    hexadecimal digits.
*/
static int match_char_range(char ch, _str pattern, _len patternlen)
{
    _str p, e;

    for (p = pattern, e = pattern + patternlen; p+2 <= e; p += 2) {
        if (ch >= p[0] && ch <= p[1])
            return 1;
    }
    return 0;
}

/* See how many characters from the start of a text match to a pattern.

[out] return: Length of the match. 0 if match found less than minc.
[out] end: Points end of matched text. If no match found, points start of text.
[in] text, textlen: Text to search.
[in] pattern, patternlen: Pattern to search for.
[in] minc: Minimum match required. Less match returns 0.
[in] maxc: Maximum match to count. No more match is searched.
*/
static int match_chars(
    f_match_char *matcher,
    _str *end,
    _str text, _len textlen,
    _str pattern, _len patternlen,
    int minc, int maxc)
{
    _str t, tend;

    *end = text;
    for (t = text, tend = text + textlen; t < tend && t - text < maxc; t++) {
        if (!(*matcher)(*t, pattern, patternlen))
            break;
    }
    *end = (t - text < minc) ? text : t;
    return *end - text;
}

/* See how many characters from the start of a text are in a charset.

[out] return: Length of the match. 0 if match found less than minc.
[out] end: Points end of matched text. If no match found, points start of text.
[in] text, textlen: Text to search.
[in] pattern, patternlen: A character set to search for.
[in] minc: Minimum match required. Less match returns 0.
[in] maxc: Maximum match to count. No more match is searched.
*/
static int match_chars_set(
    _str *end,
    _str text, _len textlen,
    _str pattern, _len patternlen,
    int minc, int maxc)
{
    return match_chars(&match_char_set,
        end, text, textlen, pattern, patternlen, minc, maxc);
}

/* See how many characters from the start of a text are in ranges.

[out] return: Length of the match. 0 if match found less than minc.
[out] end: Points end of matched text. If no match found, points start of text.
[in] text, textlen: Text to search.
[in] pattern, patternlen: Character pair list. See match_char_range().
[in] minc: Minimum match required. Less match returns 0.
[in] maxc: Maximum match to count. No more match is searched.
*/
static int match_chars_range(
    _str *end,
    _str text, _len textlen,
    _str pattern, _len patternlen,
    int minc, int maxc)
{
    return match_chars(&match_char_range,
        end, text, textlen, pattern, patternlen,minc, maxc);
}

/* Read an unsigned decimal integer, without any sign.

[out] return: Number of characters succesfully read.
              0 if pattern not matched to a decimal number.
              CARGV_VAL_OVERFLOW if value exceeds CARGV_UINT_MAX.
[out] val: Read value.
[out] end: Points end of matched text. If no match found, points start of text.
[in] text, textlen: Text to match.
[in] entire: If nonzero, text should end after matching, or fail.
*/
static int read_dec(
    _uint *val,
    _str *end, _str text, _len textlen, int entire)
{
    _str t, tend;

    *val = 0;
    *end = text;

    t = text;
    tend = text + textlen;
    if (!(match_chars_range(&t, t, tend-t, "09", 2, 0, 100)
            && match_end(t, tend-t, entire)))
        return 0;

    tend = t;
    for (t = text; t < tend; t++) {
        if (*val < _UINT_MAX / 10
                || (*val == _UINT_MAX / 10 && *t - '0' <= _UINT_MAX % 10))
            *val = *val * 10 + (*t - '0');
        else
            return CARGV_VAL_OVERFLOW;
    }
    *end = tend;
    return *end - text;
}

/* Read a signed decimal integer, from the start of the text.

[out] return: Number of characters succesfully read.
              0 if none found.
              CARGV_VAL_OVERFLOW if not in [CARGV_SINT_MIN,CARGV_SINT_MAX].
[out] val: Read value.
[out] end: Points end of matched text. If no match found, points start of text.
[in] text, textlen: Text to match.
[in] entire: If nonzero, text should end after matching, or fail.
*/
static int read_sdec(_sint *val, _str *end, _str text, _len textlen, int entire)
{
    int r;
    _uint u;
    _str t, tend;

    *val = 0;
    *end = text;

    t = text;
    tend = text + textlen;

    match_chars_set(&t, t, tend-t, "+-", 2, 0, 1);
    if ((r = read_dec(&u, &t, t, tend-t, entire)) <= 0)
        return r;

    if (*text == '-') {
        if (u < (_uint)_SINT_MAX+1)
            *val = -(_sint)u;
        else if (u == (_uint)_SINT_MAX+1)
            *val = _SINT_MIN;
        else
            return CARGV_VAL_OVERFLOW;
    }
    else {
        if (u <= (_uint)_SINT_MAX)
            *val = (_sint)u;
        else
            return CARGV_VAL_OVERFLOW;
    }

    *end = t;
    return *end - text;
}

/* Read an unsigned decimal integer, from the start of the text.

[out] return: Number of characters succesfully read.
              0 if none found.
              CARGV_VAL_OVERFLOW if not in [0,CARGV_UINT_MAX].
[out] val: Read value.
[out] end: Points end of matched text. If no match found, points start of text.
[in] text, textlen: Text to match.
[in] entire: If nonzero, text should end after matching, or fail.
*/
static int read_udec(
    _uint *val,
    _str *end, _str text, _len textlen, int entire)
{
    int r;
    _str t, tend;

    *val = 0;
    *end = text;

    t = text;
    tend = text + textlen;

    match_chars_set(&t, t, tend-t, "+-", 2, 0, 1);
    if ((r = read_dec(val, &t, t, tend-t, entire)) <= 0)
        return r;

    if (*text == '-')
        return CARGV_VAL_OVERFLOW;

    *end = t;
    return *end - text;
}

/* Read a modified ISO 8601 date, from the start of the text.

    [+ or -]YYYY-MM-DD, or [+ or -]YYYY/MM/DD

[out] return: Number of characters succesfully read.
              0 if none found.
              <0 if error; See cargv_err_t.
[out] val: Read value.
[out] end: Points end of matched text. If no match found, points start of text.
[in] text, textlen: Text to match.
*/
static int read_date_iso8601(
    _date *val,
    _str *end, _str text, _len textlen, int entire)
{
    static const int dom[] = {
        0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    _str t, tend;
    int leap;

    memset(val, 0, sizeof(*val));
    *end = text;

    t = text;
    tend = text + textlen;

    if (!(read_sdec(&val->year, &t, t, tend-t, 0) > 0
            && match_chars_set(&t, t, tend-t, "-/", 2, 1, 1)
            && read_dec(&val->month, &t, t, tend-t, 0) > 0
            && match_chars_set(&t, t, tend-t, "-/", 2, 1, 1)
            && read_dec(&val->day, &t, t, tend-t, 0) > 0
            && match_end(t, tend-t, entire)))
        return 0;

    leap = val->month == 2
            && ((!(val->year % 4) && (val->year % 100)) || !(val->year % 400));

    if (!(val->year >= -9999 && val->year != 0 && val->year <= 9999
            && val->month >= 1 && val->month <= 12
            && val->day >= 1 && val->day <= dom[val->month] + leap))
        return CARGV_VAL_OVERFLOW;

    *end = t;
    return *end - text;

}

/* Read a modified ISO 8601 time, from the start of the text.

    HH:MM[:SS]

[out] return: Number of characters succesfully read.
              0 if none found.
              <0 if error; See cargv_err_t.
[out] val: Read value.
[out] end: Points end of matched text. If no match found, points start of text.
[in] text, textlen: Text to match.
*/
static int read_time_iso8601(
    _time *val,
    _str *end, _str text, _len textlen, int entire)
{
    _str t, tend;

    memset(val, 0, sizeof(*val));
    *end = text;

    t = text;
    tend = text + textlen;

    if (!(read_dec(&val->hour, &t, t, tend-t, 0) > 0
            && match_chars_set(&t, t, tend-t, ":", 1, 1, 1)
            && read_dec(&val->minute, &t, t, tend-t, 0) > 0)) {
        return 0;
    }
    if (match_chars_set(&t, t, tend-t, ":", 1, 1, 1)) {
        if (!(read_dec(&val->second, &t, t, tend-t, 0) > 0))
            return 0;
    }
    if (!match_end(t, tend-t, entire))
        return 0;

    if (!(val->hour <= 24 && val->minute <= 60 && val->second <= 60))
        return CARGV_VAL_OVERFLOW;

    *end = t;
    return *end - text;
}

/* power of 10 */
static int _p10(int e)
{
    int x = 1;
    while (e-- > 0)
        x *= 10;
    return x;
}

/* Read a ISO 6709 degree, from the start of the text.

    <+ or ->[D]DD[.DDDDDD], <+ or ->[D]DDMM[.MMMM], or <+ or ->[D]DDMMSS[.SS]

[out] return: Number of characters succesfully read.
              0 if none found.
              <0 if error; See cargv_err_t.
[out] val: Read value.
[out] end: Points end of matched text. If no match found, points start of text.
[in] text, textlen: Text to match.
*/
static int read_degree_iso6709(
    _degree *val,
    _str *end, _str text, _len textlen, int entire)
{
    _str t, tend;
    _uint a, b;
    int alen, blen;

    memset(val, 0, sizeof(*val));
    *end = text;

    t = text;
    tend = text + textlen;

    /* sign is mandatory */
    if (!match_chars_set(&t, t, tend-t, "+-", 2, 1, 1))
        return 0;

    if ((alen = read_dec(&a, &t, t, tend-t, 0)) <= 0)
        return 0;

    /* dot is optinal */
    if (match_chars_set(&t, t, tend-t, ".", 1, 1, 1)) {
        if (!((blen = read_dec(&b, &t, t, tend-t, 0)) > 0))
            return 0;
    }
    else {
        b = 0;
        blen = 0;
    }
    if (!match_end(t, tend-t, entire))
        return 0;

    /* [D]DD[.DDDDDD] */
    if (alen < 4) {
        val->deg = a * 1000000 + b / _p10(blen-6) * _p10(6-blen);
        val->min = val->sec = 0;
    }
    /* [D]DDMM[.MMMM] */
    else if (alen < 6) {
        val->deg = a / 100 * 1000000;
        val->min = a % 100 * 1000000 + b / _p10(blen-4) * _p10(4-blen) * 100;
        val->sec = 0;
    }
    /* [D]DDMMSS[.SS] */
    else if (alen < 8) {
        val->deg = a / 10000 * 1000000;
        val->min = a % 10000 / 100 * 1000000;
        val->sec = a % 100 * 1000000 + b / _p10(blen-2) * _p10(2-blen) * 10000;
    }
    else
        return CARGV_VAL_OVERFLOW;

    if (!(val->deg <= 360000000
            && val->min <= 60000000
            && val->sec <= 60000000))
        return CARGV_VAL_OVERFLOW;

    if (*text == '-') {
        val->deg = -val->deg;
        val->min = -val->min;
        val->sec = -val->sec;
    }
    *end = t;
    return *end - text;
}

/* Read a ISO 6709 geocoord, from the start of the text.

    LATITUDELONGITUDE[/]

[out] return: Number of characters succesfully read.
              0 if none found.
              <0 if error; See cargv_err_t.
[out] val: Read value.
[out] end: Points end of matched text. If no match found, points start of text.
[in] text, textlen: Text to match.
*/
static int read_geocoord_iso6709(
    _geocoord *val,
    _str *end, _str text, _len textlen, int entire)
{
    int a, b;
    _str t, tend;

    memset(val, 0, sizeof(*val));
    *end = text;

    t = text;
    tend = text + textlen;

    if ((a = read_degree_iso6709(&val->latitude, &t, t, tend-t, 0)) == 0)
        return 0;
    if ((b = read_degree_iso6709(&val->longitude, &t, t, tend-t, 0)) == 0)
        return 0;
    match_chars_set(&t, t, tend-t, "/", 1, 0, 1);   /* optional solidus */
    if (!match_end(t, tend-t, entire))
        return 0;

    if (a < 0 || b < 0
            || !(val->latitude.deg >= -90000000
            && val->latitude.deg <= 90000000
            && val->longitude.deg >= -180000000
            && val->longitude.deg <= 180000000))
        return CARGV_VAL_OVERFLOW;

    *end = t;
    return *end - text;
}

static int err_val_result(
    struct cargv_t *cargv,
    const char *name,
    const char *type,
    _str arg,
    int result)
{
    if (result < 0) {
        if (result == CARGV_VAL_OVERFLOW) {
            fprintf(stderr,
                "%s: `%s` overflows, which is `%s`.\n",
                cargv->name, name, arg);
        }
        else {
            fprintf(stderr,
                "%s: Unknown error reading `%s`, which is `%s`.\n",
                cargv->name, name, arg);
        }
    }
    return result;
}


enum cargv_err_t
cargv_init(
    struct cargv_t *cargv,
    const char *name,
    int argc, const char **argv)
{
    cargv->name = name;
    cargv->args = argv;
    cargv->argend = argv + argc;
    return CARGV_OK;
}

int cargv_shift(struct cargv_t *cargv, cargv_len_t argc)
{
    if (argc > 0 && cargv->args + argc <= cargv->argend) {
        cargv->args += argc;
        return (int)argc;
    }
    return 0;
}

int cargv_opt(struct cargv_t *cargv, const char *optlist)
{
    _str lopt, oend, o;
    _str aend, a;
    int wildcard;

    if (!(cargv->args < cargv->argend))
        return 0;

    aend = *cargv->args + strlen(*cargv->args);
    oend = optlist + strlen(optlist);

    /* Argument is short option of `*`, default `-*`, matches any option */
    o = optlist;
    wildcard
        = match_str(&o, o, oend-o, opt_short_prefix, opt_short_prefix_len)
        && match_str(&o, o, oend-o, opt_wildcard, opt_wildcard_len)
        && match_end(o, oend-o, 1);

    /* Split optlist */
    o = optlist;
    unmatch_str(&lopt, o, oend-o, opt_long_prefix, opt_long_prefix_len);

    /* Argument is long option, like `--long-option` */
    a = *cargv->args;
    if (match_str(&a, a, aend-a, opt_long_prefix, opt_long_prefix_len)) {
        if (!(a < aend))
            return 0;   /* -- */
        if (wildcard)
            return 1;

        /* Iterate long options */
        o = lopt;
        while (o < oend) {
            o += opt_long_prefix_len;   /* already matched */
            unmatch_str(&lopt, o, oend-o, opt_long_prefix, opt_long_prefix_len);
            if (match_str(&a, a, aend-a, o, lopt-o) && match_end(a, aend-a, 1))
                return 1;
            o = lopt;
        }
    }
    /* Argument is short option, like `-s` */
    else if (match_str(&a, a, aend-a, opt_short_prefix, opt_short_prefix_len)) {
        if (!(a < aend))
            return 0;   /* - */
        if (wildcard)
            return 1;

        /* Find in short options */
        o = optlist;
        if (match_str(&o, o, lopt-o, opt_short_prefix, opt_short_prefix_len)
                && match_chars_set(&a, a, aend-a, o, lopt-o, 1, lopt-o)
                && match_end(a, aend-a, 1))
            return 1;
    }
    return 0;
}

int cargv_text(
    struct cargv_t *cargv,
    const char *name,
    const char **vals, cargv_len_t valc)
{
    _str *v, *a;

    memset((void *)vals, 0, valc * sizeof(const char *));

    v = vals;
    a = cargv->args;
    while (v - vals < valc && a < cargv->argend)
        *v++ = *a++;

    return (int)(v-vals);
}

int cargv_oneof(
    struct cargv_t *cargv,
    const char *name,
    const char *list, const char *sep,
    const char **vals, cargv_len_t valc)
{
    _str *val, *arg;
    _str listend, t, tend;
    _str aend, a;
    int seplen;

    listend = list + strlen(list);
    seplen = strlen(sep);

    memset((void *)vals, 0, valc * sizeof(*vals));
    val = vals;
    arg = cargv->args;
    while (val - vals < valc && arg < cargv->argend) {
        aend = *arg + strlen(*arg);
        a = *arg;
        t = list;
        while (t < listend) {
            unmatch_str(&tend, t, listend-t, sep, seplen);
            if (match_str(&a, a, aend-a, t, tend-t) && match_end(a, aend-a, 1))
                break;
            t = tend + seplen;
        }
        if (t < listend)    /* found */
            *val++ = *arg;
        else
            break;
        arg++;
    }
    return (int)(val-vals);
}

int cargv_int(
    struct cargv_t *cargv,
    const char *name,
    cargv_int_t *vals, cargv_len_t valc)
{
    int r;
    _sint *v;
    _str *a, e;

    memset(vals, 0, valc * sizeof(*vals));

    v = vals;
    a = cargv->args;
    while (v - vals < valc && a < cargv->argend) {
        r = read_sdec(v, &e, *a, strlen(*a), 1);
        if (r < 0)
            return err_val_result(cargv, name, "integer", *a, r);
        if (r == 0)
            break;
        v++;
        a++;
    }
    return (int)(v-vals);
}

int cargv_uint(
    struct cargv_t *cargv,
    const char *name,
    cargv_uint_t *vals, cargv_len_t valc)
{
    int r;
    _uint *v;
    _str *a, e;

    memset(vals, 0, valc * sizeof(*vals));

    v = vals;
    a = cargv->args;
    while (v - vals < valc && a < cargv->argend) {
        r = read_udec(v, &e, *a, strlen(*a), 1);
        if (r < 0)
            return err_val_result(cargv, name, "unsigned integer", *a, r);
        if (r == 0)
            break;
        v++;
        a++;
    }
    return (int)(v-vals);
}

int cargv_date(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_date_t *vals, cargv_len_t valc)
{
    int r;
    _date *v;
    _str *a, e;

    memset(vals, 0, valc * sizeof(*vals));

    v = vals;
    a = cargv->args;
    while (v - vals < valc && a < cargv->argend) {
        r = read_date_iso8601(v, &e, *a, strlen(*a), 1);
        if (r < 0)
            return err_val_result(cargv, name, "date", *a, r);
        if (r == 0)
            break;
        v++;
        a++;
    }
    return (int)(v-vals);
}

int cargv_time(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_time_t *vals, cargv_len_t valc)
{
    int r;
    _time *v;
    _str *a, e;

    memset(vals, 0, valc * sizeof(*vals));

    v = vals;
    a = cargv->args;
    while (v - vals < valc && a < cargv->argend) {
        r = read_time_iso8601(v, &e, *a, strlen(*a), 1);
        if (r < 0)
            return r;
        if (r == 0)
            break;
        v++;
        a++;
    }
    return (int)(v-vals);
}

int cargv_degree(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_degree_t *vals, cargv_len_t valc)
{
    int r;
    _degree *v;
    _str *a, e;

    memset(vals, 0, valc * sizeof(*vals));

    v = vals;
    a = cargv->args;
    while (v - vals < valc && a < cargv->argend) {
        r = read_degree_iso6709(v, &e, *a, strlen(*a), 1);
        if (r < 0)
            return err_val_result(cargv, name, "degree", *a, r);
        if (r == 0)
            break;
        v++;
        a++;
    }
    return (int)(v-vals);
}

int cargv_geocoord(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_geocoord_t *vals, cargv_len_t valc)
{
    int r;
    _geocoord *v;
    _str *a, e;

    memset(vals, 0, valc * sizeof(*vals));

    v = vals;
    a = cargv->args;
    while (v - vals < valc && a < cargv->argend) {
        r = read_geocoord_iso6709(v, &e, *a, strlen(*a), 1);
        if (r < 0)
            return err_val_result(cargv, name, "geocoord", *a, r);
        if (r == 0)
            break;
        v++;
        a++;
    }
    return (int)(v-vals);
}


time_t cargv_get_today(struct cargv_date_t *val)
{
    time_t t;
    struct tm *lt;

    time(&t);
    lt = localtime(&t);

    val->year = lt->tm_year;
    val->month = lt->tm_mon;
    val->day = lt->tm_mday;
    return t;
}

time_t cargv_get_now(struct cargv_time_t *val)
{
    time_t t;
    struct tm *lt;

    time(&t);
    lt = localtime(&t);

    val->hour = lt->tm_hour;
    val->minute = lt->tm_mon;
    val->second = lt->tm_mday;
    return t;
}

double cargv_get_degree(const struct cargv_degree_t *val)
{
    return (double)val->deg / 1E+6
        + (double)val->min / 1E+6 / 60.0
        + (double)val->sec / 1E+6 / 3600.0;
}
