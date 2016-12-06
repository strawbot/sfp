// SFP Services: sending packets and packet handlers  Robert Chapman III  Feb 20, 2015

#include <stdlib.h>

#include "framepool.h"
#include "frame.h"
#include "services.h"
#include "link.h"
#include "node.h"
#include "stats.h"
#include "sfpTxSm.h"
#include "printers.h"

// Handlers and frame list
static struct {
	packetHandler_t handler;
	sfpFrame * list;
} packetHandlers[MAX_PIDS] = {{NULL,NULL}};

// vectorizable packet handlers
packetHandler_t getPacketHandler(Byte pid)
{
	if (pid < MAX_PIDS)
		return packetHandlers[pid].handler;
	return NULL;
}

sfpFrame * getHandlerFrame(Byte pid)
{
	if (pid < MAX_PIDS)
		return packetHandlers[pid].list;
	return NULL;
}

packetHandler_t setPacketHandler(Byte pid, packetHandler_t handler)
{
	packetHandler_t oldHandler = getPacketHandler(pid);
	
	if (pid < MAX_PIDS)
		packetHandlers[pid].handler = handler;
	return oldHandler;
}

static void linkFrame(sfpFrame *frame)
{
	sfpFrame * link = (sfpFrame *)&packetHandlers[frame->pid].list;
	
	while (link->list != NULL) // find end of list
		link = link->list;
	link->list = frame;
	frame->list = NULL;
}

static void unlinkFrame(Byte n)
{
	sfpFrame * link = (sfpFrame *)&packetHandlers[n].list;
	
	link->list = link->list->list;
}

// SFP Frame decoder 
#define PRINT_PID(pid)	case pid: print(#pid); break;

static void decodeFrame(sfpFrame *frame)
{
	print("\n");
	switch(frame->pid) {
//	FOR_EACH_PID(PRINT_PID)
	}
	print(" frame from "), printDec(frame->who.from), print("to "), printDec(frame->who.to);
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

static void routeFrame(sfpFrame *frame)
{
    sfpLink_t *tolink = routeTo(frame->who.to);

	if (tolink) {
		ReRouted();
		// transfer received frame from one link to a different one
		if (frame->pid & ACK_BIT) // check to see if SPS packet
			pushq((Cell)frame, tolink->spsq);
		else
			pushq((Cell)frame, tolink->npsq);
	}
	else
		returnFrame(frame);
}

static void processLinkFrame(sfpFrame * frame, sfpLink_t *link)
{
	if (link->disableSps) {
		if (frame->pid > MAX_PIDS) {
			UnknownPid();
			returnFrame(frame);
			return;
		}
	}
	else if (frame->pid & ACK_BIT) { // ack SPS packets
		spsReceived(link);
		if (!acceptSpsFrame(frame, link)) {
			returnFrame(frame);
			return;
		}
	}

	// unhandled cases:
	// 1. a pid > MAX_PIDS with SPS is received with SPS enabled
	// 2. a pid > MAX_PIDS without SPS is received with SPS enabled

	// note: with sps acked 1st, then routing done second, each link is acked
	// and it will reduce end to end send time
	if ( (frame->pid & PID_BITS) > WHO_PIDS) // check for routing of packet
        if (frame->who.to) { // is there a destination?
			if (whoami() == ME) // do i need an identity? (multi drop)
				setWhoami(frame->who.to); // become the destination
			else if (frame->who.to != whoami()) { // is it not for me?				
				routeFrame(frame);
				return;
			}
        }

	frame->pid &= PID_BITS; // strip sps bits

	if (getPacketHandler(frame->pid) != NULL) // queue up for installed packet handler
		linkFrame(frame);
	else { // default behaviuor for no handlers
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
			case CONFIG: // ignore config frames
				IgnoreFrame();
				break;
			default:
				UnknownPid();
				break;
		}
		returnFrame(frame);
	}
}

/* packet handlers
	Frames come in over links and go to a frame queue. A machine processes the
	frame and then puts it into the the handler list if there is a handler. Otherwise
	it tries to process it as a link only frame. It stays in the handler list
	until it is either processed by the handler or becomes stale.
	
	There are multiple possible actions and paths for frames:
	 o an ACK packet signals the SPS service and is done
	 o an SPS packet signals the SPS service and is queued
	 o a network packet not for this node is rerouted
	 o other frame level services are handled
	 o if none of the above, the frame is appended to the handler packet list
	 
	Link level frames have no higher level purpose. They have a pid but not
	all have networking. All frames are processed by packet handlers. If there is
	not one, then some have default handlers.
*/
void handleFrame(Byte n)
{
	packetHandler_t handler = getPacketHandler(n);
	sfpFrame * frame = packetHandlers[n].list;
	
	if (handler != 0 && frame != 0) {
		if (handler(frame->packet, frame->length - FRAME_OVERHEAD))
			PacketProcessed();
		else if ((getTime() - frame->timestamp) > STALE_RX_FRAME) // check for stale frame
			UnDelivered();
		else
			return;
		unlinkFrame(frame->pid);
		returnFrame(frame);
	}
}

void runHandlers(void) // call handlers for frames
{
	for (Byte n = 0; n < MAX_PIDS; n++)
		handleFrame(n);
}

void distributeFrames(void)
{
	for (Byte n = 0; n < NUM_LINKS; n++) {
        sfpLink_t *link = nodeLink(n); // scan for available link

		if (link == 0)
			continue;

		if (queryq(link->receivedPool) != 0) { // check for queued frames
			sfpFrame *frame = (sfpFrame *)pullq(link->receivedPool);
			
			if (link->listFrames) // debug if enabled
				decodeFrame(frame);

			processLinkFrame(frame, link); // deal with the frame
			FrameProcessed();
		}
	}
}

void processFrames(void) // queue up frame for handler
{
	distributeFrames();
	runHandlers();
}

// Packet services
static bool sendPacketToQ(Byte *packet, Byte length, Qtype *que)
{
	sfpFrame * frame;
	
	if ( (length == 0) || (length > MAX_PACKET_LENGTH) ) {
		PacketSizeBad();
		return false;
	}

	frame = getFrame();

	if (frame == NULL)
		return false;

	buildSfpFrame(length-PID_LENGTH, &packet[1], packet[0], frame);

	pushq((Cell)frame, que);

	return true; // TODO: If for me - accept it?
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

// send frame already allocated from pool
void sendNpsFrame(sfpFrame * frame, Byte packetLength)
{
	sfpLink_t *link;

	link = routeTo(frame->who.to);
	if (link) {
		addSfpFrame(frame, packetLength);
		pushq((Cell)frame, link->npsq);
	} else {
		NoDest();
		returnFrame(frame);
	}
}

// initialization
void initServices(void) //! initialize SFP receiver state machine
{
	for (Byte i=0; i<MAX_PIDS; i++) {
		packetHandlers[i].handler = NULL;
		packetHandlers[i].list = NULL;
	}
}

/*
sending should be from frames; avoid copying; get a frame; fill out packet stuff; send it
or offer service of passing in a packet
two level services; send a packet - get frame, fill it, build it, send it; send a frame;
*/
