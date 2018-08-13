// SFP TX state machine  Robert Chapman III  Feb 16, 2012
// Includes
#include "node.h"
#include "services.h"
#include "frame.h"
#include "framePool.h"
#include "sfpRxSm.h"
#include "sfpTxSm.h"
#include "stats.h"

#include <string.h>

/*
 Transmitter is either sending a frame, or waiting for a frame
 if an sps frame is ready to send then it is sent before the nps frame
 once an sps frame is sent a timer is set to prevent it from being sent again immediately
 if an ack is recieve, the sps frame is returned
 if a timer times out, then the sps frame is resent
 if the sps frame is resent too many times, then it is discarded
 an spi link must be polled to see if there are frames to be received
*/
Byte pollTrain = 3; // how many consecutive polls to send - empirically determined
static sfpFrame pollFrame;
static sfpFrame ackFrame;

// hook for interrupts and other things
void frameOut(sfpFrame * frame);

typedef enum { NOT_POOL, FROM_POOL }frame_t;

//! set a frame up for transmission
static bool transmitFrame(sfpFrame *frame, sfpLink_t *link, frame_t source) {
    Byte length = frame->length+LENGTH_LENGTH;
    
    if (link->txq) { // use queue if its there
        Long left = leftbq(link->txq);
        if (left < length)
            return false;
            
        Byte * data = frame->content;
        
        while (length--)
            pushbq(*data++, link->txq);
        if (source == FROM_POOL)
            ireturnFrame(frame);
 		frameOut(frame);
 		
        return true;
    }
    
    if (link->sfpBytesToTx == 0) { // frame has moved on
		link->sfpTxPtr = frame->content; // set this first
		link->sfpBytesToTx = length; // set this second
		if (link->frameOut)
            ireturnFrame(link->frameOut);
		link->frameOut = (source == FROM_POOL) ? frame : NULL;
 		frameOut(frame);
 		
		return true;
	}
	
	return false;
}

// support Functions
static void transmitAckFrame(sfpLink_t *link) {
	if (transmitFrame(&ackFrame, link, NOT_POOL)) {
		SendFrame(link);
		clearAckSend(link);
	}
}

static void transmitSpsFrame(sfpLink_t * link) {
    if (queryq(link->spsq)) {
        if (transmitFrame((sfpFrame *)q(link->spsq), link, NOT_POOL)) {
            clearSpsSend(link);
            SpsSent(link);
        }
    } else {
        clearSpsSend(link);
        SpsqUnderflow(link); // this happens because frame was never sent!
    }
}

static void transmitNpsFrame(sfpLink_t *link) {
    if (transmitFrame((sfpFrame *)q(link->npsq), link, FROM_POOL)) {
        pullq(link->npsq);
		SendFrame(link);
	}
}

static void transmitPollFrame(sfpLink_t *link) {
	Byte n = pollTrain;

	if (bytesToReceive(link) > n)
		n = bytesToReceive(link);
    pollFrame.length = n;
    transmitFrame(&pollFrame, link, NOT_POOL);
	clearPollSend(link);
	PollFrame(link);
}

// SPS transmitter state machine
static void setSpsState(sfpLink_t * link, spsState_t state) {
	link->txSps = state;
}

static void setSpsBits(sfpLink_t * link) {
	sfpFrame * frame = (sfpFrame *)q(link->spsq);
	
	frame->pid &= PID_BITS;

	if (link->txSps == ONLY_SPS0) {
		frame->pid |= ACK_BIT;
        setSpsState(link, WAIT_ACK0);
    }
	else {
		frame->pid |= ACK_BIT | SPS_BIT;
		setSpsState(link, WAIT_ACK1);
	}
	addChecksum(frame); // must recalculate since bits were changed
}

