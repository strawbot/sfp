// Remote Timbre connections  Robert Chapman III  Jun 25, 2012

#include "printers.h"
#include "bktypes.h"
#include "timeout.h"
#include "library.h"
#include "pids.h"
#include "stats.h"
#include "link.h"
#include "sfp.h"

bool talkPacket(Byte *packet, Byte length);
void initTalkHandler(void);
void safe_emit(Byte c);

bool talkPacket(Byte *packet, Byte length) // send packet output to timbre output
{
	Byte *payload = ((whoPacket_t *)packet)->payload;

	while (length-- > WHO_PACKET_OVERHEAD)
		safe_emit(*payload++);
	return true;
}

void initTalkHandler(void) // install packet handlers
{
	setPacketHandler(TALK_OUT, talkPacket);
}
