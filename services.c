// SFP Services: sending packets and packet handlers  Robert Chapman III  Feb 20, 2015

#include <stdlib.h>

#include "framepool.h"
#include "frame.h"
#include "services.h"
#include "sfp.h"
#include "link.h"
#include "node.h"
#include "stats.h"
#include "sfpTxSm.h"

// ?Should handlers be in their own file? Services just contains sending services?
// but then one does not use one without the other - so keep in one file
static Timeout retryTo;  // how long to hold on to a received frame before tossing it
static QUEUE(MAX_FRAMES, retryq); // where to put frames which are in waiting
static packetHandler_t packetHandlers[MAX_PIDS] = {NULL};

// local declarations
static bool processLinkFrame(sfpFrame * frame, sfpLink_t *link);
static bool processPacket(sfpFrame * frame);
static void retryFrames(void);
static bool sendPacketToQ(Byte *packet, Byte length, Qtype *que);
static void reRouteFrame(sfpFrame *frame);
static bool acceptSpsFrame(sfpFrame * frame, sfpLink_t *link);

// vectorizable packet handlers
packetHandler_t setPacketHandler(Byte pid, packetHandler_t handler)
{
	packetHandler_t oldHandler = getPacketHandler(pid);
	
	if (pid < MAX_PIDS)
		packetHandlers[pid] = handler;
	return oldHandler;
}

packetHandler_t getPacketHandler(Byte pid)
{
	if (pid < MAX_PIDS)
		return packetHandlers[pid];
	return NULL;
}

/* default packet handlers
	Frames come in over links and go to frame queues. A machine processes the
	frame and then puts it into the retry queue if the packet handler is busy.
	Another machine processes the retry queue.
	
	There are multiple possible actions and paths for frames:
	 o an ACK packet signals the SPS service and is done
	 o an SPS packet signals the SPS service and is queued
	 o a network packet not for this node is rerouted
	 o other frame level services are handled
	 o if none of the above, the frame is pushed to the nodes packet queue
	 
	Link level frames that have no higher level purpose. They have a pid but not
	all have networking. All frames are processed by packet handlers. If there is
	not one, then some have default handlers. If a packet handler does not accept
	the packet, then it stays in the queue until it is accepted or a timeout occurs.
*/

void processFrames(void) //process received frames from links
{
	Long n = 0;

	for (n = 0; n < NUM_LINKS; n++) {
        sfpLink_t *link = nodeLink(n);
		if (link == 0)
			continue;
		if (queryq(link->frameq) != 0) {
			sfpFrame *frame = (sfpFrame *)pullq(link->frameq);
			
			if (processLinkFrame(frame, link))
				FrameProcessed();
			else {
				if (queryq(retryq) == 0)
					setTimeout(STALE_RX_FRAME, &retryTo);
				pushq((Cell)frame, retryq);
			}
		}
	}
	retryFrames();
}

/* SPS routing
 SPS is only over a single link. To be network wide, there would have to be state
 machines for each possible connection. While possible, it complicates matters.
 To simplify, SPS is only over a single link. In the case of a frame from that only
 goes over one link this is fine. But if the frame needs to go further then either:
  o drop it and keep a stat
  o send the ack, but route the frame as NPS
  o send the ack and route the frame as SPS ? would this work?
  > ACK frame should not have routing 
*/
static bool processLinkFrame(sfpFrame * frame, sfpLink_t *link)
{
	if (frame->pid & ACK_BIT) { // intercept SPS packets
		spsReceived(link);
		if (!acceptSpsFrame(frame, link)) {
			returnFrame(frame);
			return true;
		}
	}

	if ( (frame->pid & PID_BITS) > WHO_PIDS) // check for routing of packet
        if (frame->who.to) { // is there a destination?
			if (whoami() == ME) // do i need an identity? (multi drop)
				setWhoami(frame->who.to); // become the destination
			else if (frame->who.to != whoami()) { // is it not for me?				
				reRouteFrame(frame);
				return true;
			}
        }

	frame->pid &= PID_BITS; // strip sps bits

	// give first crack at packet to vectored handlers
	packetHandler_t handler = getPacketHandler(frame->pid);

	if (handler) {
		if (handler(frame->packet, frame->length - FRAME_OVERHEAD))
			PacketProcessed();
		else
			return false; // handler busy right now; will try later
	}
	else {
		switch(frame->pid) // intercept link only packets - no destination id
		{
			case PING: // other end is querying so reply
				{	
					Byte ping[3];
				
					ping[0] = PING_BACK;
					ping[1] = frame->who.from;
					ping[2] = whoami();
				
					sendNpTo(ping, sizeof(ping), frame->who.from);
				}
				break;
			case SPS_ACK: // ack frame for SPS
				spsAcknowledged(link); // pass notice to transmitter
				break;
			case SPS: // null packet used for initializing SPS and setting id
			case PING_BACK: // ignore reply
			case CONFIG: // ignore test frames
				IgnoreFrame();
				break;
			default:
				UnknownPid();
				break;
		}
	}
	returnFrame(frame);
	return true;
}

