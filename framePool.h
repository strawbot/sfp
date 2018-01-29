// SFP Frame pool  Robert Chapman III  Feb 21, 2015

#include "sfp.h"

void initFramePool(void);
Long framePoolLeft(void);

sfpFrame * getFrame(void);
void returnFrame(void * frame);
sfpFrame * igetFrame(void);
void ireturnFrame(void * frame);
sfpFrame * iigetFrame(void);
void iireturnFrame(void * frame);
void listFrames(void);
