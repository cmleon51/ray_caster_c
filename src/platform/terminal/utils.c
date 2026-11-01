#include <platform/utils.h>

#include <stdio.h>
#include <time.h>

#define LOG_LINE_SIZE 512

NsTime utils_get_current_time_ns() {
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    return current_time.tv_sec * 1000000000L + current_time.tv_nsec;
}

void utils_log(LOG_TYPE log_type, const char *fmt, ...) {
    char line[LOG_LINE_SIZE];
    int prefix_len = snprintf(line, sizeof(line), "%s", log_type == LOG_ERROR ? "ERROR: " : "INFO: ");

    va_list args;
    va_start(args, fmt);
    vsnprintf(line + prefix_len, sizeof(line) - prefix_len, fmt, args);
    va_end(args);

    fprintf(stderr, "%s\n", line);
}
