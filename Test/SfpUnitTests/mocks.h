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

extern sfpLink_t alink;
extern Byte packet[3];

void initLink();
void initNode();
Long acksIn();
Long acksOut();

void rxFrame(sfpFrame * frame);

void setTime(Long t);

#endif // MOCKS_H
