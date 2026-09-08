#ifndef UTILS_H
#define UTILS_H

#include <stdarg.h>

typedef enum {
    LOG_INFO,
    LOG_ERROR,
} LOG_TYPE;

typedef long NsTime;

NsTime utils_get_current_time_ns();
void utils_log(LOG_TYPE log_type, const char *fmt, ...);

#endif // UTILS_H
