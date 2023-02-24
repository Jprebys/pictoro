#ifndef _LOGGING_H
#define _LOGGING_H

#include <stdarg.h>

#define LOGGING_MAX_LEN 2048


typedef enum LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
} LogLevel_t;


void logger(LogLevel_t level, char* msg, ...)
{
    char buff[LOGGING_MAX_LEN];
    va_list args;
    va_start(args, msg);
    vsnprintf(buff, sizeof(buff), msg, args);
    va_end(args);

    buff[LOGGING_MAX_LEN - 1] = 0;

    switch (level)
    {
#ifdef DEBUG
        case DEBUG:
            printf("[DEBUG] %s\n", buff);
            break;
#endif
        case INFO:
            printf("[INFO ] %s\n", buff);
            break;
        case WARNING:
            printf("[WARN ] %s\n", buff);
            break;
        case ERROR:
            printf("[ERROR] %s\n", buff);
            break;
        default:
            break;
    }
}



#endif