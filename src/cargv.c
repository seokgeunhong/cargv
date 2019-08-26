
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
        ver->state = CARGV_VERSION_STATE_DEV;
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
[in] text, textend: Text to match.
[in] entire: If nonzero, text should end, or fail.
*/
static int __match_end(_str text, _str textend, int entire)
{
    return !(entire && text < textend);
}

/* See if the start of a text is matched to a pattern.

[out] return: Length of matched text. 0 if no match found.
[out] next: Points end of matched text. Untouched if no match found.
[in]  text, textend: Text to search.
[in]  pattern, patternlen: Pattern to search for.
*/
static int __match_str(
    _str *next,
    _str text, _str textend,
    _str pattern, _len patternlen)
{
    if ((textend-text) >= patternlen
        && memcmp(text, pattern, patternlen) == 0) {
        *next = text + patternlen;
        return patternlen;
    }
    else
        return 0;
}

/* See how many characters from the start of a text are not matched to a
   pattern.

[out] return: Length of unmatch. Whole length of the text if no match found.
[out] next: Points end of unmatch. It is start of the first match, or end of
            the text if no match found.
[in]  text, textend: Text to search.
[in]  pattern, patternlen: Pattern to search for.
*/
static int __unmatch_str(
    _str *next,
    _str text, _str textend,
    _str pattern, _len patternlen)
{
    _str t;

    for (t = text; textend-t >= patternlen; ++t) {
        if (memcmp(t, pattern, patternlen) == 0)
            return (int)((*next = t) - text);
    }
    return (int)((*next = textend) - text);
}


/* Character matching function type */
typedef int f_match_char(char ch, _str pattern, _len patternlen);

