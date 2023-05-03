//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#ifndef QU_LOG_H
#define QU_LOG_H

//------------------------------------------------------------------------------

typedef enum libqu_log_level
{
    LIBQU_DEBUG,
    LIBQU_INFO,
    LIBQU_WARNING,
    LIBQU_ERROR,
} libqu_log_level;

void libqu_log(libqu_log_level log_level, char const *fmt, ...);

#if defined(NDEBUG)
#   define libqu_debug
#else
#   define libqu_debug(...)     libqu_log(LIBQU_DEBUG, __VA_ARGS__)
#endif

#define libqu_info(...)         libqu_log(LIBQU_INFO, __VA_ARGS__)
#define libqu_warning(...)      libqu_log(LIBQU_WARNING, __VA_ARGS__)
#define libqu_error(...)        libqu_log(LIBQU_ERROR, __VA_ARGS__)

//------------------------------------------------------------------------------

#endif // QU_LOG_H

//------------------------------------------------------------------------------