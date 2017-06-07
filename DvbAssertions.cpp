#include "DvbAssertions.h"

#include <stdarg.h>

extern "C" 
void dvblog(const char* file, int line, const char* function, const char* level, const char* fmt, ...)
{
    char logBuffer[4096] = "";
    va_list args; 
    va_start(args, fmt);
    vsnprintf(logBuffer, 4088, fmt, args);
    va_end(args);
    printf("[%s:%d - %s] %s: %s", strrchr(file, '/') + 1, line, function, level, logBuffer);
}
