#ifndef __ERR_EXIT_H__
#define __ERR_EXIT_H__

//#define SW_DBUG_LOG
// Print error information
int errexit(const char* format, ...);

// Print work information
void printLog(const char* format, ...);

void printDebugLog(const char* format, ...);
#endif
