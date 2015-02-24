#ifndef MOCKS_H
#define MOCKS_H

#include "sfp.h"
#include "stats.h"

bool framePoolFull();

#define DECLARE_GET_STAT(stat) Long get##stat();
FOR_EACH_STAT(DECLARE_GET_STAT)

extern Long acksin, acksout;
extern Byte packet[3];
void initLink();
void initNode();
void rxFrame(sfpFrame * frame);
void callProcessFrames();
void callretryFrames();
void setTime(Long t);

bool acceptSpsFrame(sfpFrame * frame);
#undef setAckReceived
void setAckReceived(Byte who);
extern bool spsaccept;

#endif // MOCKS_H
