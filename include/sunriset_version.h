#ifndef __sunriset_version_h__
#define __sunriset_version_h__


#define SUNRISET_NAME "sunriset"
#define SUNRISET_VERSION_MAJOR 0
#define SUNRISET_VERSION_MINOR 0
#define SUNRISET_VERSION_PATCH 0

#define SUNRISET_VERSION __SUNRISET_VERSION_NULBER(\
    SUNRISET_VERSION_MAJOR,\
    SUNRISET_VERSION_MINOR,\
    SUNRISET_VERSION_PATCH)

#define SUNRISET_VERSION_STRING __SUNRISET_VERSION_STRING(\
    SUNRISET_VERSION_MAJOR,\
    SUNRISET_VERSION_MINOR,\
    SUNRISET_VERSION_PATCH)


#define __SUNRISET_VERSION_NULBER(m, n, p) ((m) << 16 | (n) <<  8 | (p))
#define __SUNRISET_QUOTE(s) #s
#define __SUNRISET_VERSION_STRING(m, n, p) \
    __SUNRISET_QUOTE(m) "." __SUNRISET_QUOTE(n) "." __SUNRISET_QUOTE(p)


#endif /* __sunriset_version_h__ */
