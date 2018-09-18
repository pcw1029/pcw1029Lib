#ifndef __COMMON_API_H__
#define __COMMON_API_H__

enum {NONE, RECV, SEND};
void printRawData(char* pchBuff, int iSize, int iTcpHeadFlag);
void icdToChar(unsigned short unIcd, char* chIcdName);

#endif
