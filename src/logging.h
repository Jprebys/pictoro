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


void logger(LogLevel_t level, char* msg, ...);


#endif