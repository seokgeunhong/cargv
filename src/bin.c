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

static void print_time_of_day(double t)
{
    printf("%02d:%02d:%02d", (int)t, (int)(t*60.0)%60, (int)(t*3600.0)%60);
}

static void print_daylen(
    const struct cargv_date_t *date,
    const struct cargv_geocoord_t *pos)
{
    print_time_of_day(__daylen__(
            (int)date->year, (int)date->month, (int)date->day,
            cargv_get_degree(&pos->longitude),
            cargv_get_degree(&pos->latitude), -35.0/60.0, 1));
    printf("\n");
}

static void __print_start_end(
    const struct cargv_date_t *date,
    const struct cargv_geocoord_t *pos,
    double altitude, int upperlimb)
{
    double start, end;
    int allday;

    allday = __sunriset__(
            (int)date->year, (int)date->month, (int)date->day,
            cargv_get_degree(&pos->longitude),
            cargv_get_degree(&pos->latitude),
            altitude, upperlimb,
            &start, &end);

    print_time_of_day(start);
    printf(" ");
    print_time_of_day(end);
    printf("\n");
}


static void print_sunrise_sunset(
    const struct cargv_date_t *date,
    const struct cargv_geocoord_t *pos)
{
    __print_start_end(date, pos, -35.0/60.0, 1);
}

static void print_civil_twilight(
    const struct cargv_date_t *date,
    const struct cargv_geocoord_t *pos)
{
    __print_start_end(date, pos, -6.0, 0);
}

static void print_nautical_twilight(
    const struct cargv_date_t *date,
    const struct cargv_geocoord_t *pos)
{
    __print_start_end(date, pos, -12.0, 0);
}

static void print_astronomical_twilight(
    const struct cargv_date_t *date,
    const struct cargv_geocoord_t *pos)
{
    __print_start_end(date, pos, -18.0, 0);
}

int sunriset_main(int argc, const char **argv)
{
    int r;
    struct cargv_t cargv;

    int localtime = 1;
    struct cargv_date_t date;
    struct cargv_geocoord_t pos;
    const char *out[100];
    int outc;
    int i;
    const char *o;

    if (cargv_init(&cargv, SUNRISET_NAME, argc, argv) != CARGV_OK)
        return -99;

    cargv_shift(&cargv, 1); /* pass argv[0] */

    if (cargv_opt(&cargv, "-h--help") > 0) {
        print_usage();
        return 0;
    }
    if (cargv_opt(&cargv, "--version") > 0) {
        print_version();
        return 0;
    }
    while (1) {
        if (cargv_opt(&cargv, "-u--utc--gmt") > 0) {
            localtime = 0;
            cargv_shift(&cargv, 1);
        }
        else if (cargv_opt(&cargv, "-l--local-time") > 0) {
            localtime = 1;
            cargv_shift(&cargv, 1);
        }
        else if (cargv_opt(&cargv, "-*") > 0) {
            fprintf(stderr, "%s: Unknown option `%s`.\n",
                    SUNRISET_NAME, *cargv.args);
            return -1;
        }
        else
            break;
    }
    if ((r = cargv_date(&cargv, "DATE", &date, 1)) < 0)
        return -2;
    if (r == 0) {
        fprintf(stderr, "%s: DATE missing.\n", SUNRISET_NAME);
        return -1;
    }
    cargv_shift(&cargv, 1);
    if ((r = cargv_geocoord(&cargv, "POSITION", &pos, 1)) < 0)
        return -2;
    if (r == 0) {
        fprintf(stderr, "%s: POSITION missing.\n", SUNRISET_NAME);
        return -1;
    }
    cargv_shift(&cargv, 1);
    if ((outc = cargv_oneof(&cargv, "OUTPUT",
            "day|daylen|sun|sunrise-sunset|sunriset|civil-twilight|civil|"
            "nautical-twilight|nautical|astronomical-twilight|astronomical",
            "|", out, 100)) < 0) {
        return -2;
    }
    if (outc == 0) {
        fprintf(stderr, "%s: WARNING. No output specified.\n", SUNRISET_NAME);
    }

    printf("date: %04lld-%02lld-%02lld\n", date.year, date.month, date.day);
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
    
    for (i = 0; i < outc; i++) {
        if (cargv_oneof(&cargv, "OUTPUT", "day|daylen", "|", out + i, 1) > 0) {
            print_daylen(&date, &pos);
        }
        else if (cargv_oneof(&cargv, "OUTPUT",
                "sun|sunrise-sunset|sunriset", "|", out + i, 1) > 0) {
            print_sunrise_sunset(&date, &pos);
        }
        else if (cargv_oneof(&cargv, "OUTPUT",
                "civil-twilight|civil", "|", out + i, 1) > 0) {
            print_civil_twilight(&date, &pos);
        }
        else if (cargv_oneof(&cargv, "OUTPUT",
                "nautical-twilight|nautical", "|", out + i, 1) > 0) {
            print_nautical_twilight(&date, &pos);
        }
        else if (cargv_oneof(&cargv, "OUTPUT",
                "astronomical-twilight|astronomical", "|", out + i, 1) > 0) {
            print_astronomical_twilight(&date, &pos);
        }
        cargv_shift(&cargv, 1);
    }

    if (cargv_shift(&cargv, 1) > 0) {
        fprintf(stderr, "%s: WARNING. Redundant arguments from `%s`.\n",
            SUNRISET_NAME, *(cargv.args-1));
    }
    return 0;
}
