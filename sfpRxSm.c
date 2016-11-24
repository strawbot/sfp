// SFP RX State Machine  Robert Chapman III  Feb 14, 2012

/*! \file
  The small frame protocol receiver state machine: Frame level
*/

// Includes
#include "frame.h"
#include "framePool.h"
#include "stats.h"
#include "link.h"

// Local Declarations
static bool sfpLengthOk(Byte length, sfpLink_t *link);
static bool Acquiring(sfpLink_t *link);
static void Hunting(Byte length, sfpLink_t *link);
static void Syncing(Byte sync, sfpLink_t *link);
static void Receiving(Byte data, sfpLink_t *link);

#define EMPTY_FRAME 0
#define NO_RESPONSE 0xFF

// length validation
static bool sfpLengthOk(Byte length, sfpLink_t *link)
{
	if ((length == EMPTY_FRAME) || (length == NO_RESPONSE))
		return false;

	if (length < MIN_FRAME_LENGTH)
	    ShortFrame(length, link);
	else if (length > MAX_FRAME_LENGTH)
        LongFrame(length, link);
    else
		return true;
	
	return false;
}

/* 
  Frame Layer processing of received frame
*/

// States definitions
static bool Acquiring(sfpLink_t *link) // waiting to acquire a frame buffer
{
	link->frameIn = igetFrame();
	if (link->frameIn != NULL) {
		link->sfpRxState = HUNTING;
		return false;
	}
	// toss byte if no frames are available to prevent lockup in interrupt
	link->sfpGet(link);
	return true;
}

static void Hunting(Byte length, sfpLink_t *link) //! waiting for a byte which will be interpreted as a length. It must be not too long and not too short
{
	if (sfpLengthOk(length, link)) {
		link->sfpBytesToRx = length;
		link->sfpRxState = SYNCING;
	}
	else
		BadLength(link);
}

static void Syncing(Byte sync, sfpLink_t *link) //! waiting for the complement of the length. If valid, start receiving a frame
{
	Byte check = sfpSync(link->sfpBytesToRx);
	
	if (sync ==  check) { // check for sync byte - 1's complement of length
		link->sfpRxPtr = &link->frameIn->length;
		*link->sfpRxPtr++ = link->sfpBytesToRx--;
		*link->sfpRxPtr++ = sync;
		link->sfpRxState = RECEIVING;
	}
	else {
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
			link->frameIn->timestamp = getTime();
			pushq((Cell)link->frameIn, link->receivedPool); // pass on to frame layer
// 			void frameIn(sfpFrame * frame);
// 			frameIn(link->frameIn);
			link->sfpRxState = ACQUIRING;
			Acquiring(link);
		}
		else {
			BadCheckSum(link);
			link->sfpRxState = HUNTING;
		}
	}
}

void checkDataTimeout(sfpLink_t *link)
{
	if ( (link->sfpRxState == SYNCING) || (link->sfpRxState == RECEIVING) )
		if (checkTimeout(&link->frameTo)) { // limit frame receive time for error detection
			link->sfpRxState = HUNTING;
			link->sfpBytesToRx = 0;
			RxTimeout(link);
		}
}

//! SFP RX state machine
bool sfpRxSm(sfpLink_t *link)
{
	checkDataTimeout(link);

	if (!link->sfpRx(link))
		return false;

	switch(link->sfpRxState) {
		case ACQUIRING:
			return Acquiring(link);
		case HUNTING:
			Hunting(link->sfpGet(link), link);
			setTimeout(SFP_FRAME_TIME, &link->frameTo); // need if moving to sync
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
}

//! initialize SFP receiver state machine
void initSfpRxSM(sfpLink_t *link, Qtype * receivedPool)
{
	link->rxSps = ANY_SPS;
	link->sfpRxState = ACQUIRING;
    zeroq(receivedPool);
    link->receivedPool = receivedPool;
}
