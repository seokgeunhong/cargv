#ifndef __cargv_version_h__
#define __cargv_version_h__


#define CARGV_NAME "cargv"
#define CARGV_VERSION_MAJOR     0
#define CARGV_VERSION_MINOR     1
#define CARGV_VERSION_PATCH     0
#define CARGV_VERSION_STATE     dev


#if (CARGV_VERSION_STATE == dev)
    #define CARGV_VERSION_STATE_NUMBER -99
#elif (CARGV_VERSION_STATE == rc)
    #define CARGV_VERSION_STATE_NUMBER -1
#else
    #define CARGV_VERSION_STATE_NUMBER 0
#endif


#define CARGV_VERSION \
    __CARGV_VERSION_NULBER(\
        CARGV_VERSION_MAJOR,\
        CARGV_VERSION_MINOR,\
        CARGV_VERSION_PATCH)

#define CARGV_VERSION_STRING \
    __CARGV_VERSION_STRING(\
        CARGV_VERSION_MAJOR,\
        CARGV_VERSION_MINOR,\
        CARGV_VERSION_PATCH,\
        CARGV_VERSION_STATE)


#define __CARGV_VERSION_NULBER(m, n, p, s) ((m)<<24 | (n)<<16 | (p)<<8 | (s))
#define __CARGV_QUOTE(s) #s
#define __CARGV_VERSION_STRING(m, n, p, s) \
    __CARGV_QUOTE(m) "." __CARGV_QUOTE(n) "." __CARGV_QUOTE(p) "."\
    __CARGV_QUOTE(s)


#endif /* __cargv_version_h__ */
