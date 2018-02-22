/* cargv - A command line argument parser.
*/

#ifndef __cargv_h__
#define __cargv_h__

#include <stddef.h>
#include <stdint.h>
#include <time.h>

typedef int64_t     cargv_int_t;
typedef uint64_t    cargv_uint_t;
typedef ptrdiff_t   cargv_len_t;

#define CARGV_SINT_MIN  INT64_MIN
#define CARGV_SINT_MAX  INT64_MAX
#define CARGV_UINT_MAX  UINT64_MAX


struct cargv_t {
    const char *name;
    const char **args, **argend;
};

enum cargv_err_t {
    CARGV_OK    = 0,
    CARGV_VAL_OVERFLOW  = -1,   /* value is read but wrong */
};


struct cargv_date_t {
    cargv_int_t year;  /* -9999..9999 */
    cargv_uint_t month, day;
};

struct cargv_time_t {
    cargv_uint_t hour, minute, second;
};

struct cargv_degree_t {
    cargv_int_t deg;    /* ddd.dddddd */
    cargv_int_t min;    /*  mm.mmmmmm */
    cargv_int_t sec;    /*  ss.ssssss */
};

struct cargv_geocoord_t {
    struct cargv_degree_t latitude, longitude;
    /* int altitude; */
};


#ifdef __cplusplus
  #define CARGV_EXPORT extern "C"
#else
  #define CARGV_EXPORT
#endif


/* Initialize cargv object.

[out] return:   0 if succeeded, <0 if error. See cargv_err_t.
[out] cargv:    cargv object.
[in]  name:     Display name of the program, used in error messages.
[in]  argc, argv: Passed from main().
*/
CARGV_EXPORT
enum cargv_err_t cargv_init(
    struct cargv_t *cargv,
    const char *name,
    int argc, const char **argv);

/* Shift: remove the first N arguments from the argument list.

[out] return:   Number of arguments removed.
                0 if remained arguments are less than requested.
[in]  cargv:    cargv object.
[in]  argc:     Number of arguments to remove.
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

[out] return:   1 if an option matched. 0 if not matched, <0 if error.
                See cargv_err_t.
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

[out] return:   Number of values successfully read.
[in]  cargv:    cargv object.
[in]  name:     Display name of the program, used in error messages.
[out] vals:     Array to read values into.
[in]  valc:     Max number of values to read in. Values beyond will be ignored.
*/
CARGV_EXPORT
int cargv_text(
    struct cargv_t *cargv,
    const char *name,
    const char **vals, cargv_len_t valc);


/* Read text in a list.

[out] return:   Number of values successfully read.
[in]  cargv:    cargv object.
[in]  name:     Display name of the program, used in error messages.
[in]  list:     Text list seprated by `sep`. Only texts in this list are read.
[in]  sep:      Text seprator.
[out] vals:     Array to read values into.
[in]  valc:     Max number of values to read in. Values beyond will be ignored.
*/
CARGV_EXPORT
int cargv_oneof(
    struct cargv_t *cargv,
    const char *name,
    const char *list, const char *sep,
    const char **vals, cargv_len_t valc);


/* Read signed integer value arguments.

[out] return:   Number of values successfully read.
                CARGV_VAL_OVERFLOW if any values are not cargv_int_t.
[out] vals:     Array to read values into.
[in]  valc:     Max number of values to read in. Values beyond will be ignored.
*/
CARGV_EXPORT
int cargv_int(
    struct cargv_t *cargv,
    const char *name,
    cargv_int_t *vals, cargv_len_t valc);


/* Read unsigned integer value arguments.

[out] return:   Number of values successfully read.
                CARGV_VAL_OVERFLOW if any values are not cargv_uint_t.
[out] vals:     Array to read values into.
[in]  valc:     Max number of values to read in. Values beyond will be ignored.
*/
CARGV_EXPORT
int cargv_uint(
    struct cargv_t *cargv,
    const char *name,
    cargv_uint_t *vals, cargv_len_t valc);


/* Read date value arguments.

[out] return:   Number of values successfully read.
                CARGV_VAL_OVERFLOW if any read value are not valid dates.
[out] vals:     Array to read values into.
[in]  valc:     Max number of values to read in. Values beyond will be ignored.
*/
CARGV_EXPORT
int cargv_date(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_date_t *vals, cargv_len_t valc);


/* Read time value arguments.

[out] return:   Number of values successfully read.
                CARGV_VAL_OVERFLOW if any read value are not valid.
[out] vals:     Array to read values into.
[in]  valc:     Max number of values to read in. Values beyond will be ignored.
*/
CARGV_EXPORT
int cargv_time(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_time_t *vals, cargv_len_t valc);


/* Read degree value arguments.

[out] return:   Number of values successfully read.
                CARGV_VAL_OVERFLOW if any valus are not valid degree.
[out] vals:     Array to read values into.
[in]  valc:     Max number of values to read in. Values beyond will be ignored.
*/
CARGV_EXPORT
int cargv_degree(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_degree_t *vals, cargv_len_t valc);


/* Read geocoord value arguments.

[out] return:   Number of values successfully read.
                CARGV_VAL_OVERFLOW if any values are not valid geocoord.
[out] vals:     Array to read values into.
[in]  valc:     Max number of values to read in. Values beyond will be ignored.
*/
CARGV_EXPORT
int cargv_geocoord(
    struct cargv_t *cargv,
    const char *name,
    struct cargv_geocoord_t *vals, cargv_len_t valc);


/* Get current date.

[out] return:   time_t value of current time.
*/
CARGV_EXPORT
time_t cargv_get_today(struct cargv_date_t *val);

/* Get current time.

[out] return:   time_t value of current time.
*/
CARGV_EXPORT
time_t cargv_get_now(struct cargv_time_t *val);

/* Convert to degree with decimal fraction */
CARGV_EXPORT
double cargv_get_degree(const struct cargv_degree_t *val);


#endif /* __cargv_h__ */
