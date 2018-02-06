/*
sunriset
*/

#include "sunriset_option.h"
#include "sunriset_version.h"
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
    // if (option.verbose)
    //     printf("opt: --verbose\n");
    if (option.longitude)
        printf("opt: --longitude '%s'\n", option.longitude);
    if (option.latitude)
        printf("opt: --latitude '%s'\n", option.latitude);
    if (option.altitude)
        printf("opt: --altitude '%s'\n", option.altitude);

    return 0;
}
