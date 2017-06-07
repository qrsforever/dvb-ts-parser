#ifndef __DvbAssertions__H_
#define __DvbAssertions__H_

#include <string.h>
#include <stdio.h>

extern "C" void dvblog(const char* file, int line, const char* function, const char* level, const char* fmt, ...);

#define LogDvbError(fmt...) \
    do {\
        dvblog(__FILE__, __LINE__, __func__, "Error", fmt);\
    }while(0)

#define LogDvbWarn(fmt...)\
    do {\
        dvblog(__FILE__, __LINE__, __func__, "Warn", fmt);\
    }while(0)

#define LogDvbDebug(fmt...)\
    do {\
        dvblog(__FILE__, __LINE__, __func__, "Debug", fmt);\
    }while(0)

#endif

#define __DEBUG LogDvbDebug("\n");
