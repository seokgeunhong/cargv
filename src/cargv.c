
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


/* Short internal names for exported types and constants */
typedef const char               *_str;
typedef cargv_len_t               _len;
typedef cargv_int_t               _sint;
typedef cargv_uint_t              _uint;
typedef cargv_real_t              _real;
typedef struct cargv_date_t       _ymd;
typedef struct _hms_t {
    _uint hour;    /* 0..24 */
    _uint minute;  /* 0..59 */
    _uint second;  /* 0..59 */
    _uint milisecond;  /* 0..999 */
} _hms;
typedef struct cargv_timezone_t   _tz;
typedef struct cargv_datetime_t   _datetime;
typedef struct cargv_degree_t     _degree;
typedef struct cargv_geocoord_t   _geocoord;

#define _SINT_MIN   CARGV_SINT_MIN
#define _SINT_MAX   CARGV_SINT_MAX
#define _UINT_MAX   CARGV_UINT_MAX


/* power of 10 */
static int __p10(int e)
{
    int x = 1;
    while (e-- > 0)
        x *= 10;
    return x;
}

/* See if text ran out.

[out] return: 1 if empty, else 0.
[in] text, textend: Text to match.
*/
static int __match_end(_str text, _str textend)
{
    return !(text < textend);
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
    if ((textend - text) >= patternlen
        && memcmp(text, pattern, patternlen) == 0) {
        *next = text + patternlen;
        return patternlen;
    }
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

/* Read a number sign, `+` or `-`.

[out] return: Number of characters succesfully read.
              0 if none found.
[out] val: -1 for `-`, 1 otherwise.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textend: Text to match.
*/
static int __read_sign(_sint *val, _str *next, _str text, _str textend)
{
    _str t = text;

    __match_chars_set(&t, t, textend, "+-", 2, 0, 1);
    *val = (*text == '-') ? -1 : 1;
    return (int)((*next = t) - text);
}

/* Read a decimal integer, without any sign. [0-9]+

[out] return: Number of characters succesfully read.
              0 if pattern not matched to a decimal number.
              CARGV_VAL_OVERFLOW if value exceeds CARGV_UINT_MAX.
[out] val: Read value. Untouched on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textend: Text to match.
*/
static int __read_dec(_uint *val, _str *next, _str text, _str textend)
{
    int r;
    _str t = text, s;
    _uint u;

    if (!((r = __match_chars_range(&t, t, textend, "09", 2, 0, 100)) != 0))
        return 0;

    *next = t;
    if (r < 0)
        return r;

    u = 0;
    for (s = text; s < t; s++) {
        if (u < _UINT_MAX / 10
            || (u == _UINT_MAX / 10 && *s - '0' <= _UINT_MAX % 10))
            u = u * 10 + (*s - '0');
        else
            return CARGV_VAL_OVERFLOW;
    }
    *val = u;
    return (int)(*next - text);
}

/* Read a signed decimal integer.

[out] return: Number of characters succesfully read.
              0 if none found.
              CARGV_VAL_OVERFLOW if not in [CARGV_SINT_MIN,CARGV_SINT_MAX].
[out] val: Read value. Untouched on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textend: Text to match.
*/
static int __read_sint_dec(_sint *val, _str *next, _str text, _str textend)
{
    int r;
    _str t;
    _sint sign;
    _uint u;

    /* [+-]<0-9>{..} */
    if (__read_sign(&sign, &t, (t = text), textend) >= 0
        && (r = __read_dec(&u, &t, t, textend)) != 0) {
    }
    else
        return 0;

    *next = t;
    if (r < 0)
        return r;

    if (sign > 0) {
        if (u <= (_uint)_SINT_MAX)
            *val = (_sint)u;
        else
            return CARGV_VAL_OVERFLOW;
    }
    else {
        if (u < (_uint)_SINT_MAX+1)
            *val = -(_sint)u;
        else if (u == (_uint)_SINT_MAX+1)
            *val = _SINT_MIN;
        else
            return CARGV_VAL_OVERFLOW;
    }
    return (int)(*next - text);
}

/* Read an unsigned decimal integer.

[out] return: Number of characters succesfully read.
              0 if none found.
              CARGV_VAL_OVERFLOW if not in [0,CARGV_UINT_MAX].
[out] val: Read value. Untouched on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textend: Text to match.
*/
static int __read_uint_dec(_uint *val, _str *next, _str text, _str textend)
{
    int r;
    _str t;
    _sint sign;
    _uint u;

    /* [+]<0-9>{..} */
    if (__read_sign(&sign, &t, (t = text), textend) >= 0
        && (r = __read_dec(&u, &t, t, textend)) != 0) {
    }
    else
        return 0;

    *next = t;
    if (sign < 0)
        return CARGV_VAL_OVERFLOW;
    if (r < 0)
        return r;

    *val = u;
    return (int)(*next - text);
}


static const struct cargv_date_t _YMD_DEFAULT
    = {CARGV_YEAR_DEFAULT, CARGV_MONTH_DEFAULT, CARGV_DAY_DEFAULT};
/*const struct cargv_date_t *CARGV_DATE_DEFAULT = &_YMD_DEFAULT;*/

static const struct _hms_t _HMS_DEFAULT = {
    CARGV_HOUR_DEFAULT, CARGV_MINUTE_DEFAULT, CARGV_SECOND_DEFAULT,
    CARGV_MILISECOND_DEFAULT,
};

static const struct cargv_timezone_t _TZ_DEFAULT
    = {CARGV_TZ_HOUR_DEFAULT, CARGV_TZ_MINUTE_DEFAULT};
const struct cargv_timezone_t *CARGV_TZ_LOCAL = &_TZ_DEFAULT;

/*
static const struct cargv_time_t _TIME_DEFAULT = {
    CARGV_HOUR_DEFAULT, CARGV_MINUTE_DEFAULT, CARGV_SECOND_DEFAULT,
    CARGV_MILISECOND_DEFAULT,
    {CARGV_TZ_HOUR_DEFAULT, CARGV_TZ_MINUTE_DEFAULT}
};
const struct cargv_time_t *CARGV_TIME_DEFAULT = &_TIME_DEFAULT;
*/

/*
static const struct cargv_datetime_t _DATETIME_DEFAULT = {
    CARGV_YEAR_DEFAULT, CARGV_MONTH_DEFAULT, CARGV_DAY_DEFAULT,
    CARGV_HOUR_DEFAULT, CARGV_MINUTE_DEFAULT, CARGV_SECOND_DEFAULT,
    CARGV_MILISECOND_DEFAULT,
    {CARGV_TZ_HOUR_DEFAULT, CARGV_TZ_MINUTE_DEFAULT}
};
const struct cargv_datetime_t *CARGV_DATETIME_DEFAULT = &_DATETIME_DEFAULT;
*/

static const struct cargv_timezone_t _UTC         = {+0,0};
static const struct cargv_timezone_t _TZ_SEOUL    = {+9,0};
const struct cargv_timezone_t *CARGV_UTC = &_UTC;
const struct cargv_timezone_t *CARGV_TZ_SEOUL = &_TZ_SEOUL;

/* return 1 if the month is leap month, otherwise 0. */
static int __leap(_sint year, _uint month)
{
    return month == 2 && ((!(year % 4) && (year % 100)) || !(year % 400));
}

static int __days_of_month_this_year(_uint month)
{
    static const int days_of_month[] = {
        0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    return days_of_month[month];
}
static int __days_of_month(_sint year, _uint month)
{
    static const int days_of_month[] = {
        0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    return days_of_month[month] + __leap(year, month);
}

/* Read a modified ISO 8601 year, month, and day, no omission.

[out] return: Number of characters succesfully read.
              0 if none found. `next` untouched.
              <0 if matched but wrong. `next` still progress.
[out] val: Read value. Untouched on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textend: Text to match.
*/
static int __read_iso8601_YMD(_ymd *val, _str *next, _str text, _str textend)
{
    int r;
    _str t;
    _sint sign, y;
    _uint m, d, n;

    /* [+-]YYYYMMDD */
    if (__read_sign(&sign, &t, (t = text), textend) >= 0
        && __read_dec(&n, &t, t, textend) == 8) {
        y = (_sint)n / 10000 * sign;
        m = (n % 10000) / 100;
        d = n % 100;
    }
    /* [+-]Y[..4]<-/>[M]M<-/>[D]D */
    else if (__read_sign(&sign, &t, (t = text), textend) >= 0
             && (r = __read_dec(&n, &t, t, textend)) > 0 && r <= 4
             && __match_chars_set(&t, t, textend, "-/", 2, 1, 1) == 1
             && (r = __read_dec(&m, &t, t, textend)) > 0 && r <= 2
             && __match_chars_set(&t, t, textend, "-/", 2, 1, 1) == 1
             && (r = __read_dec(&d, &t, t, textend)) > 0 && r <= 2) {
        y = (_sint)n * sign;;
    }
    else
        return 0;

    *next = t;

    /* -9999-1-1..+9999-12-31 */
    if (!(y >= -9999 && y <= 9999
          && m > 0 && m <= 12
          && d > 0 && d <= __days_of_month(y, m)))
        return CARGV_VAL_OVERFLOW;

    val->year = y;
    val->month = m;
    val->day = d;
    return (int)(*next - text);
}

/* Read a modified ISO 8601 year and month only.

[out] return: Number of characters succesfully read.
              0 if none found. `next` untouched.
              <0 if matched but wrong. `next` still progress.
[out] val: Read value. Untouched on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textend: Text to match.
*/
static int __read_iso8601_YM(_ymd *val, _str *next, _str text, _str textend)
{
    int r;
    _str t;
    _sint sign, y;
    _uint m, n;

    /* [+-]Y[..4]<-/>[M]M */
    if (__read_sign(&sign, &t, (t = text), textend) >= 0
        && (r = __read_dec(&n, &t, t, textend)) > 0 && r <= 4
        && __match_chars_set(&t, t, textend, "-/", 2, 1, 1) == 1
        && (r = __read_dec(&m, &t, t, textend)) > 0 && r <= 2) {
        y = (_sint)n * sign;
    }
    else
        return 0;

    *next = t;

    /* -9999-01..+9999-12 */
    if (!(y >= -9999 && y <= 9999 && m > 0 && m <= 12))
        return CARGV_VAL_OVERFLOW;

    val->year = y;
    val->month = m;
    val->day = CARGV_DAY_DEFAULT;
    return (int)(*next - text);
}

/* Read a modified ISO 8601 year only.

[out] return: Number of characters succesfully read.
              0 if none found. `next` untouched.
              <0 if matched but wrong. `next` still progress.
[out] val: Read value. Untouched on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textend: Text to match.
*/
static int __read_iso8601_Y(_ymd *val, _str *next, _str text, _str textend)
{
    int r;
    _str t;
    _sint sign, y;
    _uint n;

    /* [+-]Y[..4] */
    if (__read_sign(&sign, &t, (t = text), textend) >= 0
        && (r = __read_dec(&n, &t, t, textend)) > 0 && r <= 4) {
        y = (_sint)n * sign;
    }
    else
        return 0;
        
    *next = t;

    /* -9999..+9999 */
    if (!(y >= -9999 && y <= 9999))
        return CARGV_VAL_OVERFLOW;

    val->year = y;
    val->month = CARGV_MONTH_DEFAULT;
    val->day = CARGV_DAY_DEFAULT;
    return (int)(*next - text);
}

/* Read a modified ISO 8601 month and day only.

[out] return: Number of characters succesfully read.
              0 if none found. `next` untouched.
              <0 if matched but wrong. `next` still progress.
[out] val: Read value. Untouched on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textend: Text to match.
*/
static int __read_iso8601_MD(_ymd *val, _str *next, _str text, _str textend)
{
    int r;
    _str t;
    _uint m, d, n;

    /* --MMDD */
    if (__match_str(&t, (t = text), textend, "--", 2) == 2
        && __read_dec(&n, &t, t, textend) == 4) {
        d = n % 100;
        m = n / 100;
    }
    /* --[M]M<-/>[D]D */
    else if (__match_str(&t, (t = text), textend, "--", 2) == 2
             && (r = __read_dec(&m, &t, t, textend)) > 0 && r <= 2
             && __match_chars_set(&t, t, textend, "-/", 2, 1, 1) == 1
             && (r = __read_dec(&d, &t, t, textend)) > 0 && r <= 2) {
    }
    else
        return 0;

    *next = t;

    /* 1-1..12-31 */
    if (!(m > 0 && m <= 12 && d > 0 && d <= __days_of_month_this_year(m)))
        return CARGV_VAL_OVERFLOW;

    val->year = CARGV_YEAR_DEFAULT;
    val->month = m;
    val->day = d;
    return (int)(*next - text);
}

/* `Z` */
static int __read_iso8601_Z(_tz *val, _str *next, _str text, _str textend)
{
    _str t = text;

    if (!(__match_chars_set(&t, t, textend, "Z", 1, 1, 1) > 0))
        return 0;

    memcpy(val, &_UTC, sizeof(*val));
    return (int)((*next = t) - text);
}

static int __read_iso8601_Zhm(_tz *val, _str *next, _str text, _str textend)
{
    int r;
    _str t;
    _sint sign;
    _uint h, m, n;

    /* <+->hhmm */
    if (__read_sign(&sign, &t, (t = text), textend) == 1
        && __read_dec(&n, &t, t, textend) == 4) {
        h = n / 100;
        m = n % 100;
    }
    /* <+->h{1..2}:m{1..2} */
    else if (__read_sign(&sign, &t, (t = text), textend) == 1
             && (r = __read_dec(&h, &t, t, textend)) > 0 && r <= 2
             && __match_chars_set(&t, t, textend, ":", 2, 1, 1) == 1
             && (r = __read_dec(&m, &t, t, textend)) > 0 && r <= 2) {
    }
    else
        return 0;

    *next = t;

    /* -12:00..+12:00 */
    if (!((h == 12 && m == 0) || (h < 12 && m < 60)))
        return CARGV_VAL_OVERFLOW;

    val->hour = sign * (_sint)h;
    val->minute = sign * (_sint)m;
    return (int)(*next - text);
}

static int __read_iso8601_Zh(_tz *val, _str *next, _str text, _str textend)
{
    int r;
    _str t;
    _sint sign;
    _uint h;

    /* <+->h{1..2} */
    if (__read_sign(&sign, &t, (t = text), textend) == 1
        && (r = __read_dec(&h, &t, t, textend)) > 0 && r <= 2) {
    }
    else
        return 0;

    *next = t;

    /* -12..+12 */
    if (!(h <= 12))
        return CARGV_VAL_OVERFLOW;

    val->hour = sign * (_sint)h;
    val->minute = 0;
    return (int)(*next - text);
}

/* Read a modified ISO 8601 timezone.

[out] return: Number of characters succesfully read.
              0 if none found. `next` untouched.
              <0 if matched but wrong. `next` still progress.
[out] val: Read value. Untouched on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textend: Text to match.
*/
static int __read_iso8601_tz(_tz *val, _str *next, _str text, _str textend)
{
    int r;
    _str t;
    _tz v;

    if ((r = __read_iso8601_Z(&v, &t, (t = text), textend)) == 0
        && (r = __read_iso8601_Zhm(&v, &t, (t = text), textend)) == 0
        && (r = __read_iso8601_Zh(&v, &t, (t = text), textend)) == 0)
        return 0;

    *next = t;
    if (r < 0)
        return r;

    memcpy(val, &v, sizeof(*val));
    return (int)(*next - text);
}

/* Read a modified ISO 8601 hour, minute, and second, no omission.

[out] return: Number of characters succesfully read.
              0 if none found. `next` untouched.
              <0 if matched but wrong. `next` still progress.
[out] val: Read value. Untouched on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textend: Text to match.
*/
static int __read_iso8601_hms(_hms *val, _str *next, _str text, _str textend)
{
    int r;
    _str t;
    _uint h, m, s, n;

    /* hhmmss[<.,>s[..3]] */
    if (__read_dec(&n, &t, (t = text), textend) == 6) {
        h = n / 10000;
        m = (n % 10000) / 100;
        s = n % 100;
    }
    /* [h]h:[m]m:[s]s[<.,>s[..3]] */
    else if ((r = __read_dec(&h, &t, (t = text), textend)) > 0 && r <= 2
             && __match_chars_set(&t, t, textend, ":", 1, 1, 1) == 1
             && (r = __read_dec(&m, &t, t, textend)) > 0 && r <= 2
             && __match_chars_set(&t, t, textend, ":", 1, 1, 1) == 1
             && (r = __read_dec(&s, &t, t, textend)) > 0 && r <= 2) {
    }
    else
        return 0;

    *next = t;

    /* 00:00:00..24:00:00 */
    if (!((h == 24 && m == 0 && s == 0) || (h < 24 && m < 60 && s < 60)))
        return CARGV_VAL_OVERFLOW;

    val->hour = h;
    val->minute = m;
    val->second = s;
    val->milisecond = 0;
    return (int)(*next - text);
}

/* Read a modified ISO 8601 hour and minute.

[out] return: Number of characters succesfully read.
              0 if none found. `next` untouched.
              <0 if matched but wrong. `next` still progress.
[out] val: Read value. Untouched on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textend: Text to match.
*/
static int __read_iso8601_hm(_hms *val, _str *next, _str text, _str textend)
{
    int r;
    _str t;
    _uint h, m, n;

    /* hhmm[.mmm] */
    if (__read_dec(&n, &t, (t = text), textend) == 4) {
        h = n / 100;
        m = n % 100;
    }
    /* [h]h:[m]m[.mmm] */
    else if ((r = __read_dec(&h, &t, (t = text), textend)) > 0 && r <= 2
             && __match_chars_set(&t, t, textend, ":", 1, 1, 1) == 1
             && (r = __read_dec(&m, &t, t, textend)) > 0 && r <= 2) {
    }
    else
        return 0;

    *next = t;

    /* 00:00..24:00 */
    if (!((h == 24 && m == 0) || (h < 24 && m < 60)))
        return CARGV_VAL_OVERFLOW;

    val->hour = h;
    val->minute = m;
    val->second = 0;
    val->milisecond = 0;
    return (int)(*next - text);
}

/* Read a modified ISO 8601 hour only.

[out] return: Number of characters succesfully read.
              0 if none found. `next` untouched.
              <0 if matched but wrong. `next` still progress.
[out] val: Read value. Untouched on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textend: Text to match.
*/
static int __read_iso8601_h(_hms *val, _str *next, _str text, _str textend)
{
    int r;
    _str t;
    _uint h, n;

    /* [h]h[.hhh] */
    if ((r = __read_dec(&h, &t, (t = text), textend)) > 0 && r <= 2) {
    }
    else
        return 0;

    *next = t;

    /* 00..24 */
    if (!(h <= 24))
        return CARGV_VAL_OVERFLOW;

    val->hour = h;
    val->minute = 0;
    val->second = 0;
    val->milisecond = 0;
    return (int)(*next - text);
}

/* Read a modified ISO 6709 degree.

[out] return: Number of characters succesfully read.
              0 if none found. `next` untouched.
              <0 if matched but wrong. `next` still progress.
[out] val: Read value. Untouched on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textend: Text to match.
*/
static int __read_iso6709_degree(
    _degree *val, _str *next, _str text, _str textend)
{
    _str t, tf;
    int rn, rf;
    _sint sign;
    _uint n, f;
    _sint d, m, s, df, mf, sf;

    /* <+-> */
    if (!(__read_sign(&sign, &t, (t = text), textend) == 1))
        return 0;

    /* Integer part: d[..] */
    if (!((rn = __read_dec(&n, &t, t, textend)) > 0))
        return 0;

    /* Fraction part: [<.,>d[..]] */
    if (__match_chars_set(&tf, (tf = t), textend, ".,", 2, 1, 1) == 1
        && (rf = __read_dec(&f, &tf, tf, textend)) > 0) {
        t = tf;
    }
    else {
        rf = 0;
        f = 0;
    }

    *next = t;

    /* D[..3][<.,>D[..6]] */
    if (rn >= 1 && rn <= 3 && rf <= 6) {
        d = (_sint)n;
        df = f / __p10(rf-6) * __p10(6-rf);
        m = mf = 0;
        s = sf = 0;
    }
    /* [D]DDMM[<.,>M[..4]] */
    else if (rn >= 4 && rn <= 5 && rf <= 4) {
        d = (_sint)n / 100;
        df = 0;
        m = (_sint)n % 100;
        mf = f / __p10(rf-4) * __p10(4-rf);
        s = sf = 0;
    }
    /* [D]DDMMSS[<.,>S[..2]] */
    else if (rn >= 6 && rn <= 7 && rf <= 2) {
        d = (_sint)n / 10000;
        df = 0;
        m = (_sint)n / 100 % 100;
        mf = 0;
        s = (_sint)n % 100;
        sf = f / __p10(rf-2) * __p10(2-rf);
    }
    else
        return CARGV_VAL_OVERFLOW;

    if (!(d <= 360 && m <= 60 && s <= 60))
        return CARGV_VAL_OVERFLOW;

    val->degree = sign * d;
    val->minute = sign * m;
    val->second = sign * s;
    val->microdegree = sign * df;
    val->microminute = sign * mf * 100;
    val->microsecond = sign * sf * 10000;
    return (int)(*next - text);
}

/* Read a modified ISO 6709 geocoord.

[out] return: Number of characters succesfully read.
              0 if none found. `next` untouched.
              <0 if matched but wrong. `next` still progress.
[out] val: Read value. Untouched on failure.
[out] next: Points end of matched text. Untouched if no match found.
[in] text, textend: Text to match.
*/
static int __read_iso6709_geocoord(
    _geocoord *val, _str *next, _str text, _str textend)
{
    _str t;
    int ry, rx;
    _degree x, y;

    /* <latitude><longitude>[/] */
    if ((ry = __read_iso6709_degree(&y, &t, (t = text), textend)) != 0
        && (rx = __read_iso6709_degree(&x, &t, t, textend)) != 0
        && __match_chars_set(&t, t, textend, "/", 1, 0, 1) >= 0) {
    }
    else
        return 0;

    *next = t;

    if (ry < 0)
        return ry;
    if (rx < 0)
        return ry;
    if (!(y.degree >= -90 && y.degree <= 90
          && x.degree >= -180 && x.degree <= 180))
        return CARGV_VAL_OVERFLOW;

    memcpy(&val->latitude, &y, sizeof(val->latitude));
    memcpy(&val->longitude, &x, sizeof(val->longitude));
    return (int)(*next - text);
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
    wildcard = __match_str(&o, o, oend, "-", 1)
               && __match_str(&o, o, oend, "*", 1)
               && __match_end(o, oend);

    /* Split optlist */
    o = optlist;
    __unmatch_str(&lopt, o, oend, "--", 2);

    /* Argument is long option, like `--long-option` */
    a = *cargv->args;
    if (__match_str(&a, a, aend, "--", 2)) {
        if (__match_end(a, aend))
            return 0;   /* -- */
        if (wildcard)
            return 1;

        /* Iterate long options */
        o = lopt;
        while (o < oend) {
            o += 2;   /* already matched */
            __unmatch_str(&lopt, o, oend, "--", 2);
            if (__match_str(&a, a, aend, o, lopt-o) && __match_end(a, aend))
                return 1;
            o = lopt;
        }
    }
    /* Argument is short option, like `-s` */
    else if (__match_str(&a, a, aend, "-", 1)) {
        if (__match_end(a, aend))
            return 0;   /* - */
        if (wildcard)
            return 1;

        /* Find in short options */
        o = optlist;
        if (__match_str(&o, o, lopt, "-", 1)
            && __match_chars_set(&a, a, aend, o, lopt-o, 1, lopt-o)
            && __match_end(a, aend))
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

    val = vals;
    arg = cargv->args;
    while (val - vals < valc && arg < cargv->argend) {
        aend = *arg + strlen(*arg);
        a = *arg;
        t = list;
        while (t < listend) {
            __unmatch_str(&tend, t, listend, sep, seplen);
            if (__match_str(&a, a, aend, t, tend-t) && __match_end(a, aend))
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
    _sint *v, n;
    _str *a, t, e;

    v = vals;
    a = cargv->args;
    while (v - vals < valc && a < cargv->argend) {
        t = *a;
        e = t + strlen(t);
        if ((r = __read_sint_dec(&n, &t, t, e)) == 0)
            break;
        if (!__match_end(t, e))
            break;
        if (r < 0)
            return err_val_result(cargv, name, "integer", *a, r);

        *v++ = n;
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
    _uint *v, n;
    _str *a, t, e;

    v = vals;
    a = cargv->args;
    while (v - vals < valc && a < cargv->argend) {
        t = *a;
        e = t + strlen(t);
        if ((r = __read_uint_dec(&n, &t, t, e)) == 0)
            break;
        if (!__match_end(t, e))
            break;
        if (r < 0)
            return err_val_result(cargv, name, "unsigned integer", *a, r);

        *v++ = n;
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
    _ymd *v, d;
    _str *a, t, e;

    v = vals;
    a = cargv->args;
    while (v - vals < valc && a < cargv->argend) {
        t = *a;
        e = t + strlen(t);
        if ((r = __read_iso8601_YMD(&d, &t, t, e)) == 0
            && (r = __read_iso8601_YM(&d, &t, t, e)) == 0
            && (r = __read_iso8601_Y(&d, &t, t, e)) == 0
            && (r = __read_iso8601_MD(&d, &t, t, e)) == 0)
            break;
        if (!__match_end(t, e))
            break;
        if (r < 0)
            return err_val_result(cargv, name, "date", *a, r);

        memcpy(v++, &d, sizeof(*v));
        a++;
    }
    return (int)(v-vals);
}

int cargv_time(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_time_t *vals, cargv_len_t valc)
{
    int rh, rz;
    struct cargv_time_t *v;
    _hms h;
    _tz z;
    _str *a, t, e;

    for (a = cargv->args, v = vals; v - vals < valc && a < cargv->argend;) {
        t = *a;
        e = t + strlen(t);

        /* <hms>[z] */
        if ((rh = __read_iso8601_hms(&h, &t, t, e)) == 0
            && (rh = __read_iso8601_hm(&h, &t, t, e)) == 0
            && (rh = __read_iso8601_h(&h, &t, t, e)) == 0)
            break;
        if ((rz = __read_iso8601_tz(&z, &t, t, e)) == 0)
            memcpy(&z, &_TZ_DEFAULT, sizeof(z));
        if (!__match_end(t, e))
            break;
        if (rh < 0)
            return err_val_result(cargv, name, "time", *a, rh);
        if (rz < 0)
            return err_val_result(cargv, name, "time", *a, rz);

        v->hour = h.hour;
        v->minute = h.minute;
        v->second = h.second;
        v->milisecond = h.milisecond;
        memcpy(&v->tz, &z, sizeof(v->tz));
        ++v;
        ++a;
    }
    return (int)(v-vals);
}

int cargv_timezone(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_timezone_t *vals, cargv_len_t valc)
{
    int r;
    _tz *v, z;
    _str *a, t, e;

    for (a = cargv->args, v = vals; v - vals < valc && a < cargv->argend;) {
        t = *a;
        e = t + strlen(t);
        if ((r = __read_iso8601_tz(&z, &t, t, e)) == 0)
            break;
        if (!__match_end(t, e))
            break;
        if (r < 0)
            return err_val_result(cargv, name, "timezone", *a, r);

        memcpy(v++, &z, sizeof(*v));
        ++a;
    }
    return (int)(v-vals);
}

int cargv_datetime(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_datetime_t *vals, cargv_len_t valc)
{
    int r;
    _datetime *v;
    _ymd d;
    _hms h;
    _tz z;
    _str *a, t, e;
    int rd = 0, rh = 0, rz = 0;

    for (a = cargv->args, v = vals; v - vals < valc && a < cargv->argend;) {
        t = *a;
        e = t + strlen(t);

        /* <date>T<time>[tz] */
        if (((rd = __read_iso8601_YMD(&d, &t, (t = *a), e)) != 0
            || (rd = __read_iso8601_YM(&d, &t, t, e)) != 0
            || (rd = __read_iso8601_Y(&d, &t, t, e)) != 0
            || (rd = __read_iso8601_MD(&d, &t, t, e)) != 0)
            && __match_chars_set(&t, t, e, "T ", 2, 1, 1) == 1
            && ((rh = __read_iso8601_hms(&h, &t, t, e)) != 0
                || (rh = __read_iso8601_hm(&h, &t, t, e)) != 0
                || (rh = __read_iso8601_h(&h, &t, t, e)) != 0)) {
            if ((rz = __read_iso8601_tz(&z, &t, t, e)) == 0)
                memcpy(&z, &_TZ_DEFAULT, sizeof(z));
        }
        /* <date> */
        else if ((rd = __read_iso8601_YMD(&d, &t, (t = *a), e)) != 0
                || (rd = __read_iso8601_YM(&d, &t, t, e)) != 0
                || (rd = __read_iso8601_MD(&d, &t, t, e)) != 0) {
            memcpy(&h, &_HMS_DEFAULT, sizeof(h));
            memcpy(&z, &_TZ_DEFAULT, sizeof(z));
        }
        /* <time>[tz] */
        else if ((rh = __read_iso8601_hms(&h, &t, (t = *a), e)) != 0
                 || (rh = __read_iso8601_hm(&h, &t, t, e)) != 0) {
            memcpy(&d, &_YMD_DEFAULT, sizeof(d));
            if ((rz = __read_iso8601_tz(&z, &t, t, e)) == 0)
                memcpy(&z, &_TZ_DEFAULT, sizeof(z));
        }
        /* <hour><tz> */
        else if ((rh = __read_iso8601_h(&h, &t, t, e)) != 0
                 && (rz = __read_iso8601_tz(&z, &t, t, e)) != 0) {
            memcpy(&d, &_YMD_DEFAULT, sizeof(d));
        }
        /* <year> */
        else if ((rd = __read_iso8601_Y(&d, &t, (t = *a), e)) != 0) {
            memcpy(&h, &_HMS_DEFAULT, sizeof(h));
            memcpy(&z, &_TZ_DEFAULT, sizeof(z));
        }
        else
            break;

        if (!__match_end(t, e))
            break;
        if (rd < 0)
            return err_val_result(cargv, name, "datetime", *a, rd);
        if (rh < 0)
            return err_val_result(cargv, name, "datetime", *a, rh);
        if (rz < 0)
            return err_val_result(cargv, name, "datetime", *a, rz);

        v->year = d.year;
        v->month = d.month;
        v->day = d.day;
        v->hour = h.hour;
        v->minute = h.minute;
        v->second = h.second;
        v->milisecond = h.milisecond;
        memcpy(&v->tz, &z, sizeof(v->tz));

        ++v;
        ++a;
    }
    return (int)(v-vals);
}

enum cargv_err_t cargv_convert_localtime(
    struct cargv_datetime_t *dst,
    const struct cargv_datetime_t *src,
    const struct cargv_timezone_t *tz)
{
    static const int days_of_month[] = {
        0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    _sint year, month, day, hour, minute;
    _sint tzh, tzm;

    year = (_sint)src->year;
    month = (_sint)src->month;
    day = (_sint)src->day;
    hour = (_sint)src->hour;
    minute = (_sint)src->minute;
    tzh = -src->tz.hour + tz->hour;
    tzm = -src->tz.minute + tz->minute;

    hour += 24 + tzh;
    minute += 60 + tzm;
    hour += minute / 60 - 1;
    if (day > 0) {  /* date included */
        day += hour / 24 - 1;
        if (day < 1) {
            month -= 1;
            if (month < 1) {
                year -= 1;
                if (year < -9999)
                    return CARGV_VAL_OVERFLOW;
                month = 12;
            }
            day = days_of_month[month] + __leap(year, month);
        }
        else if (day > days_of_month[month] + __leap(year, month)) {
            month += 1;
            if (month > 12) {
                year += 1;
                if (year > 9999)
                    return CARGV_VAL_OVERFLOW;
                month = 1;
            }
            day = 1;
        }
    }
    hour %= 24;
    minute %= 60;

    dst->year = year;
    dst->month = (_uint)month;
    dst->day = (_uint)day;
    dst->hour = hour;
    dst->minute = (_uint)minute;
    dst->second = src->second;
    dst->milisecond = src->milisecond;
    memcpy(&dst->tz, tz, sizeof(dst->tz));
    return CARGV_OK;
}


int cargv_degree(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_degree_t *vals, cargv_len_t valc)
{
    int r;
    _degree *v;
    _str *a, t, e;

    for (v = vals, a = cargv->args; v - vals < valc && a < cargv->argend;) {
        t = *a;
        e = t + strlen(t);
        if ((r = __read_iso6709_degree(v, &t, t, e)) == 0)
            break;
        if (!__match_end(t, e))
            break;
        if (r < 0)
            return err_val_result(cargv, name, "degree", *a, r);

        ++v;
        ++a;
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
    _str *a, t, e;

    for (v = vals, a = cargv->args; v - vals < valc && a < cargv->argend;) {
        t = *a;
        e = t + strlen(t);
        if ((r = __read_iso6709_geocoord(v, &t, t, e)) == 0)
            break;
        if (!__match_end(t, e))
            break;
        if (r < 0)
            return err_val_result(cargv, name, "geocoord", *a, r);

        ++v;
        ++a;
    }
    return (int)(v-vals);
}


_real cargv_get_degree(const struct cargv_degree_t *val)
{
    return (_real)val->degree / 1E+6
        + (_real)val->minute / 1E+6 / 60.0
        + (_real)val->second / 1E+6 / 3600.0;
}
