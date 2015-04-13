// SFP RX State Machine  Robert Chapman III  Feb 14, 2012

/*! \file
  The small frame protocol receiver state machine: Frame level
*/

// Includes
#include "framepool.h"
#include "frame.h"
#include "stats.h"
#include "link.h"

// Local Declarations
static bool sfpLengthOk(Byte length, sfpLink_t *link);
static void Acquiring(sfpLink_t *link);
static void Hunting(Byte length, sfpLink_t *link);
static void Syncing(Byte sync, sfpLink_t *link);
static void Receiving(Byte data, sfpLink_t *link);
static void checkDataTimeout(sfpLink_t *link);

// length validation
static bool sfpLengthOk(Byte length, sfpLink_t *link) //! check length for a frame
{
	if ( (length != 0) && (length != 255) ) { // special cases for SPI	
		if (length < MIN_FRAME_LENGTH)
        	ShortFrame(length, link);
		else if (length > MAX_FRAME_LENGTH)
        	LongFrame(length, link);
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
		RxLinkError(link);
}

static void Syncing(Byte sync, sfpLink_t *link) //! waiting for the complement of the length. If valid, start receiving a frame
{
	if ( sync == sfpSync(link->sfpBytesToRx) ) { // check for sync byte - 1's complement of length
		link->sfpRxPtr = &link->frameIn->length;
		*link->sfpRxPtr++ = link->sfpBytesToRx--;
		*link->sfpRxPtr++ = sync;
		link->sfpRxState = RECEIVING;
	}
	else {
		RxLinkError(link);
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
        Byte cslength = link->frameIn->length + LENGTH_LENGTH - CHECKSUM_LENGTH;
        checkSum_t cs, *csf = (checkSum_t *)(&link->frameIn->length + cslength);
		
		calculateCheckSum(&cs, cslength, &link->frameIn->length);

		if ( (cs.sum == csf->sum) && (cs.sumsum == csf->sumsum) ) { // check for good frame
			GoodFrame(link);
			pushq((Cell)link->frameIn, link->frameq); // pass on to frame layer
			link->sfpRxState = ACQUIRING;
		}
		else {
			RxLinkError(link);
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
    if (link->sfpRx(link) != 0) // process any received bytes
		switch(link->sfpRxState) {
			case ACQUIRING:
				Acquiring(link);
				return false;
			case HUNTING:
                Hunting(link->sfpGet(link), link);
				return true;
			case SYNCING:
				setTimeout(SFP_FRAME_TIME, &link->frameTo);
                Syncing(link->sfpGet(link), link);
				return true;
			case RECEIVING:
				setTimeout(SFP_FRAME_TIME, &link->frameTo);
                Receiving(link->sfpGet(link), link);
				return true;
		}
	
	if ( (link->sfpRxState == SYNCING) || (link->sfpRxState == RECEIVING) )
		checkDataTimeout(link);

	return false;
}

void initSfpRxSM(sfpLink_t *link, Qtype * frameInq) //! initialize SFP receiver state machine
{
	link->rxSps = ANY_SPS;
	link->sfpRxState = ACQUIRING;
    zeroq(frameInq);
    link->frameq = frameInq;
}
