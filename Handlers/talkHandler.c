// Timbre talk handler  Robert Chapman III  Jun 25, 2012

#include "localio.h"
#include "library.h"
#include "services.h"
#include "talkhandler.h"
#include "node.h"

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

void sendeqSfp(void)
{
	Byte c, *p, packet[MAX_PACKET_LENGTH];
	whoPacket_t *w=(whoPacket_t *)packet;
	Byte length = qbq(eq);

	w->pid = TALK_OUT;
	w->who.to = talkTo;
	w->who.from = whoami();
	p = &w->payload[0];
	
	if (length >= MAX_WHO_PAYLOAD_LENGTH)
		length = MAX_WHO_PAYLOAD_LENGTH;
	
	while (length--)
	{
		c = pullbq(eq);
		if (c != 0xD)
			*p++ = c;
	}
	
	length = (Byte)(p - &w->payload[0]);
	if (length)
	{
		length += WHO_HEADER_SIZE;
		while(!sendNpTo(packet, length, talkTo))
			runMachines();
	}
}

// TODO: need to direct input to rxq or keyq; keep Timbre separate from SFP
void setSfpTalk(void)
{
	setTalkOut(sendeqSfp);
}

void sendeqSerial(void);

void setSerialTalk(void)
{
	setTalkOut(sendeqSerial);
	activate(serialTalk);
}

void initTalkHandler(void) // install packet handlers
{
	setSfpTalk();
	setPacketHandler(TALK_IN, keyPacket);
	setPacketHandler(EVAL, evalPacket);
	setPacketHandler(TALK_OUT, talkPacket);
}
