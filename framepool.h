// SFP Frame pool  Robert Chapman III  Feb 21, 2015

#include "sfp.h"

void initFramePool(void);
Long framePoolLeft(void);

sfpFrame * getFrame(void);
void putFrame(void * frame);
sfpFrame * igetFrame(void);
void iputFrame(void * frame);
sfpFrame * iigetFrame(void);
void iiputFrame(void * frame);
void listFrames(void);