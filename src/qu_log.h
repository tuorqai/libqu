//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#ifndef QU_LOG_H
#define QU_LOG_H

//------------------------------------------------------------------------------

#ifndef QU_MODULE
#define QU_MODULE "?"__FILE__
#endif

typedef enum libqu_log_level
{
    LIBQU_DEBUG,
    LIBQU_INFO,
    LIBQU_WARNING,
    LIBQU_ERROR,
} libqu_log_level;

void libqu_log(libqu_log_level log_level, char const *module, char const *fmt, ...);

#if defined(NDEBUG)
#   define libqu_debug
#else
#   define libqu_debug(...)     libqu_log(LIBQU_DEBUG, QU_MODULE, __VA_ARGS__)
#endif

#define libqu_info(...)         libqu_log(LIBQU_INFO, QU_MODULE, __VA_ARGS__)
#define libqu_warning(...)      libqu_log(LIBQU_WARNING, QU_MODULE, __VA_ARGS__)
#define libqu_error(...)        libqu_log(LIBQU_ERROR, QU_MODULE, __VA_ARGS__)

//------------------------------------------------------------------------------

#endif // QU_LOG_H

//------------------------------------------------------------------------------