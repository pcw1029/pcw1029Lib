// va_list
#include <stdarg.h>     
#include <stdio.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include <syslog.h>

// Print error information
int errexit(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}

// Print work information
void printLog(const char* format, ...)
{
    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
	va_end(args);
}

void printDebugLog(const char* format, ...)
{
    va_list args;
	char chDebugMsg[256];

    va_start(args, format);
#ifdef SW_DBUG_LOG
	memset(chDebugMsg, 0x0, sizeof(chDebugMsg));
	vsprintf(chDebugMsg, format, args);
    syslog(LOG_INFO,"%s\n",chDebugMsg);
#else
    vfprintf(stderr, format, args);
#endif
	va_end(args);
}


