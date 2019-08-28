/* cargv - A command line argument parser.
*/

#ifndef __cargv_h__
#define __cargv_h__

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
  #define CARGV_EXPORT extern "C"
#else
  #define CARGV_EXPORT
#endif


/* Version */
typedef uint32_t cargv_version_num_t;

struct cargv_version_t {
    short major, minor, patch, state;
};


/* Common types */
typedef ptrdiff_t   cargv_len_t;


/* cargv object */
struct cargv_t {
    const char *name;
    const char **args, **argend;
};

/* Common exit code */
enum cargv_err_t {
    CARGV_OK    = 0,
    CARGV_VAL_OVERFLOW  = -1,   /* value is well-formed but wrong */
};


/* Integral */
typedef int64_t     cargv_int_t;
typedef uint64_t    cargv_uint_t;

#define CARGV_SINT_MIN  INT64_MIN
#define CARGV_SINT_MAX  INT64_MAX
#define CARGV_UINT_MAX  UINT64_MAX


/* Datetime */
struct cargv_date_t {
    cargv_int_t year;    /* -9999..9999 */
    cargv_uint_t month;  /* 0..12, 0 if omitted */
    cargv_uint_t day;    /* 0..31, 0 if omitted */
};

struct cargv_timezone_t {
    cargv_int_t hour;    /* -12..12 */
    cargv_int_t minute;  /* -59..59 */
};

struct cargv_time_t {
    cargv_uint_t hour;    /* 0..24 */
    cargv_uint_t minute;  /* 0..59 */
    cargv_uint_t second;  /* 0..59 */
    cargv_uint_t milisecond;  /* 0..999 */
    struct cargv_timezone_t tz;
};

struct cargv_datetime_t {
    cargv_int_t year;    /* -9999..9999 */
    cargv_uint_t month;  /* 0..12, 0 if omitted */
    cargv_uint_t day;    /* 0..31, 0 if omitted */
    cargv_uint_t hour;    /* 0..24 */
    cargv_uint_t minute;  /* 0..59 */
    cargv_uint_t second;  /* 0..59 */
    cargv_uint_t milisecond;  /* 0..999 */
    struct cargv_timezone_t tz;
};

#define CARGV_THIS_YEAR   INT64_MIN
#define CARGV_THIS_MONTH  0
#define CARGV_THIS_DAY    0
extern const struct cargv_date_t *CARGV_TODAY;

#define CARGV_TZ_LOCAL_HOUR   INT64_MIN
#define CARGV_TZ_LOCAL_MINUTE INT64_MIN
extern const struct cargv_timezone_t *CARGV_TZ_LOCAL;

#define CARGV_THIS_HOUR        UINT64_MAX
#define CARGV_THIS_MINUTE      UINT64_MAX
#define CARGV_THIS_SECOND      UINT64_MAX
#define CARGV_THIS_MILISECOND  UINT64_MAX
extern const struct cargv_time_t *CARGV_NOW;

extern const struct cargv_datetime_t *CARGV_TODAY_NOW;

extern const struct cargv_timezone_t *CARGV_UTC;
extern const struct cargv_timezone_t *CARGV_TZ_SEOUL;


/* Geocoord */
struct cargv_degree_t {
    cargv_int_t deg;  /* ddd.dddddd */
    cargv_int_t min;  /*  mm.mmmmmm */
    cargv_int_t sec;  /*  ss.ssssss */
};

struct cargv_geocoord_t {
    struct cargv_degree_t latitude, longitude;
    /* int altitude; */
};


/* Get cargv version info.

[out] return: 32bit version number. Newer version has bigger number.
[out] version: Contains version info., if not null.
*/
CARGV_EXPORT
cargv_version_num_t cargv_version(struct cargv_version_t *version);

/* Get human readable version string. */
CARGV_EXPORT
const char *cargv_version_string();


/* Initialize cargv object.

[out] return: 0 if succeeded, <0 if error. See cargv_err_t.
[out] cargv:  cargv object.
[in]  name:   Display name of the program, used in error messages.
[in]  argc, argv: Passed from main().
*/
CARGV_EXPORT
enum cargv_err_t cargv_init(
    struct cargv_t *cargv,
    const char *name,
    int argc, const char **argv);

