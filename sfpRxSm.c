// SFP RX State Machine  Robert Chapman III  Feb 14, 2012

/*! \file
  The small frame protocol receiver state machine: Frame level
*/

// Includes
#include "sfp.h"
#include "stats.h"
#include "link.h"
#include "framePool.h"

// 
// #include "sfpLink.h"
// #include "pids.h"
// #include "sfpTxSm.h"
// #include "sfpRxSm.h"
// #include "packets.h"
// #include "node.h"
// #include "printers.h"
// #include "routing.h"

// Local Declarations
static bool sfpLengthOk(Byte length, sfpLink_t *link);
static void Acquiring(sfpLink_t *link);
static void Hunting(Byte length, sfpLink_t *link);
static void Syncing(Byte sync, sfpLink_t *link);
static void Receiving(Byte data, sfpLink_t *link);
static void checkDataTimeout(sfpLink_t *link);

// SPS State Machine support
static bool sfpLengthOk(Byte length, sfpLink_t *link) //! check length for a frame
{
	if ( (length != 0) && (length != 255) ) { // special cases for SPI	
		if (length < MIN_FRAME_LENGTH)
        { ShortFrame(length, link); }
		else if (length > MAX_FRAME_LENGTH)
           { LongFrame(length, link); }
		else 
			return true;
	}
	return false;
}

/* 
  Frame Layer processing of received frame
*/

// States definitions
static void Acquiring(sfpLink_t *link) // waiting to acquire a frame buffer
{
	link->frameIn = getFrame();
	if (link->frameIn != NULL)
		link->sfpRxState = HUNTING;
}

static void Hunting(Byte length, sfpLink_t *link) //! waiting for a byte which will be interpreted as a length. It must be not too long and not too short
{
	if (sfpLengthOk(length, link)) {
		link->sfpBytesToRx = length;
		link->sfpRxState = SYNCING;
	}
	else
		rxLinkError(link);
}

static void Syncing(Byte sync, sfpLink_t *link) //! waiting for the complement of the length. If valid and a buffer is available, start receiving a frame
{
	if ( sync == sfpSync(link->sfpBytesToRx) ) { // check for sync byte - 1's complement of length
		link->sfpRxPtr = &link->frameIn->length;
		*link->sfpRxPtr++ = link->sfpBytesToRx--;
		*link->sfpRxPtr++ = sync;
		link->sfpRxState = RECEIVING;
	}
	else {
		rxLinkError(link);
		BadSync(link);
		
		if (sfpLengthOk(sync, link)) // assume sync is first byte of new frame
			link->sfpBytesToRx = sync; // use sync as length and stay in current state
		else
			link->sfpRxState = HUNTING;
	}
}

static void Receiving(Byte data, sfpLink_t *link) //! accumulate bytes in frame buffer until length is satisfied
{
	*link->sfpRxPtr++ = data; // store data
	if (--link->sfpBytesToRx == 0) // done receiving bytes
	{
		Byte c1=0, c2=0, length = link->frameIn->length;
		
		calculateFletcherCheckSum(&c1, &c2, length, &link->frameIn->length);

		if ( (c1 == *(link->sfpRxPtr-2)) && (c2 == data) ) { // check for good frame
			GoodFrame(link);
			pushq((Cell)link->frameIn, link->frameq); // pass on to frame layer
			link->sfpRxState = ACQUIRING;
		}
		else {
			rxLinkError(link);
			BadCheckSum(link);
			link->sfpRxState = HUNTING;
		}
	}
}

static void checkDataTimeout(sfpLink_t *link)
{
	if (checkTimeout(&link->frameTo)) { // limit frame receive time for error detection
		link->sfpRxState = HUNTING;
		Gaveup(link);
	}
}

//! SFP RX state machine
bool sfpRxSm(sfpLink_t *link) // return true if byte processed
{
	if (link->sfpRx() != 0) // process any received bytes
		switch(link->sfpRxState) {
			case ACQUIRING:
				Acquiring(link);
				return false;
			case HUNTING:
				Hunting(link->sfpGet(), link);
				return true;
			case SYNCING:
				setTimeout(SFP_FRAME_TIME, &link->frameTo);
				Syncing(link->sfpGet(), link);
				return true;
			case RECEIVING:
				setTimeout(SFP_FRAME_TIME, &link->frameTo);
				Receiving(link->sfpGet(), link);
				return true;
		}
	
	if ( (link->sfpRxState == SYNCING) || (link->sfpRxState == RECEIVING) )
		checkDataTimeout(link);

	return false;
}

void initSfpRxSM(sfpLink_t *link) //! initialize SFP receiver state machine
{
	link->rxSps = ANY_SPS;
	link->sfpRxState = ACQUIRING;
}
