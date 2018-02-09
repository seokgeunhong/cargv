#ifndef __sunriset_version_h__
#define __sunriset_version_h__


#define SUNRISET_NAME "sunriset"
#define SUNRISET_VERSION_MAJOR     0
#define SUNRISET_VERSION_MINOR     0
#define SUNRISET_VERSION_PATCH     0
#define SUNRISET_VERSION_STATE     dev
#define SUNRISET_VERSION_REVISION  0

#define SUNRISET_VERSION \
    __SUNRISET_VERSION_NULBER(\
        SUNRISET_VERSION_MAJOR,\
        SUNRISET_VERSION_MINOR,\
        SUNRISET_VERSION_PATCH,\
        SUNRISET_VERSION_REVISION)

#define SUNRISET_VERSION_STRING \
    __SUNRISET_VERSION_STRING(\
        SUNRISET_VERSION_MAJOR,\
        SUNRISET_VERSION_MINOR,\
        SUNRISET_VERSION_PATCH,\
        SUNRISET_VERSION_STATE,\
        SUNRISET_VERSION_REVISION)


#define __SUNRISET_VERSION_NULBER(m, n, p, r) ((m)<<24 | (n)<<16 | (p)<<8 | (r))
#define __SUNRISET_QUOTE(s) #s
#define __SUNRISET_VERSION_STRING(m, n, p, s, r) \
    __SUNRISET_QUOTE(m) "." __SUNRISET_QUOTE(n) "." __SUNRISET_QUOTE(p) "."\
    __SUNRISET_QUOTE(s)__SUNRISET_QUOTE(r)


#endif /* __sunriset_version_h__ */
