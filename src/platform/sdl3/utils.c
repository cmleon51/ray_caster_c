#include <platform/utils.h>

#include <SDL3/SDL_time.h>
#include <SDL3/SDL_log.h>

NsTime utils_get_current_time_ns() {
    SDL_Time current_time;
    SDL_GetCurrentTime(&current_time);

    return current_time;
}

void utils_log(LOG_TYPE log_type, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    switch (log_type) {
        case LOG_INFO:
            SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, fmt, args);
        break;
        case LOG_ERROR:
            SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, fmt, args);
        break;
    }

    va_end(args);
}