/* See if a character is in a charset.

[out] return: 1 if match, else 0.
[in]  ch: A character to search for in the pattern.
[in]  pattern, patternlen: A character set to search.
*/
static int __match_char_set(char ch, _str pattern, _len patternlen)
{
    _str p, pend;

    for (p = pattern, pend = pattern + patternlen; p < pend; ++p) {
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
static int __match_char_range(char ch, _str pattern, _len patternlen)
{
    _str p, pend;

    for (p = pattern, pend = pattern + patternlen; p+2 <= pend; p += 2) {
        if (ch >= p[0] && ch <= p[1])
            return 1;
    }
    return 0;
}

/* See how many characters from the start of a text match to a pattern.

[out] return: Length of the match. 0 if match found less than minc.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textend: Text to search.
[in] pattern, patternlen: Pattern to search for.
[in] minc: Minimum match required. Less match returns 0.
[in] maxc: Maximum match to count. No more match is searched.
*/
static int __match_chars(
    f_match_char *matcher,
    _str *next,
    _str text, _str textend,
    _str pattern, _len patternlen,
    int minc, int maxc)
{
    _str t, tend;

    tend = (textend < text + maxc) ? textend : text + maxc;
    for (t = text; t < tend; ++t) {
        if (!(*matcher)(*t, pattern, patternlen))
            break;
    }
    if (t < text + minc)
        return 0;
    return (int)((*next = t) - text);
}

/* See how many characters from the start of a text are in a charset.

[out] return: Length of the match. 0 if match found less than minc.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textend: Text to search.
[in] pattern, patternlen: A character set to search for.
[in] minc: Minimum match required. Less match returns 0.
[in] maxc: Maximum match to count. No more match is searched.
*/
static int __match_chars_set(
    _str *next,
    _str text, _str textend,
    _str pattern, _len patternlen,
    int minc, int maxc)
{
    return __match_chars(&__match_char_set,
        next, text, textend, pattern, patternlen, minc, maxc);
}

/* See how many characters from the start of a text are in ranges.

[out] return: Length of the match. 0 if match found less than minc.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textend: Text to search.
[in] pattern, patternlen: Character pair list. See __match_char_range().
[in] minc: Minimum match required. Less match returns 0.
[in] maxc: Maximum match to count. No more match is searched.
*/
static int __match_chars_range(
    _str *next,
    _str text, _str textend,
    _str pattern, _len patternlen,
    int minc, int maxc)
{
    return __match_chars(&__match_char_range,
        next, text, textend, pattern, patternlen,minc, maxc);
}

/* Read an unsigned decimal integer, without any sign.

[out] return: Number of characters succesfully read.
              0 if pattern not matched to a decimal number.
              CARGV_VAL_OVERFLOW if value exceeds CARGV_UINT_MAX.
[out] val: Read value. Undefined on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textlen: Text to match.
*/
static int __read_dec(
    _uint *val,
    _str *next, _str text, _str textend)
{
    _str t, u;

    *val = 0;

    t = text;
    if (!(__match_chars_range(&t, t, textend, "09", 2, 0, 100) > 0))
        return 0;

    for (u = text; u < t; u++) {
        if (*val < _UINT_MAX / 10
                || (*val == _UINT_MAX / 10 && *u - '0' <= _UINT_MAX % 10))
            *val = *val * 10 + (*u - '0');
        else
            return CARGV_VAL_OVERFLOW;
    }
    return (int)((*next = t) - text);
}

/* Read a signed decimal integer, from the start of the text.

[out] return: Number of characters succesfully read.
              0 if none found.
              CARGV_VAL_OVERFLOW if not in [CARGV_SINT_MIN,CARGV_SINT_MAX].
[out] val: Read value. Undefined on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textlen: Text to match.
[in] entire: If nonzero, text should end after matching, or fail.
*/
static int read_sdec(_sint *val, _str *next, _str text, _len textlen, int entire)
{
    _str t, tend;
    _uint u;
    int r;

    t = text;
    tend = text + textlen;

    __match_chars_set(&t, t, tend, "+-", 2, 0, 1);
    if (!((r = __read_dec(&u, &t, t, tend)) > 0))
        return r;
    if (!__match_end(t, tend, entire))
        return 0;

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

    return (int)((*next = t) - text);
}

/* Read an unsigned decimal integer, from the start of the text.

[out] return: Number of characters succesfully read.
              0 if none found.
              CARGV_VAL_OVERFLOW if not in [0,CARGV_UINT_MAX].
[out] val: Read value. Undefined on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textlen: Text to match.
[in] entire: If nonzero, text should end after matching, or fail.
*/
static int read_udec(
    _uint *val,
    _str *next, _str text, _len textlen, int entire)
{
    _str t, tend;
    int r;

    t = text;
    tend = text + textlen;

    __match_chars_set(&t, t, tend, "+-", 2, 0, 1);
    if (!((r = __read_dec(val, &t, t, tend)) > 0))
        return r;
    if (!__match_end(t, tend, entire))
        return 0;

    if (*text == '-')
        return CARGV_VAL_OVERFLOW;

    return (int)((*next = t) - text);
}


/* DD */
static int __read_iso8601_DD(
    _date *val,
    _str *next, _str text, _str textend)
{
    _str t = text;

    if (__read_dec(&val->day, &t, t, textend) > 0)
        return (int)((*next = t) - text);
    else
        return 0;
}

/* MM */
static int __read_iso8601_MM(
    _date *val,
    _str *next, _str text, _str textend)
{
    _str t = text;

    if (__read_dec(&val->month, &t, t, textend) > 0)
        return (int)((*next = t) - text);
    else
        return 0;
}

/* MMDD */
static int __read_iso8601_MMDD(
    _date *val,
    _str *next, _str text, _str textend)
{
    _str t = text;

    if (__read_dec(&val->month, &t, t, textend) == 4) {
        val->day = val->month % 100;
        val->month /= 100;
        return (int)((*next = t) - text);
    }
    return 0;
}

/* MM-DD or MM/DD */
static int __read_iso8601_MM_DD(
    _date *val,
    _str *next, _str text, _str textend)
{
    _str t = text;

    if (__read_iso8601_MM(val, &t, t, textend) > 0
            && __match_chars_set(&t, t, textend, "-/", 2, 1, 1) > 0
            && __read_iso8601_DD(val, &t, t, textend) > 0) {
        return (int)((*next = t) - text);
    }
    return 0;
}

/* [+-] */
static int __read_iso8601_sign(
    _sint *sign,
    _str *next, _str text, _str textend)
{
    _str t = text;

    if (__match_char_set(*t, "+", 1)) {
        *sign = 1;
        return (int)((*next = ++t) - text);
    }
    else if (__match_char_set(*t, "-", 1)) {
        *sign = -1;
        return (int)((*next = ++t) - text);
    }
    *sign = 1;
    return 0;
}

/* [+-]YYYY */
static int __read_iso8601_YYYY(
    _date *val,
    _str *next, _str text, _str textend)
{
    _str t = text;
    _sint sign;

    __read_iso8601_sign(&sign, &t, t, textend);
    if (__read_dec(&val->year, &t, t, textend) > 0) {
        val->year *= sign;
        return (int)((*next = t) - text);
    }
    return 0;
}

/* [+-]YYYYMMDD */
static int __read_iso8601_YYYYMMDD(
    _date *val,
    _str *next, _str text, _str textend)
{
    _str t = text;
    _sint sign;
    _uint n;

    __read_iso8601_sign(&sign, &t, t, textend);
    if (__read_dec(&n, &t, t, textend) == 8) {
        val->year = n / 10000 * sign;
        val->month = (n % 10000) / 100;
        val->day = n % 100;
        return (int)((*next = t) - text);
    }
    return 0;
}

/* [+-]YYYY-MM-DD or [+-]YYYY/MM/DD */
static int __read_iso8601_YYYY_MM_DD(
    _date *val,
    _str *next, _str text, _str textend)
{
    _str t = text;

    if (__read_iso8601_YYYY(val, &t, t, textend) > 0
            && __match_chars_set(&t, t, textend, "-/", 2, 1, 1) > 0
            && __read_iso8601_MM_DD(val, &t, t, textend) > 0) {
        return (int)((*next = t) - text);
    }
    return 0;
}

/* [+-]YYYY-MM or [+-]YYYY/MM */
static int __read_iso8601_YYYY_MM(
    _date *val,
    _str *next, _str text, _str textend)
{
    _str t = text;

    if (__read_iso8601_YYYY(val, &t, t, textend) > 0
            && __match_chars_set(&t, t, textend, "-/", 2, 1, 1) > 0
            && __read_iso8601_MM(val, &t, t, textend) > 0) {
        return (int)((*next = t) - text);
    }
    return 0;
}

/* --MM-DD or --MM/DD */
static int __read_iso8601___MM_DD(
    _date *val,
    _str *next, _str text, _str textend)
{
    _str t = text;

    if (__match_str(&t, t, textend, "--", 2) == 2
            && __read_iso8601_MM_DD(val, &t, t, textend) > 0)
        return (int)((*next = t) - text);
    else
        return 0;
}

/* --MMDD */
static int __read_iso8601___MMDD(
    _date *val,
    _str *next, _str text, _str textend)
{
    _str t = text;

    if (__match_str(&t, t, textend, "--", 2) == 2
            && __read_iso8601_MMDD(val, &t, t, textend) > 0)
        return (int)((*next = t) - text);
    else
        return 0;
}

/* hhmmss[.sss] */
static int __read_iso8601_hhmmss(
    _time *val,
    _str *next, _str text, _str textend)
{
    _str t = text;
    _uint n;

    if (__read_dec(&n, &t, t, textend) == 6) {
        val->hour = n / 10000;
        val->minute = (n % 10000) / 100;
        val->second = n % 100;
        // if (__match_char_set(*next, ".,", 2) > 0
        //     && __read_dec(&n, &t, t, textend) > 0) {
        //     val->milisecond = ...;
        // }
        return (int)((*next = t) - text);
    }
    return 0;
}

/* hhmm[.mmm] */
static int __read_iso8601_hhmm(
    _time *val,
    _str *next, _str text, _str textend)
{
    _str t = text;
    _uint n;

    if (__read_dec(&n, &t, t, textend) == 4) {
        val->hour = n / 100;
        val->minute = n % 100;
        // if (__match_char_set(*next, ".,", 2) > 0
        //     && __read_dec(&n, &t, t, textend) > 0) {
        //     val->second = ...;
        // }
        return (int)((*next = t) - text);
    }
    return 0;
}

/* hh:mm:ss[.sss] */
static int __read_iso8601_hh_mm_ss(
    _time *val,
    _str *next, _str text, _str textend)
{
    _str t = text;

    if (__read_dec(&val->hour, &t, t, textend) > 0
        && __match_chars_set(&t, t, textend, ":", 1, 1, 1) > 0
        && __read_dec(&val->minute, &t, t, textend) > 0
        && __match_chars_set(&t, t, textend, ":", 1, 1, 1) > 0
        && __read_dec(&val->second, &t, t, textend) > 0) {
        // if (__match_char_set(*next, ".,", 2) > 0
        //     && __read_dec(&n, &t, t, textend) > 0) {
        //     val->milisecond = ...;
        // }
        return (int)((*next = t) - text);
    }
    return 0;
}

/* hh:mm[.mmm] */
static int __read_iso8601_hh_mm(
    _time *val,
    _str *next, _str text, _str textend)
{
    _str t = text;

    if (__read_dec(&val->hour, &t, t, textend) > 0
        && __match_chars_set(&t, t, textend, ":", 1, 1, 1) > 0
        && __read_dec(&val->minute, &t, t, textend) > 0) {
        // if (__match_char_set(*next, ".,", 2) > 0
        //     && __read_dec(&n, &t, t, textend) > 0) {
        //     val->milisecond = ...;
        // }
        return (int)((*next = t) - text);
    }
    return 0;
}

/* hh[.hhh] */
static int __read_iso8601_hh(
    _time *val,
    _str *next, _str text, _str textend)
{
    _str t = text;

    if (__read_dec(&val->hour, &t, t, textend) > 0) {
        // if (__match_char_set(*next, ".,", 2) > 0
        //     && __read_dec(&n, &t, t, textend) > 0) {
        //     val->milisecond = ...;
        // }
        return (int)((*next = t) - text);
    }
    return 0;
}

/* Z */
static int __read_iso8601_Z(
    _time *val,
    _str *next, _str text, _str textend)
{
    _str t = text;

    if (__match_char_set(*t, "Z", 1) > 0)
        return (int)((*next = ++t) - text);
    else
        return 0;
}

/* (+|-)hhmm */
static int __read_iso8601_Zhhmm(
    _time *val,
    _str *next, _str text, _str textend)
{
    _str t = text;
    _sint sign;
    _uint n;

    if (__read_iso8601_sign(&sign, &t, t, textend) > 0
        && __read_dec(&n, &t, t, textend) == 4) {
        val->hour = n / 100 * sign;
        val->minute = n % 100;
        return (int)((*next = t) - text);
    }
    return 0;
}

/* (+|-)hh:mm */
static int __read_iso8601_Zhh_mm(
    _time *val,
    _str *next, _str text, _str textend)
{
    _str t = text;
    _sint sign;

    if (__read_iso8601_sign(&sign, &t, t, textend) > 0
        && __read_dec(&val->hour, &t, t, textend) > 0
        && __match_chars_set(&t, t, textend, ":", 1, 1, 1) > 0
        && __read_dec(&val->minute, &t, t, textend) > 0) {
        val->hour *= sign;
        return (int)((*next = t) - text);
    }
    return 0;
}

/* (+|-)hh */
static int __read_iso8601_Zhh(
    _time *val,
    _str *next, _str text, _str textend)
{
    _str t = text;
    _sint sign;

    if (__read_iso8601_sign(&sign, &t, t, textend) > 0
        && __read_dec(&val->hour, &t, t, textend) > 0) {
        val->hour *= sign;
        return (int)((*next = t) - text);
    }
    return 0;
}


/* Read a modified ISO 8601 date, from the start of the text.

[out] return: Number of characters succesfully read.
              0 if none found.
              <0 if error; See cargv_err_t.
[out] val: Read value. Undefined on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textlen: Text to match.
[in] entire: If nonzero, text should end after matching, or fail.
*/
static int read_date_iso8601(
    _date *val,
    _str *next, _str text, _len len, int entire)
{
    static const int days_of_month[] = {
        0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    static const int date_of_month[] = {
        0, 1, 32, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335,
    };
    _str t = text, textend = text + len;
    int leap;

    memset(val, 0, sizeof(*val));

    if (!(__read_iso8601___MMDD(val, &t, t, textend) > 0
        || __read_iso8601___MM_DD(val, &t, t, textend) > 0
        || __read_iso8601_YYYYMMDD(val, &t, t, textend) > 0
        //|| __read_iso8601_YYYYDDD(val, &t, t, textend) > 0
        //|| __read_iso8601_YYYY_DDD(val, &t, t, textend) > 0
        || __read_iso8601_YYYY_MM_DD(val, &t, t, textend) > 0
        || __read_iso8601_YYYY_MM(val, &t, t, textend) > 0
        || __read_iso8601_YYYY(val, &t, t, textend) > 0))
        return 0;

    if (!__match_end(t, textend, entire))
        return 0;

    leap = val->month == 2
            && ((!(val->year % 4) && (val->year % 100)) || !(val->year % 400));
    if (!(val->year >= -9999 && val->year <= 9999
            && val->month >= 0 && val->month <= 12
            && val->day >= 0 && val->day <= days_of_month[val->month] + leap))
        return CARGV_VAL_OVERFLOW;

    return (int)((*next = t) - text);
}

/* Read a modified ISO 8601 time as UTC, from the start of the text.

[out] return: Number of characters succesfully read.
              0 if none found.
              <0 if error; See cargv_err_t.
[out] val: Read value. Undefined on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textlen: Text to match.
[in] entire: If nonzero, text should end after matching, or fail.
*/
static int read_time_iso8601(
    _time *val,
    _str *next, _str text, _len len, int entire)
{
    _str t = text, textend = text + len;
    _time tz;

    memset(val, 0, sizeof(*val));
    memset(&tz, 0, sizeof(tz));

    if (!(__read_iso8601_hhmmss(val, &t, t, textend) > 0
        || __read_iso8601_hhmm(val, &t, t, textend) > 0
        || __read_iso8601_hh_mm_ss(val, &t, t, textend) > 0
        || __read_iso8601_hh_mm(val, &t, t, textend) > 0
        || __read_iso8601_hh(val, &t, t, textend) > 0))
        return 0;
    /* 00:00..24:00 */
    if (!((val->hour == 24 && val->minute == 0 && val->second == 0
                && val->milisecond == 0) ||
          (val->hour >= 0 && val->hour < 24 && val->minute < 60
                && val->second < 60 && val->milisecond < 1000)))
        return CARGV_VAL_OVERFLOW;

    if (!(__read_iso8601_Z(&tz, &t, t, textend) > 0
        || __read_iso8601_Zhhmm(&tz, &t, t, textend) > 0
        || __read_iso8601_Zhh_mm(&tz, &t, t, textend) > 0
        || __read_iso8601_Zhh(&tz, &t, t, textend) > 0)) {
        return 0;
    }
    /* -1200..+1200 */
    if (!(((tz.hour == -12 || tz.hour == 12) && tz.minute == 0) ||
            (tz.hour > -12 && tz.hour < 12 && tz.minute < 60)))
        return CARGV_VAL_OVERFLOW;

    if (!__match_end(t, textend, entire))
        return 0;

    if (tz.hour < 0) {
        cargv_int_t m = val->minute + tz.minute;
        val->hour += -tz.hour + m / 60;
        val->minute = m % 60;
    }
    else {
        cargv_int_t m = val->minute - tz.minute;
        val->minute = (m + 60) % 60;
        val->hour -= (59 - m) / 60;
        val->hour -= tz.hour;
    }

    return (int)((*next = t) - text);
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
[out] val: Read value. Undefined on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textlen: Text to match.
[in] entire: If nonzero, text should end after matching, or fail.
*/
static int read_degree_iso6709(
    _degree *val,
    _str *next, _str text, _len textlen, int entire)
{
    _str t = text, tend = text + textlen;
    _uint a, b;
    int alen, blen;

    memset(val, 0, sizeof(*val));

    /* sign is mandatory */
    if (!__match_chars_set(&t, t, tend, "+-", 2, 1, 1))
        return 0;

    if ((alen = __read_dec(&a, &t, t, tend)) <= 0)
        return 0;

    /* dot is optinal */
    if (__match_chars_set(&t, t, tend, ".", 1, 1, 1)) {
        if (!((blen = __read_dec(&b, &t, t, tend)) > 0))
            return 0;
    }
    else {
        b = 0;
        blen = 0;
    }
    if (!__match_end(t, tend, entire))
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

    return (int)((*next = t) - text);
}

/* Read a ISO 6709 geocoord, from the start of the text.

    LATITUDELONGITUDE[/]

[out] return: Number of characters succesfully read.
              0 if none found.
              <0 if error; See cargv_err_t.
[out] val: Read value. Undefined on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textlen: Text to match.
[in] entire: If nonzero, text should end after matching, or fail.
*/
static int read_geocoord_iso6709(
    _geocoord *val,
    _str *next, _str text, _len textlen, int entire)
{
    _str t = text, tend = text + textlen;
    int a, b;

    memset(val, 0, sizeof(*val));

    if ((a = read_degree_iso6709(&val->latitude, &t, t, tend-t, 0)) == 0)
        return 0;
    if ((b = read_degree_iso6709(&val->longitude, &t, t, tend-t, 0)) == 0)
        return 0;
    __match_chars_set(&t, t, tend, "/", 1, 0, 1);   /* optional solidus */
    if (!__match_end(t, tend, entire))
        return 0;

    if (a < 0 || b < 0
            || !(val->latitude.deg >= -90000000
            && val->latitude.deg <= 90000000
            && val->longitude.deg >= -180000000
            && val->longitude.deg <= 180000000))
        return CARGV_VAL_OVERFLOW;

    return (int)((*next = t) - text);
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
                "%s: %s `%s` overflows, which is `%s`.\n",
                cargv->name, type, name, arg);
        }
        else {
            fprintf(stderr,
                "%s: Unknown error reading %s `%s`, which is `%s`.\n",
                cargv->name, type, name, arg);
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

int cargv_len(const struct cargv_t *cargv)
{
    return (int)(cargv->argend - cargv->args);
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
        = __match_str(&o, o, oend, opt_short_prefix, opt_short_prefix_len)
        && __match_str(&o, o, oend, opt_wildcard, opt_wildcard_len)
        && __match_end(o, oend, 1);

    /* Split optlist */
    o = optlist;
    __unmatch_str(&lopt, o, oend, opt_long_prefix, opt_long_prefix_len);

    /* Argument is long option, like `--long-option` */
    a = *cargv->args;
    if (__match_str(&a, a, aend, opt_long_prefix, opt_long_prefix_len)) {
        if (!(a < aend))
            return 0;   /* -- */
        if (wildcard)
            return 1;

        /* Iterate long options */
        o = lopt;
        while (o < oend) {
            o += opt_long_prefix_len;   /* already matched */
            __unmatch_str(&lopt, o, oend, opt_long_prefix, opt_long_prefix_len);
            if (__match_str(&a, a, aend, o, lopt-o) && __match_end(a, aend, 1))
                return 1;
            o = lopt;
        }
    }
    /* Argument is short option, like `-s` */
    else if (__match_str(&a, a, aend, opt_short_prefix, opt_short_prefix_len)) {
        if (!(a < aend))
            return 0;   /* - */
        if (wildcard)
            return 1;

        /* Find in short options */
        o = optlist;
        if (__match_str(&o, o, lopt, opt_short_prefix, opt_short_prefix_len)
                && __match_chars_set(&a, a, aend, o, lopt-o, 1, lopt-o)
                && __match_end(a, aend, 1))
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
            __unmatch_str(&tend, t, listend, sep, seplen);
            if (__match_str(&a, a, aend, t, tend-t) && __match_end(a, aend, 1))
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

    v = vals;
    a = cargv->args;
    while (v - vals < valc && a < cargv->argend) {
        r = read_time_iso8601(v, &e, *a, strlen(*a), 1);
        if (r < 0)
            return err_val_result(cargv, name, "time", *a, r);;
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