static void retryFrames(void) //process packets for busy handlers
{
	if (queryq(retryq)) { // process waiting frames
		sfpFrame *frame = (sfpFrame*)q(retryq);

        if (processPacket(frame)) {
			PacketProcessed();
            returnFrame((sfpFrame*)pullq(retryq));
		}
		else if (checkTimeout(&retryTo)) {
			UnDelivered();
            returnFrame((sfpFrame*)pullq(retryq));
            if (queryq(retryq) != 0)
				setTimeout(STALE_RX_FRAME, &retryTo);
		}
	}
}

static bool processPacket(sfpFrame * frame)
{
	packetHandler_t handler = getPacketHandler(frame->pid);

	if (handler != NULL)
		return handler(frame->packet, frame->length - FRAME_OVERHEAD);

	UnknownPid();
	return true;
}
void breakPoint1(void);

static bool acceptSpsFrame(sfpFrame * frame, sfpLink_t *link) //! accept or reject incoming sps
{
    Byte sps = frame->pid & SPS_BIT;

    if (link == 0) {
        NoDest();
    }
    else {
        switch (link->rxSps) {
        case ONLY_SPS0:
            if (sps)
                return false;
            link->rxSps = ONLY_SPS1;
            break;
        case ONLY_SPS1:
            if (!sps)
                return false;
            link->rxSps = ONLY_SPS0;
            break;
        default:
            if (sps)
                link->rxSps = ONLY_SPS0;
            else
                link->rxSps = ONLY_SPS1;
            break;
        }
    }
	return true;
}


// Packet services
static void reRouteFrame(sfpFrame *frame)
{
    sfpLink_t *link = routeTo(frame->who.to);
	
	if (link) {
		ReRouted();
		if (frame->pid & ACK_BIT) // check to see if SPS packet
			pushq((Cell)frame, link->spsq);
		else
			pushq((Cell)frame, link->npsq);
	}
	else {
		UnRouted();
		returnFrame(frame);
	}
}

static bool sendPacketToQ(Byte *packet, Byte length, Qtype *que)
{
	sfpFrame *frame;
	
	if ( (length == 0) || (length > MAX_PACKET_LENGTH) ) {
		PacketSizeBad();
		return false;
	}

	frame = getFrame();
	if (frame == NULL)
		return false;

	buildSfpFrame(length-1, &packet[1], packet[0], frame);

	pushq((Cell)frame, que);

	return true; // TODO: If for me - accept it?
}

void queueFrame(sfpFrame *frame, Byte packetlength) // frame and queue a frame pool frame
{
	sfpLink_t *link = routeTo(frame->who.to);
	
	addSfpFrame(frame, packetlength);
	if (frame->pid & ACK_BIT) // check for SPS bit; need to isolate this - data hide
		pushq((Cell)frame, link->spsq);
	else
		pushq((Cell)frame, link->npsq);
}

bool sendNpTo(Byte *packet, Byte length, Byte to) //! send a packet using NPS
{
	sfpLink_t *link = routeTo(to);
	
	if (link)
        return sendPacketToQ(packet, length, link->npsq);

	// TODO: If for me - accept it?
	NoDest();
	return true;
}

bool sendSpTo(Byte *packet, Byte length, Byte to) //! send a packet using SPS
{
	sfpLink_t *link = routeTo(to);
	
	if (link) {
		if (link->txSps == NO_SPS)
			return false;
		return sendPacketToQ(packet, length, link->spsq);
	}
		
	// TODO: If for me - accept it?
	NoDest();
	return true;
}

// initialization
void initServices(void) //! initialize SFP receiver state machine
{
	setTimeout(STALE_RX_FRAME, &retryTo);
	zeroq(retryq);
}

/*
sending should be from frames; avoid copying; get a frame; fill out packet stuff; send it
or offer service of passing in a packet
two level services; send a packet - get frame, fill it, build it, send it; send a frame;
*/
