#ifndef __cargv_version_h__
#define __cargv_version_h__


#define CARGV_NAME "cargv"
#define CARGV_VERSION_MAJOR     0
#define CARGV_VERSION_MINOR     1
#define CARGV_VERSION_PATCH     0
#define CARGV_VERSION_STATE     dev
#define CARGV_VERSION_REVISION  2

#define CARGV_VERSION \
    __CARGV_VERSION_NULBER(\
        CARGV_VERSION_MAJOR,\
        CARGV_VERSION_MINOR,\
        CARGV_VERSION_PATCH,\
        CARGV_VERSION_REVISION)

#define CARGV_VERSION_STRING \
    __CARGV_VERSION_STRING(\
        CARGV_VERSION_MAJOR,\
        CARGV_VERSION_MINOR,\
        CARGV_VERSION_PATCH,\
        CARGV_VERSION_STATE,\
        CARGV_VERSION_REVISION)


#define __CARGV_VERSION_NULBER(m, n, p, r) ((m)<<24 | (n)<<16 | (p)<<8 | (r))
#define __CARGV_QUOTE(s) #s
#define __CARGV_VERSION_STRING(m, n, p, s, r) \
    __CARGV_QUOTE(m) "." __CARGV_QUOTE(n) "." __CARGV_QUOTE(p) "."\
    __CARGV_QUOTE(s)__CARGV_QUOTE(r)


#endif /* __cargv_version_h__ */
