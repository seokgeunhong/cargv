#include "sunriset.h"
#include "sunriset_version.h"

#include "cargv.h"
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <time.h>


static void print_usage()
{
    static const char *usage[] = {
"Usage: sunriset [OPTIONS] [--] DATE POSITION [OUTPUT1 [OUTPUT2 ...]]",
"",
"Calculate time of sunrise and sunset, and length of day on a date",
"at a position on earth.",
"",
"OPTIONS:",
"",
"  --version         Print version info. and exit.",
"  -h --help         Print this help message and exit.",
"",
"  -u --utc --gmt    Print UTC time.",
"  -l --local-time   Print local time. This is default.",
"",
"  --hour            Print time in hours with decimal fraction, like hh.hhhhhh",
"                    Range is 0. to 24.",
"",
"  --sep SEP     Output values are separated by SEP. Default is new line.",
"",
"DATE        Date of observation. Years of [1801,2099] are valid.",
"            Only a part of ISO 8601 date format is supported.",
"",
"    ISO 8601 calendar dates:    [+]YYYY-MM-DD, [+]YYYYMMDD, --MMDD",
"",
"POSITION    Position of observation on the globe.",
"            Only ISO 6709 geocoord format is supported.",
"",
"    ISO 6709: +-LATITUDE+-LONGITUDE[/]",
"",
"        LATITUDE:   (+|-)DD[.DDDDDD]  degrees",
"                    (+|-)DDMM[.MMMM]  degrees, and minutes",
"                    (+|-)DDMMSS[.SS]  degrees, minutes and seconds",
"",
"        LONGITUDE:  (+|-)DDD[.DDDDDD] degrees",
"                    (+|-)DDDMM[.MMMM] degrees, and minutes",
"                    (+|-)DDDMMSS[.SS] degrees, minutes and seconds",
"",
"        Trailing solidus can be omitted and will be ignored.",
"        + sign means North or East, - does South or West, respectively.",
"",
"        Examples: Seoul, Korea: +37.56667+126.966667 or +3734+12658",
"                  New York City, U.S.: +40.7127-74.0059",
"                  Antananarivo, Madagascar: -18.933333+47.516667",
"",
"OUTPUTn     List of calculated output. Each result is printed to stdout,",
"            followed by a new line. Keywords are;",
"",
"    sunrise, or S                       Time of sunrise.",
"    sunset, or s                        Time of sunset.",
"    day length, or d                    Length of the day.",
"    civil-twilight-start, or C          Start time of civil twilight.",
"    civil-twilight-end, or c            End time of civil twilight.",
"    nautical-twilight-start, or N       Start time of civil twilight.",
"    nautical-twilight-end, or n         End time of civil twilight.",
"    astronomical-twilight-start, or A   Start time of civil twilight.",
"    astronomical-twilight-end, or a     End time of civil twilight.",
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

static void print_version()
{
    printf("%s: %s\n", SUNRISET_NAME, SUNRISET_VERSION_STRING);
}

enum sunriset_out {
    SUNRISET_OUT_DAYLEN,
    SUNRISET_OUT_SUNRISE,
    SUNRISET_OUT_SUNSET,
    SUNRISET_OUT_CIVIL_TWILIGHT_START,
    SUNRISET_OUT_CIVIL_TWILIGHT_END,
    SUNRISET_OUT_NAUTICAL_TWILIGHT_START,
    SUNRISET_OUT_NAUTICAL_TWILIGHT_END,
    SUNRISET_OUT_ASTRONOMICAL_TWILIGHT_START,
    SUNRISET_OUT_ASTRONOMICAL_TWILIGHT_END,
};

static const char sunriset_out_ch[] =
    "DSsCcNnAa";

static const char *sunriset_out_s[] = {
    "daylen",
    "sunrise",
    "sunset",
    "civil-twilight-start",
    "civil-twilight-end",
    "nautical-twilight-start",
    "nautical-twilight-end",
    "astronomical-twilight-start",
    "astronomical-twilight-end",
};


int sunriset_main(int argc, const char **argv)
{
    struct cargv_t cargv;

    int localtime = 1;
    struct cargv_date_t date;
    struct cargv_geocoord_t pos;
    int output[100];

    if (cargv_init(&cargv, SUNRISET_NAME, argc, argv) != CARGV_OK)
        return -99;

    cargv_shift(&cargv, 1); /* pass argv[0] */

    if (cargv_opt(&cargv, "-h--help") == CARGV_OK) {
        print_usage();
        return 0;
    }
    if (cargv_opt(&cargv, "--version") == CARGV_OK) {
        print_version();
        return 0;
    }
    while (1) {
        if (cargv_opt(&cargv, "-u--utc--gmt") == CARGV_OK)
            localtime = 0;
        else if (cargv_opt(&cargv, "-l--local-time") == CARGV_OK)
            localtime = 1;
        else if (cargv_opt(&cargv, "-*") == CARGV_OK) {
            fprintf(stderr, "%s: Unknown option `%s`.\n",
                    SUNRISET_NAME, *cargv.args);
            return -1;
        }
        else
            break;
    }
    if (cargv_date(&cargv, "DATE", &date, 1) != CARGV_OK)
        return -2;
    if (cargv_geocoord(&cargv, "POSITION", &pos, 1) != CARGV_OK)
        return -3;
    // if (cargv_key(&cargv, "OUTPUT", output,
    //         "-DSsCcNnAa"
    //         "--daylen--sunrise--sunset"
    //         "--civil-twilight-start--civil-twilight-end"
    //         "--nautical-twilight-start--nautical-twilight-end"
    //         "--astronomical-twilight-start--astronomical-twilight-end",
    //         100) != CARGV_OK)
    //     return -4;

    printf("date:%04lld-%02lld-%02lld\n", date.year, date.month, date.day);
    printf("latitude: %fdeg %fmin %fsec = %fdeg\n",
        (double)pos.latitude.deg / 1000000.0,
        (double)pos.latitude.min / 1000000.0,
        (double)pos.latitude.sec / 1000000.0,
        cargv_get_degree(&pos.latitude));
    printf("longitude: %fdeg %fmin %fsec = %fdeg\n",
        (double)pos.longitude.deg / 1000000.0,
        (double)pos.longitude.min / 1000000.0,
        (double)pos.longitude.sec / 1000000.0,
        cargv_get_degree(&pos.longitude));

    return 0;
}
