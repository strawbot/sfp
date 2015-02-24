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
#include "routing.h"

?Should handlers be in their own file? Services just contains sending services?
static Timeout frameTo;  // how long to hold on to a received frame before tossing it
static QUEUE(MAX_FRAMES, frameq); // where to put frames which are in waiting
static packetHandler_t packetHandlers[MAX_PIDS] = {NULL};

// local declarations
static bool processFrame(sfpFrame * frame);
static bool processPacket(sfpFrame * frame);
static void retryFrames(void);
static void processFrames(void);
static bool sendPacketToQ(Byte *packet, Byte length, Qtype *que);
static void reRouteFrame(sfpFrame *frame);

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

static void processFrames(void) // machine to process received frames from links
{
	Long n = 0;
	sfpLink_t *link;

	while ((link = nodeLink(n++)) != NULL) {		
		if (queryq(link->frameq) != 0) {
			sfpFrame *frame = (sfpFrame *)pullq(link->frameq);
			
			if (processFrame(frame))
				FrameProcessed();
			else
				pushq((Cell)frame, frameq);
		}
	}
	activate(processFrames);
}

static void retryFrames(void) // machine to process packets for busy handlers
{
	if (queryq(frameq)) { // process waiting frames
		sfpFrame *frame = (sfpFrame*)q(frameq);

        if (processPacket(frame)) {
			PacketProcessed();
            returnFrame((sfpFrame*)pullq(frameq));
		}
		else if (checkTimeout(&frameTo)) {
			UnDelivered();
            returnFrame((sfpFrame*)pullq(frameq));
		}
	}
	activate(retryFrames);
}

static bool processFrame(sfpFrame * frame)
{
	if ( (frame->pid & ACK_BIT) != 0 ) { // intercept SPS packets
		if (!acceptSpsFrame(frame)) {
			returnFrame(frame);
			return true;
		}
		frame->pid &= PID_BITS; // strip ack and sps bits
	}

	if (frame->pid > WHO_PIDS) // check for routing of packet
        if (frame->who.to) { // is there a destination?
			if (whoami() == ME) // do i need an identity? (multi drop)
				setMe(frame->who.to); // become the destination
			else if (frame->who.to != whoami()) { // is it not for me?				
				reRouteFrame(frame);
				return true;
			}
        }

	// give first crack at packet to vectored handlers
	packetHandler_t handler = getPacketHandler(frame->pid);

	if (handler != NULL)
		if (!handler(frame->packet, frame->length - FRAME_OVERHEAD))
			return false; // handler busy right now; will try later

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
			setAckReceived(frame->who.from); // pass notice to transmitter
			break;
		case SPS: // null packet used for initializing SPS and setting id
		case PING_BACK: // ignore reply
		case TEST_FRAME: // ignore test frames
			IgnoreFrame();
			break;
		default:
			UnknownPid();
			break;
	}
	returnFrame(frame);
	return true;
}

static bool processPacket(sfpFrame * frame)
{
	packetHandler_t handler = getPacketHandler(frame->pid);

	if (handler != NULL)
		return handler(frame->packet, frame->length - FRAME_OVERHEAD);

	UnknownPid();
	return true;
}

// Packet services
static void reRouteFrame(sfpFrame *frame)
{
    sfpLink_t *link = routeTo(frame->who.to);
	
	if (link) {
		ReRouted();
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
	
	if (link)
        return sendPacketToQ(packet, length, link->spsq);

	// TODO: If for me - accept it?
	NoDest();
	return true;
}

// initialization
void initServices(void) //! initialize SFP receiver state machine
{
	setTimeout(STALE_RX_FRAME, &frameTo);
	zeroq(frameq);
	activateOnce(processFrames); // note: gets called once for each link
	activateOnce(retryFrames);
}

/*
sending should be from frames; avoid copying; get a frame; fill out packet stuff; send it
or offer service of passing in a packet
two level services; send a packet - get frame, fill it, build it, send it; send a frame;
*/
