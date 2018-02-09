#ifndef __sunriset_option_h__
#define __sunriset_option_h__

#ifdef __cplusplus
  #define SUNRISET_EXPORT extern "C"
#else
  #define SUNRISET_EXPORT
#endif


#include "cline.h"


enum sunriset_twilight {
    SUNRISET_NO_TWILIGHT            = 0,
    SUNRISET_CIVIL_TWILIGHT,
    SUNRISET_NAUTICAL_TWILIGHT,
    SUNRISET_ASTRONOMICAL_TWILIGHT,
};

struct sunriset_option_t {
    int help;
    int version;
    enum sunriset_twilight twilight;

    struct cline_date date;
    struct cline_geocoord position;
};

SUNRISET_EXPORT
void print_usage();

SUNRISET_EXPORT
int parse_args(struct sunriset_option_t *opt, int argc, char *argv[]);


#endif /* __sunriset_option_h__ */