static void checkSps(sfpLink_t * link) {
	if (link->disableSps) return;

    if (testAckReceived(link)) {
        clearAckReceived(link);
		SpsAcked(link);

		switch(link->txSps) {
		case ANY_SPS:
		case NO_SPS:
            setSpsState(link, ONLY_SPS0);
			break;
		case ONLY_SPS0:
		case ONLY_SPS1:
            UnexpectedAck(link);
			break;
		case WAIT_ACK0:
			ireturnFrame((sfpFrame *)pullq(link->spsq));
            setSpsState(link, ONLY_SPS1);
			break;
		case WAIT_ACK1:
			ireturnFrame((sfpFrame *)pullq(link->spsq));
            setSpsState(link, ONLY_SPS0);
            break;
		}
	}

	switch(link->txSps) {
	case ANY_SPS:
	case NO_SPS:
        if (checkTimeout(&link->spsTo)) {
         	sfpFrame * frame = igetFrame();
       	
        	if (frame != NULL) {
				who_t who = {DIRECT, whoami()};

    			buildSfpFrame(sizeof(who), &who.to, SPS, frame); // empty frame
				stuffq((Cell)frame, link->spsq); // make sure it is first out
                setSpsState(link, ONLY_SPS0);
			}
			else
				setTimeout(SFP_SPS_TIME, &link->spsTo);
		}
		break;
	case ONLY_SPS0:
	case ONLY_SPS1:
		if (queryq(link->spsq)) {
            setTimeout(SFP_SPS_TIME, &link->spsTo);
			setSpsBits(link);
			setSpsSend(link);
			link->spsRetries = 0;
		}
		break;
	case WAIT_ACK0:
	case WAIT_ACK1:
        if (checkTimeout(&link->spsTo)) {
            if (link->spsRetries < SPS_RETRIES) {
                setTimeout(SFP_SPS_TIME, &link->spsTo);
				link->spsRetries += 1;
				setSpsSend(link);
			}
			else {
				ireturnFrame((sfpFrame *)pullq(link->spsq)); // what if frame not sent but pending?
                setSpsState(link, NO_SPS);
				SpsTimeout(link);
				if (queryq(link->spsq) == 0)
					clearSpsSend(link);
            }
		}
		break;
	}
}

// API
void spsAcknowledged(sfpLink_t *link) {
	setAckReceived(link);
}

void spsReceived(sfpLink_t *link) {
	setAckSend(link);
}

void transmitSfpByte(sfpLink_t *link) { // send a byte if transmitter is able to
    if (link->sfpTx(link))
	{
		link->sfpBytesToTx--;
        link->sfpPut(*link->sfpTxPtr++, link);
	}
}

void serviceTx(sfpLink_t *link) { // try to send a byte if there are bytes to send
	if (link->sfpBytesToTx)
		transmitSfpByte(link);
}

void serviceTxq(sfpLink_t *link) { // try to send a byte to q queue if there are bytes to send
    Cell l = link->sfpBytesToTx;
    Cell q = leftbq(link->txq);
	Cell n =  l < q ? l : q;
	
	link->sfpBytesToTx -= n;

	while (n--)
	    pushbq(*link->sfpTxPtr++, link->txq);
}

void serviceMasterTx(sfpLink_t *link) { // try to send a byte if there are bytes to send
    if (link->sfpBytesToTx)
        transmitSfpByte(link);
    if (checkTimeout(&link->pollTo)) {
        setPollSend(link);
        setTimeout(SFP_POLL_TIME, &link->pollTo);
    }
}

void resetTransmitter(sfpLink_t *link) {
	link->sfpBytesToTx = 0;
	link->txFlags = 0;

	if (link->frameOut) // frame has been transmitted return if needed
		ireturnFrame(link->frameOut);
	link->frameOut = NULL;

	while(queryq(link->npsq))
		ireturnFrame((sfpFrame *)pullq(link->npsq));
}

void sfpTxSm(sfpLink_t *link) { //! continue to send a frame or start a new one or just exit if all done
    checkSps(link);
	// prioritized transmission actions
    if 		(testAckSend(link))		transmitAckFrame(link);
    else if (testSpsSend(link))		transmitSpsFrame(link);
    else if (queryq(link->npsq))    transmitNpsFrame(link);
    else if (testPollSend(link))	transmitPollFrame(link);
}

void initSfpTxSM(sfpLink_t *link, Qtype * npsq, Qtype * spsq) { //! initialize SFP receiver state machine
	who_t who = {DIRECT, DIRECT};

	setTimeout(SFP_SPS_TIME, &link->spsTo);  // startup sps service - use giveup timeout
	memset(&pollFrame, 0, sizeof(pollFrame)); // set to all zeroes
    buildSfpFrame(sizeof(who), &who.to, SPS_ACK, &ackFrame); // one time build of ACK starter frame

	link->sfpBytesToTx = 0;
    setSpsState(link, NO_SPS);
	link->txFlags = 0;
    link->frameOut = 0;
    link->serviceTx = serviceTx;
	link->txSps = NO_SPS;

    zeroq(npsq);
    if (spsq) {
    	zeroq(spsq);
     	link->spsq = spsq;
    	link->disableSps = false;
    }
    else {
    	link->disableSps = true;
     	link->spsq = npsq;
    }
	link->npsq = npsq;
}

