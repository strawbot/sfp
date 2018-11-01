// Timbre talk handler  Robert Chapman III  Jun 25, 2012

#include "cli.h"
#include "services.h"
#include "framePool.h"
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
		keyIn(*payload++);
	return true;
}

static bool evalPacket(Byte *packet, Byte length) // silently evaluate input string
{
	packet_t * p = (packet_t *)packet;
    Byte * string = p->whoload;
    
	talkTo = p->who.from;
    listenQuietly(string, length-WHO_HEADER_SIZE);
	return true;
}

static bool talkPacket(Byte *packet, Byte length) // send packet output to timbre output
{
	packet_t * p = (packet_t *)packet;
    Byte * payload = p->whoload;

    while (length-- > WHO_HEADER_SIZE)
    	safeEmit(*payload++);
	return true;
}

Byte talkWho(void) // who are we talking to
{
	return talkTo;
}

void talkToMe() {
    talkTo = ME;
}

void talkToThem(Byte who) {
	talkTo = who;
}

void sendeqSfp(void)
{
	Cell length = qbq(emitq);

	if (length) {
		sfpFrame * frame = getFrame();
		
		if (frame) {
			Byte *p = &frame->whoload[0];

			if (length >= MAX_WHO_PAYLOAD_LENGTH)
				length = MAX_WHO_PAYLOAD_LENGTH;
	
			while (length--) {
				Byte c = pullbq(emitq);
				
				if (c != 0xD)
					*p++ = c;
			}
			length = (Byte)(p - &frame->whoload[0]);

			if (length) {
				frame->pid = TALK_OUT;
				frame->who.to = talkTo;
				frame->who.from = whoami();
				sendNpsFrame(frame, length + WHO_HEADER_SIZE);
			} else
				returnFrame(frame);
		}
	}
}

// TODO: need to direct input to rxq or keyq; keep Timbre separate from SFP
void initTalkHandler(void) // install packet handlers
{
//	setSfpTalk();
	setPacketHandler(TALK_IN, keyPacket);
	setPacketHandler(EVAL_PID, evalPacket);
	setPacketHandler(TALK_OUT, talkPacket);
}
