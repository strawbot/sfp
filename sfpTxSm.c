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

// debugging
void frameOut(sfpFrame * frame);
void txSpsState(sfpLink_t * link, spsState_t state);

// setup for transmitter
static bool transmitFrame(sfpFrame *frame, sfpLink_t *link) //! set a frame up for transmission
{
	if (link->sfpBytesToTx == 0)
	{
// 		void frameOut(sfpFrame * frame);
// 		frameOut(frame);
		link->sfpTxPtr = &frame->length; // set this first
		link->sfpBytesToTx = frame->length + LENGTH_LENGTH; // set this second
		return true;
	}
	return false;
}

static void transmitPoll(Byte n, sfpLink_t *link) //! set number of poll bytes for transmission
{
	if (link->sfpBytesToTx == 0)
	{
		link->sfpTxPtr = &pollFrame.length; // set this first
		link->sfpBytesToTx = n; // set this second
	}
}

// support Functions
static void sendAckFrame(sfpLink_t *link)
{
	if (transmitFrame(&ackFrame, link)) {
		SendFrame(link);
		clearAckSend(link);
	}
}

static void sendSpsFrame(sfpLink_t * link)
{
    if (queryq(link->spsq)) {
        if (transmitFrame((sfpFrame *)q(link->spsq), link)) {
            clearSpsSend(link);
            SpsSent(link);
       }
    }
    else {
        clearSpsSend(link);
        SpsqUnderflow(link); // this happens because frame was never sent!
    }
}

static void sendNpsFrame(sfpLink_t *link)
{
    sfpFrame * frame = (sfpFrame *)q(link->npsq);

    if (transmitFrame(frame, link))
	{
		if (link->frameOut) // frame has been transmitted return if needed
			ireturnFrame(link->frameOut);
        link->frameOut = frame; // set for returning when done
        pullq(link->npsq);

		SendFrame(link);
	}
}

static void sendPollFrame(sfpLink_t *link)
{
	Byte n = pollTrain;

	if (bytesToReceive(link) > n)
		n = bytesToReceive(link);
	transmitPoll(n,link);
	clearPollSend(link);
	PollFrame(link);
}

// SPS transmitter state machine
static void setSpsState(sfpLink_t * link, spsState_t state)
{
	link->txSps = state;
}

static void setSpsBits(sfpLink_t * link)
{
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

static void checkSps(sfpLink_t * link)
{
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
void spsAcknowledged(sfpLink_t *link)
{
	setAckReceived(link);
}

void spsReceived(sfpLink_t *link)
{
	setAckSend(link);
}

void transmitSfpByte(sfpLink_t *link) // send a byte if transmitter is able to
{
    if (link->sfpTx(link))
	{
		link->sfpBytesToTx--;
        link->sfpPut(*link->sfpTxPtr++, link);
	}
}

void serviceTx(sfpLink_t *link) // try to send a byte if there are bytes to send
{
	if (link->sfpBytesToTx)
		transmitSfpByte(link);
}

void serviceMasterTx(sfpLink_t *link) // try to send a byte if there are bytes to send
{
    if (link->sfpBytesToTx)
        transmitSfpByte(link);
    if (checkTimeout(&link->pollTo)) {
        setPollSend(link);
        setTimeout(SFP_POLL_TIME, &link->pollTo);
    }
}

void resetTransmitter(sfpLink_t *link)
{
	link->sfpBytesToTx = 0;
	link->txFlags = 0;

	if (link->frameOut) // frame has been transmitted return if needed
		ireturnFrame(link->frameOut);
	link->frameOut = NULL;

	while(queryq(link->npsq))
		ireturnFrame((sfpFrame *)pullq(link->npsq));
}

void sfpTxSm(sfpLink_t *link) //! continue to send a frame or start a new one or just exit if all done
{
    checkSps(link);
	// prioritized transmission actions
    if 		(testAckSend(link))		sendAckFrame(link);
    else if (testSpsSend(link))		sendSpsFrame(link);
    else if (queryq(link->npsq))    sendNpsFrame(link);
    else if (testPollSend(link))	sendPollFrame(link);
    else if (link->sfpBytesToTx == 0) {
		if (link->frameOut) // frame has been transmitted return if needed
			ireturnFrame(link->frameOut);
		link->frameOut = NULL;
	}
}

void initSfpTxSM(sfpLink_t *link, Qtype * npsq, Qtype * spsq) //! initialize SFP receiver state machine
{
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