/* Get number of arguments remained.

[out] return: Number of arguments.
[in]  cargv:  cargv object.
*/
CARGV_EXPORT
int cargv_len(const struct cargv_t *cargv);

/* Shift: remove the first N arguments from the argument list.

[out] return: Number of arguments removed.
              0 if arguments are less than requested, and also untouched.
[in]  cargv:  cargv object.
[in]  argc:   Number of arguments to remove.
*/
CARGV_EXPORT
int cargv_shift(struct cargv_t *cargv, cargv_len_t argc);

/* See if the first argument is found in an option list.

Short options:  Options with short prefix(defaults `-`) and a character
                [a-zA-Z0-9]. These options can be combined in one argument,
                like `-axvf`, which is same as `-a -x -c -f`.

Long options:   Options with long prefix(defaults `--`) and a name, consist of
                words of characters [a-zA-Z0-9], optionally combined by `-`.
                `-` should not be used twice. e.g. `--a-very-long-option`

[out] return:   1 if an option matched.
                0 if not matched.
                <0 if error. See cargv_err_t.
[in]  cargv:    cargv object.
[in]  optlist:  An option list which is constist of two parts. First part is
                a combination of short options, and second part is a list of
                long options. Every part is optional.
                Specially, `-*` means any options.

    `-*`        Any options start with `-` or `--`
    `-axvf`     `-a`, `-x`, `-c`, -`f`, or any of their combinations.
    `-h--help`  `-h`, or `--help`
    `--1st-option--2nd-option`  `--1st-option` or `--2nd-option`
*/
CARGV_EXPORT
int cargv_opt(
    struct cargv_t *cargv,
    const char *optlist);

/* Read text value arguments.

[out] return: Number of values successfully read.
[in]  cargv:  cargv object.
[in]  name:   Display name of the program, used in error messages.
[out] vals:   Array to read values into.
[in]  valc:   Max number of values to read in. Values beyond are not processed.
*/
CARGV_EXPORT
int cargv_text(
    struct cargv_t *cargv,
    const char *name,
    const char **vals, cargv_len_t valc);


/* Read text value arguments if in a list.

[out] return: Number of values successfully read.
[in]  cargv:  cargv object.
[in]  name:   Display name of the program, used in error messages.
[in]  list:   Text list seprated by `sep`. Only texts in this list are read.
[in]  sep:    Text seprator.
[out] vals:   Array to read values into.
[in]  valc:   Max number of values to read in. Values beyond are not processed.
*/
CARGV_EXPORT
int cargv_oneof(
    struct cargv_t *cargv,
    const char *name,
    const char *list, const char *sep,
    const char **vals, cargv_len_t valc);


/* Read signed integer value arguments.

[out] return: Number of values successfully read.
              CARGV_VAL_OVERFLOW if any values are not cargv_int_t.
[out] vals:   Array to read values into.
[in]  valc:   Max number of values to read in. Values beyond are not processed.
*/
CARGV_EXPORT
int cargv_int(
    struct cargv_t *cargv,
    const char *name,
    cargv_int_t *vals, cargv_len_t valc);


/* Read unsigned integer value arguments.

[out] return: Number of values successfully read.
              CARGV_VAL_OVERFLOW if any values are not cargv_uint_t.
[out] vals:   Array to read values into.
[in]  valc:   Max number of values to read in. Values beyond are not processed.
*/
CARGV_EXPORT
int cargv_uint(
    struct cargv_t *cargv,
    const char *name,
    cargv_uint_t *vals, cargv_len_t valc);


