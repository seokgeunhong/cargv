#ifndef __cargv_version_h__
#define __cargv_version_h__


#define CARGV_NAME "cargv"
#define CARGV_VERSION_MAJOR         0
#define CARGV_VERSION_MINOR         1
#define CARGV_VERSION_PATCH         0
#define CARGV_VERSION_STATE_DEV     6


#if (CARGV_VERSION_STATE_DEV > 0)

    #define CARGV_VERSION_STATE         dev
    #define CARGV_VERSION_STATE_STRING  __CARGV_QUOTE(CARGV_VERSION_STATE)

    #define CARGV_VERSION \
        __CARGV_VERSION_NULBER(\
            CARGV_VERSION_MAJOR,\
            CARGV_VERSION_MINOR,\
            CARGV_VERSION_PATCH,\
            CARGV_VERSION_STATE_DEV)

    #define CARGV_VERSION_STRING \
        __CARGV_VERSION_STATE_STRING(\
            CARGV_VERSION_MAJOR,\
            CARGV_VERSION_MINOR,\
            CARGV_VERSION_PATCH,\
            CARGV_VERSION_STATE,\
            CARGV_VERSION_STATE_DEV)

#else

    #define CARGV_VERSION_STATE         stable
    #define CARGV_VERSION_STATE_STRING  __CARGV_QUOTE(CARGV_VERSION_STATE)

    #define CARGV_VERSION \
        __CARGV_VERSION_NULBER(\
            CARGV_VERSION_MAJOR,\
            CARGV_VERSION_MINOR,\
            CARGV_VERSION_PATCH,\
            0)

    #define CARGV_VERSION_STRING \
        __CARGV_VERSION_STRING(\
            CARGV_VERSION_MAJOR,\
            CARGV_VERSION_MINOR,\
            CARGV_VERSION_PATCH,\
            CARGV_VERSION_STATE)

#endif


#ifdef __cplusplus
  #define __CARGV_VERSION_NULBER(m, n, p, d)  \
      (uint32_t(m)<<24 | uint32_t(n)<<16 | uint32_t(p)<<8 | uint32_t(d))
#else
  #define __CARGV_VERSION_NULBER(m, n, p, d)  \
      ((uint32_t)(m)<<24 | (uint32_t)(n)<<16 | (uint32_t)(p)<<8 | (uint32_t)(d))
#endif

#define __CARGV_VERSION_STATE_STRING(m, n, p, s, r) \
    __CARGV_QUOTE(m) "."\
    __CARGV_QUOTE(n) "."\
    __CARGV_QUOTE(p) "."\
    __CARGV_QUOTE(s) "-" __CARGV_QUOTE(r)

#define __CARGV_VERSION_STABLE_STRING(m, n, p, s) \
    __CARGV_QUOTE(m) "."\
    __CARGV_QUOTE(n) "."\
    __CARGV_QUOTE(p) "."\
    __CARGV_QUOTE(s)

#define __CARGV_QUOTE(s) #s


#endif /* __cargv_version_h__ */
