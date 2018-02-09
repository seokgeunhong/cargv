#include "sunriset_option.h"
#include "sunriset_version.h"

#include "cline.h"
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <time.h>


void print_usage()
{
    static const char *usage[] = {
"Usage: sunriset [OPTIONS]",
"",
"Calculate time of sunrise and sunset, and length of day on a date",
"at a position on earth.",
"",
"OPTIONS:",
"",
"  --version     Print version info. and exit.",
"  -h --help     Print this help message and exit.",
"",
"  --no-twilight (default)   Sunrise/set is considered to occur when the Sun's",
"                            upper limb is 35 arc minutes below the horizon,",
"                            accounting for the refraction of the Earth's",
"                            atmosphere.",
"  --civil-twilight          Civil twilight starts/ends when the Sun's center",
"                            is 6 degrees below the horizon.",
"  --nautical-twilight       Nautical twilight starts/ends when the Sun's",
"                            center is 12 degrees below the horizon.",
"  --astronomical-twilight   Astronomical twilight starts/ends when the Sun's",
"                            center is 18 degrees below the horizon.",
"",
"  --date DATE               Date of observation. Only years of [1801,2099]",
"                            are valid. Default is date of local time.",
"",
"      ISO 8601:   yyyy-mm-dd",
"",
"  -p --position GEOCOORD    Position of observation. Default is Seoul.",
"",
"      ISO 6709: +-LATITUDE+-LONGITUDE[/]",
"",
"          LATITUDE:   (+|-)DD[.DDDDDD]  degrees",
"                      (+|-)DDMM[.MMMM]  degrees, and minutes",
"                      (+|-)DDMMSS[.SS]  degrees, minutes and seconds",
"",
"          LONGITUDE:  (+|-)DDD[.DDDDDD] degrees",
"                      (+|-)DDDMM[.MMMM] degrees, and minutes",
"                      (+|-)DDDMMSS[.SS] degrees, minutes and seconds",
"",
"          Trailing solidus can be omitted and will be ignored.",
"          + sign means North or East, - does South or West, respectively.",
"",
"          Examples: Seoul, Korea: +37.56667+126.966667 or +3734+12658",
"                    New York City, U.S.: +40.7127-74.0059",
"                    Antananarivo, Madagascar: -18.933333+47.516667",
"",
"Original code was written and released to the public by Paul Schlyter,",
"December 1992, and can be found at: http://stjarnhimlen.se/comp/sunriset.c",
"",
"(c) Paul Schlyter, 1989, 1992",
"",
0
    };
    const char **s;

    for (s = usage; *s; s++)
        fprintf(stderr, "%s\n", *s);
}

int parse_args(struct sunriset_option_t *option, int argc, char *argv[])
{
    static const struct cline_opt _opts[] = {
        {"--help\0-h\0",                CLINE_BOOL},
        {"--version\0",                 CLINE_BOOL},
        {"--no-twilight\0",             CLINE_BOOL},
        {"--civil-twilight\0",          CLINE_BOOL},
        {"--nautical-twilight\0",       CLINE_BOOL},
        {"--astronomical-twilight\0",   CLINE_BOOL},
        {"--date\0",                    CLINE_DATE},
        {"--year\0-Y\0",                CLINE_INTEGER},
        {"--month\0-M\0",               CLINE_INTEGER},
        {"--day\0-D\0",                 CLINE_INTEGER},
        {"--position\0--pos\0-p\0",     CLINE_GEOCOORD},
    };

    struct cline_parser parser;
    int err;
    const struct cline_opt *opt;
    const char *name;
    struct cline_value val;
    time_t now;
    struct tm *today;

    memset(option, 0, sizeof(*option));

    /* date defaults to today */
    time(&now);
    today = localtime(&now);
    option->date.year = today->tm_year + 1900;
    option->date.month = today->tm_mon + 1;
    option->date.day = today->tm_mday;

    /* position defaults to Seoul */
    option->position.latitude.deg = 37000000;
    option->position.latitude.min = 34000000;
    option->position.latitude.sec = 0;
    option->position.longitude.deg = 126000000;
    option->position.longitude.min = 58000000;
    option->position.longitude.sec = 0;

    if (cline_init(&parser, sizeof(_opts)/sizeof(_opts[0]), _opts, argc, argv))
        return 1;

    while ((err = cline_read(&parser, &opt, &name, &val)) == CLINE_OK) {
        if (strcmp(opt->names, "--help") == 0) {
            option->help = 1;
        }
        else if (strcmp(opt->names, "--version") == 0) {
            option->version = 1;
        }
        else if (strcmp(opt->names, "--no-twilight") == 0) {
            option->twilight = SUNRISET_NO_TWILIGHT;
        }
        else if (strcmp(opt->names, "--civil-twilight") == 0) {
            option->twilight = SUNRISET_CIVIL_TWILIGHT;
        }
        else if (strcmp(opt->names, "--nautical-twilight") == 0) {
            option->twilight = SUNRISET_NAUTICAL_TWILIGHT;
        }
        else if (strcmp(opt->names, "--astronomical-twilight") == 0) {
            option->twilight = SUNRISET_ASTRONOMICAL_TWILIGHT;
        }
        else if (strcmp(opt->names, "--date") == 0) {
            memcpy(&option->date, &val.date, sizeof(option->date));
        }
        else if (strcmp(opt->names, "--year") == 0) {
            option->date.year = val.integer;
        }
        else if (strcmp(opt->names, "--month") == 0) {
            option->date.month = val.integer;
        }
        else if (strcmp(opt->names, "--day") == 0) {
            option->date.day = val.integer;
        }
        else if (strcmp(opt->names, "--position") == 0) {
            memcpy(&option->position, &val.geocoord, sizeof(option->position));
        }
        else {
            fprintf(stderr,
                "FATAL: parsed, but not recognized option `%s`\n", name);
        return CLINE_ERR_OPTION;
        }
    }
    if (err != CLINE_END) {
        fprintf(stderr, "Error: %d\n", err);
        return err;
    }
    return 0;
}
