#ifndef MOCKS_H
#define MOCKS_H

#include "sfp.h"
#include "stats.h"
#include "link.h"
#include "node.h"

bool framePoolFull();

#define DECLARE_GET_NODE_STAT(stat) Long get##stat();
#define DECLARE_GET_LINK_STAT(stat) Long get##stat(sfpLink_t * link);
FOR_EACH_NODE_STAT(DECLARE_GET_NODE_STAT)
FOR_EACH_LINK_STAT(DECLARE_GET_LINK_STAT)

extern bool verbose;
extern sfpLink_t alink;
extern sfpNode_t anode;
extern Byte packet[3];

void initTestLink();
void initTestNode();
Long acksIn();
Long acksOut();
void runSm(Long n);
void frameOut(sfpFrame * frame);
void frameIn(sfpFrame * frame);
extern bool exitflag;
void breakPoint1(void);

void rxFrame(sfpFrame * frame);
void runDistrubuter();

void setTime(Long t);

#endif // MOCKS_H
