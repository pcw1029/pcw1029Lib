#ifndef __ICD_HANDLER_H__
#define __ICD_HANDLER_H__

int ICDHandler(unsigned char* puchWriteData, unsigned char* puchReadData, unsigned char uchIcdCode);

int changeTrackerSW(unsigned char* puchWriteData, unsigned char* puchReadData);
#endif 

