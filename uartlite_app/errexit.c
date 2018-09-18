// va_list
#include <stdarg.h>     
#include <stdio.h>
#include <stdlib.h>

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
//	fprintf(stderr,"#### %s():%d ####\n",__func__,__LINE__);
//#ifdef SW_DBUG_LOG
//	fprintf(stderr,"#### %s():%d ####\n",__func__,__LINE__);
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
//#endif
}