/* ISO 8601 datetime, modified.

  Years
    [+-]Y{1-4}                Year 0 is 1 B.C., -1 is 2 B.C. and so on.
                              `month` and `day` are filled with 0.

  Months
    [+-]Y{1-4}-M{1-2}         `day` is filled with 0.
    [+-]Y{1-4}/M{1-2}

  Calendar dates
    [+-]YYYYMMDD
    [+-]Y{1-4}-M{1-2}-D{1-2}
    [+-]Y{1-4}/M{1-2}/D{1-2}
    --M{1-2}-D{1-2}
    --M{1-2}/D{1-2}           `year` is filled with CARGV_TODAY.

  Week dates: NOT supported
    [+-]Y{1-4}-Www-D
    [+-]YYYYWwwD

  Ordinal dates
    [+-]YYYYDDD
    [+-]Y{1-4}-DDD
    [+-]Y{1-4}/DDD

  Times
    hh:mm:ss[.sss]    `,` may be used.
    hhmmss[.sss]
    hh:mm[.mmm]
    hhmm[.mmm]
    hh[.hhh]

  Time zones
    Z                 UTC
    (+|-)hh:mm        time offset
    (+|-)hhmm
    (+|-)hh
    (omitted)         Uses system local time

  Date-times
    <date>T<time>
    <date> <time>
*/

/* Read date value arguments.

[out] return: Number of values successfully read.
              CARGV_VAL_OVERFLOW if any read value are not valid dates.
[out] vals:   Array to read values into.
[in]  valc:   Max number of values to read in. Values beyond are not processed.
*/
CARGV_EXPORT
int cargv_date(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_date_t *vals, cargv_len_t valc);

/* Read time, including time zone, value arguments.

[out] return: Number of values successfully read.
              CARGV_VAL_OVERFLOW if any read value are not valid.
[out] vals:   Array to read values into.
[in]  valc:   Max number of values to read in. Values beyond are not processed.
*/
CARGV_EXPORT
int cargv_time(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_time_t *vals, cargv_len_t valc);

/* Read time zone arguments.

[out] return: Number of values successfully read.
              CARGV_VAL_OVERFLOW if any read value are not valid.
[out] vals:   Array to read values into.
[in]  valc:   Max number of values to read in. Values beyond are not processed.
*/
CARGV_EXPORT
int cargv_timezone(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_timezone_t *vals, cargv_len_t valc);

/* Read date and time value arguments.

[out] return: Number of values successfully read.
              CARGV_VAL_OVERFLOW if any read value are not valid dates.
[out] vals:   Array to read values into.
[in]  valc:   Max number of values to read in. Values beyond are not processed.
*/
CARGV_EXPORT
int cargv_datetime(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_datetime_t *vals, cargv_len_t valc);

/* Convert a local time to another local time with a time zone.

[out] return: 0 if succeeded, <0 if error. See cargv_err_t.
[out] dst:    Converted local time.
[in]  src:    Local time to convert.
[in]  tz:     Time zone to convert with.
*/
CARGV_EXPORT
enum cargv_err_t cargv_convert_localtime(
    struct cargv_datetime_t *dst,
    const struct cargv_datetime_t *src,
    const struct cargv_timezone_t *tz);


/* Read degree value arguments.

Supported formats are:

  ISO 6709 degree   (+|-)[D]DD[.DDDDDD]
                    (+|-)[D]DDMM[.MMMM]
                    (+|-)[D]DDMMSS[.SS]

[out] return: Number of values successfully read.
              CARGV_VAL_OVERFLOW if any valus are not valid degree.
[out] vals:   Array to read values into.
[in]  valc:   Max number of values to read in. Values beyond are not processed.
*/
CARGV_EXPORT
int cargv_degree(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_degree_t *vals, cargv_len_t valc);


/* Read geocoord value arguments.

Supported formats are:

  ISO 6709 geocoord     LATITUDELONGITUDE[/]

[out] return: Number of values successfully read.
              CARGV_VAL_OVERFLOW if any values are not valid geocoord.
[out] vals:   Array to read values into.
[in]  valc:   Max number of values to read in. Values beyond are not processed.
*/
CARGV_EXPORT
int cargv_geocoord(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_geocoord_t *vals, cargv_len_t valc);


/* Convert to degree with decimal fraction */
CARGV_EXPORT
double cargv_get_degree(const struct cargv_degree_t *val);


#endif /* __cargv_h__ */
