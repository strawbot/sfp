
#include "sfsp.h"
#include "sfp.h"

bool processPacket(Byte *packet, Byte length, linkInfo_t *link);

bool sendNormalPacket(Byte *packet, Byte length);
bool sendNormalPacketLink(Byte *packet, Byte length, linkInfo_t *link);
bool sendSecurePacket(Byte *packet, Byte length);
bool sendSecurePacketLink(Byte *packet, Byte length, linkInfo_t *link);
bool sendPidLink(Byte pid, linkInfo_t *link);

bool sendSpTo(Byte *packet, Byte length, Byte to);
bool sendPidTo(Byte pid, Byte to);
