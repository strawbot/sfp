// SFP Frame pool  Robert Chapman III  Feb 21, 2015

#include "sfp.h"

bool returnFrame(void *frame);
sfpFrame *getFrame(void);

void initFramePool(void);
Long framePoolLeft(void);