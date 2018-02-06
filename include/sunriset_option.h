#ifndef __sunriset_option_h__
#define __sunriset_option_h__


struct sunriset_option_t {
    int help;
    int version;
    int verbose;
    char const *longitude, *latitude, *altitude;
};

void print_usage();

int parse_args(struct sunriset_option_t *opt, int argc, char *argv[]);


#endif /* __sunriset_option_h__ */
