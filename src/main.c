/*
sunriset
*/

#include "sunriset_version.h"
#include "sunriset_option.h"
#include "sunriset.h"

#include <stdio.h>


int main(int argc, char *argv[])
{
    struct sunriset_option_t option;
    int err;

    if (argc <= 1) {
        print_usage();
        return 1;
    }
    if ((err = parse_args(&option, argc, argv)) != 0)
        return err;

    if (option.version) {
        printf("%s: %s\n", SUNRISET_NAME, SUNRISET_VERSION_STRING);
        return 0;
    }
    if (option.help) {
        print_usage();
        return 0;
    }

    printf("date:%04d-%02d-%02d\n",
        option.date.year, option.date.month, option.date.day);

    printf("latitude: %fdeg %fmin %fsec\n",
        (double)option.latitude.deg / 1000000.0,
        (double)option.latitude.min / 10000.0,
        (double)option.latitude.sec / 100.0);
    printf(" = %fdeg\n", cline_get_latitude_degree(&option.latitude));

    return 0;
}
