#include "sfp.h"

void calculateCheckSum(checkSum_t *cs, Byte length, Byte *data);
void buildSfpFrame(Byte length, Byte *data, Byte pid, sfpFrame *f);
void addChecksum(sfpFrame *frame);
void addSfpFrame(sfpFrame *frame, Byte length);
