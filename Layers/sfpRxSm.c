// SFP RX State Machine  Robert Chapman III  Feb 14, 2012

/*! \file
  The small frame synchronous protocol receiver state machine: Frame level
*/

// Includes
#include "timbre.h"
#include "sfpStats.h"
#include "sfpLink.h"
#include "pids.h"
#include "sfpTxSm.h"
#include "sfpRxSm.h"
#include "packets.h"
#include "node.h"
#include "printers.h"
#include "routing.h"

static Long tracePackets;

void traceOn(void);
void traceOn(void)
{
	tracePackets = 1;
}

void traceOff(void);
void traceOff(void)
{
	tracePackets = 0;
}

// Declarations
void processRxFrames(void);
void processLinkFrame(void *lq);
void newRxSmState(sfpRxState_t state, linkInfo_t *link);

// SPS State Machine support
bool sfpLengthOk(Byte length, linkInfo_t *link) //! check length for a frame
{
	if (length)
	{
		if (length >= MIN_SFP_LENGTH)
		{
			if (length <= MAX_SFP_LENGTH)
				return true;
			else if (length < 255) // ignore float high lines
				LongFrame(length, link->stats);
		}
		else
			ShortFrame(length, link->stats);
	}
	return false;
}

bool checkRxSps(Byte sps, linkInfo_t *link) //! accept or reject incoming sps
{
	switch (link->rxSps)
	{
		case ONLY_SPS0:
			if (sps != 0)
				return false;
			link->rxSps = ONLY_SPS1;
			break;
		case ONLY_SPS1:
			if (sps == 0)
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
	return true;
}

/* 
  Frame Layer processing of received frame
*/
QUEUE(100, rxframeq); // queue between frame layer to packet layer
QUEUE(100, framewaitq); // where to put frames which are in waiting

void processLinkFrame(void *lq)
{
	linkInfo_t *link = (linkInfo_t *)pullq(lq);
	sfpFrame *frame = (sfpFrame*)(&link->frameIn[0]);
	sfpNode_t *n = setNode(link->node);

	link->inFrameState = FRAME_PROCESSED;
	// pass on to packet layer
	if (processPacket((Byte *)&frame->pid, frame->length - FRAME_OVERHEAD, link))
	{
		link->inFrameState = FRAME_EMPTY;
		newRxSmState(HUNTING, link);
	}
	else
	{
		if (checkTimeout(&link->packetTo)) // limit frame receive time for error detection
		{
			UnDelivered(link->stats);
			newRxSmState(HUNTING, link);
			link->inFrameState = FRAME_EMPTY;
		}
		else
		{
			pushq((Cell)link, framewaitq); // queue it for later
			link->inFrameState = FRAME_REQUEUED;
		}
	}
	setNode(n);
}

void processRxFrames(void) // machine to process received frames as packets
{
	if (queryq(framewaitq)) // process waiting frames first
		processLinkFrame(framewaitq);
	if (queryq(rxframeq)) // then any new ones
		processLinkFrame(rxframeq);
	activate(processRxFrames);
}
	
void processSfpFrame(linkInfo_t *link) //! extract pieces of the frame and process it
{
	sfpFrame *frame = (sfpFrame*)(&link->frameIn[0]);

	link->inFrameState = FRAME_FULL;
	if ( (frame->pid & ACK_BIT) != 0 ) // see if ack needed
	{
		setAckSend(link);
		if ( !checkRxSps(frame->pid & SPS_BIT, link) ) // see if not wanted
		{
			link->inFrameState = FRAME_EMPTY;
			newRxSmState(HUNTING, link);
			return;
		}
		frame->pid &= PID_BITS; // strip ack and sps bits
	}
	setTimeout(SFP_FRAME_PROCESS, &link->packetTo);
	safe(pushq((Cell)link, rxframeq)); // process it using another machine
	link->inFrameState = FRAME_QUEUED;
	newRxSmState(PROCESSING, link);
}

// States definitions
void newRxSmState(sfpRxState_t state, linkInfo_t *link)
{
	link->sfpRxState = state;
}

void Hunting(Byte length, linkInfo_t *link) //! waiting for a byte which will be interpreted as a length. It must be not too long and not too short
{
	if (sfpLengthOk(length, link))
	{
		link->sfpBytesToRx = length;
		newRxSmState(SYNCING, link);
	}
	else
		rxLinkError(link);
}

void Syncing(Byte sync, linkInfo_t *link) //! waiting for the complement of the length. If valid and a buffer is available, start receiving a frame
{
	if ( sfpSync(sync) == link->sfpBytesToRx )	// check for sync byte - 1's complement of length
	{
		link->sfpRxPtr = &link->frameIn[0];
		*link->sfpRxPtr++ = link->sfpBytesToRx;
		*link->sfpRxPtr++ = sync;
		newRxSmState(RECEIVING, link);
		link->sfpBytesToRx--;
	}
	else
	{
		rxLinkError(link);
		BadSync(link->stats);
		if (sfpLengthOk(sync, link)) // assume sync is first byte of new frame
			link->sfpBytesToRx = sync;
		else
		{
			link->sfpBytesToRx = 0;
			newRxSmState(HUNTING, link);
		}
	}
}

void Receiving(Byte data, linkInfo_t *link) //! accumulate bytes in frame buffer until length is satisfied
{
	*link->sfpRxPtr++ = data; // store data
	if (--link->sfpBytesToRx == 0) // done receiving bytes
	{
		Byte c1=0, c2=0, length = link->frameIn[0]-1;
		
		calculateFletcherCheckSum(&c1, &c2, length, &link->frameIn[0]);
		if ( (c1 == *(link->sfpRxPtr-2)) && (c2 == data) ) // check for good frame
		{
			GoodFrame(link->stats);
			processSfpFrame(link); // pass on to frame layer
			if (tracePackets)
			{
				print(" Node: "), printDec(whoami());
				print("PID:"), printHex2(link->frameIn[2]);
			}
		}
		else
		{
			rxLinkError(link);
			BadCheckSum(link->stats);
			newRxSmState(HUNTING, link);
		}
	}
}

//! SFP RX state machine
// note: Timeouts set in one state will not be checked for unless there is data
bool sfpRxSm(linkInfo_t *link) // return true if byte processed
{
	Byte data;

	if (link->sfpRx() != 0) // process any received bytes
	{
		switch(link->sfpRxState)
		{
			case HUNTING:
				data = link->sfpGet();
				Hunting(data, link);
				setTimeout(SFP_FRAME_TIME, &link->frameTo);
				return true;
			case SYNCING:
				data = link->sfpGet();
				Syncing(data, link);
				setTimeout(SFP_FRAME_TIME, &link->frameTo);
				return true;
			case RECEIVING:
				data = link->sfpGet();
				Receiving(data, link);
				setTimeout(SFP_FRAME_TIME, &link->frameTo);
				return true;
			case PROCESSING:
				break;
		}
	}
	else // no data so check for timeouts
	{
		switch(link->sfpRxState)
		{
			case SYNCING:
			case RECEIVING:
				if (checkTimeout(&link->frameTo)) // limit frame receive time for error detection
				{
					newRxSmState(HUNTING, link);
					Gaveup(link->stats);
				}
		}
	}
	return false;
}

void initSfpRxSM(linkInfo_t *link) //! initialize SFP receiver state machine
{
	link->rxSps = ANY_SPS;
	newRxSmState(HUNTING, link);
	link->sfpBytesToRx = 0;
	link->frameIn[0] = 0;
	stopTimeout(&link->frameTo);
	stopTimeout(&link->packetTo);
	zeroq(rxframeq);
	zeroq(framewaitq);
	activateOnce(processRxFrames); // note: gets called once for each link
}
