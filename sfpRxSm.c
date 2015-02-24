// SFP RX State Machine  Robert Chapman III  Feb 14, 2012

/*! \file
  The small frame protocol receiver state machine: Frame level
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
void newRxSmState(sfpRxState_t state, sfpLink_t *link);
void pushFrame(sfpLink_t *link);

// SPS State Machine support
bool sfpLengthOk(Byte length, sfpLink_t *link) //! check length for a frame
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

bool checkRxSps(Byte sps, sfpLink_t *link) //! accept or reject incoming sps
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

// States definitions
void newRxSmState(sfpRxState_t state, sfpLink_t *link)
{
	link->sfpRxState = state;
}

void Hunting(Byte length, sfpLink_t *link) //! waiting for a byte which will be interpreted as a length. It must be not too long and not too short
{
	if (sfpLengthOk(length, link))
	{
		link->sfpBytesToRx = length;
		newRxSmState(SYNCING, link);
	}
	else
		rxLinkError(link);
}

void Syncing(Byte sync, sfpLink_t *link) //! waiting for the complement of the length. If valid and a buffer is available, start receiving a frame
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

void passFrameUp(sfpLink_t *link)
{
			pushq(link->frameIn, link->frameq); // pass on to frame layer
			getNewFrame(link);
	get a new frame for link and write into link->frameIn setting up
	pointers and counters if necessary or just let it happen in HUNTING
	Hunting is responsible for making sure a frame is available or dumping data if not
	and then resetting frame pointers when first byte put into frame or perhaps Sync will
	do that if sync if valid
	RX SM link has 3 values to work with: frame, #bytes to rx, where to put next byte
	SYNC must check for length too long
	HUNTING must check for frame
	RECEIVING gets bytes and looks for end and checks checksum
}

void Receiving(Byte data, sfpLink_t *link) //! accumulate bytes in frame buffer until length is satisfied
{
	*link->sfpRxPtr++ = data; // store data
	if (--link->sfpBytesToRx == 0) // done receiving bytes
	{
		Byte c1=0, c2=0, length = link->frameIn[0]-1;
		
		calculateFletcherCheckSum(&c1, &c2, length, &link->frameIn[0]);
		if ( (c1 == *(link->sfpRxPtr-2)) && (c2 == data) ) // check for good frame
		{
			GoodFrame(link->stats);
			passFrameUp(link);
		}
		else
		{
			rxLinkError(link);
			BadCheckSum(link->stats);
		}
		newRxSmState(HUNTING, link);
	}
}

//! SFP RX state machine
// note: Timeouts set in one state will not be checked for unless there is data
bool sfpRxSm(sfpLink_t *link) // return true if byte processed
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

void initSfpRxSM(sfpLink_t *link) //! initialize SFP receiver state machine
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
