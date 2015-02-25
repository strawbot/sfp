#ifndef MOCKS_H
#define MOCKS_H

#include "sfp.h"
#include "stats.h"
#include "link.h"

bool framePoolFull();

#define DECLARE_GET_STAT(stat) Long get##stat();
#define DECLARE_GET_LINK_STAT(stat) Long get##stat(sfpLink_t * link);
FOR_EACH_STAT(DECLARE_GET_STAT)
FOR_EACH_LINK_STAT(DECLARE_GET_LINK_STAT)

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
extern sfpLink_t alink;

#endif // MOCKS_H
