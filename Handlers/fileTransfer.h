// File transfer API

#include <stdbool.h>

void initTransfer(void ); // needs to be called by App before use
bool openUpload(char * name);
int  readUpload(unsigned char * address, unsigned char n);
void closeUpload(void);   
bool openDownload(char * name, int size);
bool writeDownload(unsigned char * data, unsigned char length);
void closeDownload(void);
