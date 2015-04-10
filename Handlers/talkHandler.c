// Timbre talk handler  Robert Chapman III  Jun 25, 2012

#include "bktypes.h"
#include "localio.h"
#include "library.h"
#include "pids.h"
#include "services.h"
#include "talkhandler.h"
#include "sfp.h"

static bool keyPacket(Byte *packet, Byte length);
static bool evalPacket(Byte *packet, Byte);
static bool talkPacket(Byte *packet, Byte length);

static Byte talkTo = 0;

static bool keyPacket(Byte * packet, Byte length) // feed input into Timbre
{
	packet_t * p = (packet_t *)packet;
	Byte * payload = p->whoload;

	talkTo = p->who.from;
    while (length-- > WHO_HEADER_SIZE)
		keyin(*payload++);
	return true;
}

static bool evalPacket(Byte *packet, Byte l) // silently evaluate input string
{
	packet_t * p = (packet_t *)packet;

	talkTo = p->who.from;
	evaluate(p->whoload);
	return true;
	(void)l;
}

static bool talkPacket(Byte *packet, Byte length) // send packet output to timbre output
{
	packet_t * p = (packet_t *)packet;
    Byte * payload = p->whoload;

    while (length-- > WHO_HEADER_SIZE)
		safe_emit(*payload++);
	return true;
}

Byte talkWho(void) // who are we talking to
{
	return talkTo;
}

void initTalkHandler(void) // install packet handlers
{
	setPacketHandler(TALK_IN, keyPacket);
	setPacketHandler(EVAL, evalPacket);
	setPacketHandler(TALK_OUT, talkPacket);
}
