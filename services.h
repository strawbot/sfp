// SFP Services: sending packets and packet handlers  Robert Chapman III  Feb 20, 2015

<<<<<<< HEAD
#include "bktypes.h"
=======
#include "sfp.h"
>>>>>>> cfdb340e73553904cd3c921a62d8d25bcdea5095

#ifndef _SERVICES_H_
#define _SERVICES_H_

typedef bool (*packetHandler_t)(Byte *packet, Byte length);

#endif

packetHandler_t getPacketHandler(Byte pid);
packetHandler_t setPacketHandler(Byte pid, packetHandler_t handler);

void processFrames(void);

bool sendNpTo(Byte *packet, Byte length, Byte to);
bool sendSpTo(Byte *packet, Byte length, Byte to);
void queueFrame(sfpFrame *frame, Byte packetlength);

void initServices(void);
