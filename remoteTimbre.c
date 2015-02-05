// Remote Timbre connections  Robert Chapman III  Jun 25, 2012

#include "localio.h"
#include "library.h"
#include "timeout.h"

#include "pids.h"
#include "sfpStats.h"
#include "sfsp.h"
#include "packets.h"
#include "printers.h"
#include "routing.h"

bool keyPacket(Byte *packet, Byte length);
bool evalPacket(Byte *packet, Byte);
bool talkPacket(Byte *packet, Byte length); // in talkHandler.c
void initRemoteTimbre(void);

bool keyPacket(Byte *packet, Byte length) // feed input into Timbre
{
	Byte *payload = ((whoPacket_t *)packet)->payload;

	talkTo = ((whoPacket_t *)packet)->who.from;
	while (length-- > WHO_PACKET_OVERHEAD)
		keyin(*payload++);
	return true;
}

bool evalPacket(Byte *p, Byte) // silently evaluate input string
{
	talkTo = ((whoPacket_t *)p)->who.from;
	evaluate(((evaluatePacket_t *)p)->payload);
	return true;
}

void initRemoteTimbre(void) // install packet handlers
{
	setPacketHandler(TALK_IN, keyPacket);
	setPacketHandler(EVAL, evalPacket);
	setPacketHandler(TALK_OUT, talkPacket);
}
