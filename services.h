// SFP Services: sending packets and packet handlers  Robert Chapman III  Feb 20, 2015

#include "bktypes.h"

#ifndef _SERVICES_H_
#define _SERVICES_H_

typedef bool (*packetHandler_t)(Byte *packet, Byte length);

#endif

packetHandler_t setPacketHandler(Byte pid, packetHandler_t handler);
packetHandler_t getPacketHandler(Byte pid);

bool sendNpsFrame(sfpFrame *frame);

bool sendNpTo(Byte *packet, Byte length, Byte to);
bool sendSpTo(Byte *packet, Byte length, Byte to);

void initServices(void);