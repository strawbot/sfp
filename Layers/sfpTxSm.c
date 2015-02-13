// SFP TX state machine  Robert Chapman III  Feb 16, 2012
// Includes
#include "timbre.h"
#include "sfpStats.h"
#include "sfpLink.h"
#include "pids.h"
#include "sfpTxSm.h"
#include "sfpRxSm.h"
#include "packets.h"
#include <stdlib.h>

Byte pollTrain = 3; // how many consecutive polls to send - empirically determined
static Byte pollFrame[MAX_SFP_FRAME] = {0};
static Byte ackFrame[MIN_SFP_FRAME];

void transmitPoll(Byte n, linkInfo_t *link);

void initSfpTxSM(linkInfo_t *link) //! initialize SFP receiver state machine
{
	if (link->enableSps)
		setTimeout(STARTUPSPS_TIME, &link->giveupTo);  // startup sps service - use giveup timeout
	buildSfpFrame(0, NULL, SPS_ACK, ackFrame);
	link->sfpTxState = IDLING;
	link->sfpBytesToTx = 0;
	link->txSps = ANY_SPS;
	link->txFlags = 0;
	link->frameOutNps = link->frameOutNps1;
}

void switchNpsFramesLink(linkInfo_t *link)
{
	if (link->frameOutNps != &link->frameOutNps1[0])
		link->frameOutNps = &link->frameOutNps1[0];
	else
		link->frameOutNps = &link->frameOutNps2[0];
}

void transmitSfpByte(linkInfo_t *link)
{
	if (link->sfpTx())
	{
		link->sfpBytesToTx--;
		link->sfpPut(*link->sfpTxPtr++);
	}
}

bool transmitFrame(Byte *frame, linkInfo_t *link) //! set a frame up for transmission
{
	if (bytesToSend(link) == 0)
	{
		link->sfpTxPtr = frame; // set this first
		link->sfpBytesToTx = frame[0] + 1; // set this second
		return true;
	}
	return false;
}

void transmitPoll(Byte n, linkInfo_t *link) //! set number of poll bytes for transmission
{
	if (bytesToSend(link) == 0)
	{
		SendFrame(link->stats);
		link->sfpTxPtr = pollFrame; // set this first
		link->sfpBytesToTx = n; // set this second
	}
}

void spsAcknowledgedLink(linkInfo_t *link) //! an ack has been received - process accordingly
{
	clearAckReceived(link);
	SpsAcked(link->stats);
	if (link->txSps == WAIT_ACK1)
		link->txSps = ONLY_SPS0;
	else if (link->txSps == WAIT_ACK0)
		link->txSps = ONLY_SPS1;
}

void serviceTx(linkInfo_t *link)
{
	if (bytesToSend(link))
		if (link->sfpTx())
			transmitSfpByte(link);
}

/*! TODO
When to call the state machine?
When to block txflag bits?
Add 2nd nps frame and switch every time transmit frame is called. this makes the frame
available more easily without and locks.
Add who byte to the link structure to identify it. initially it is zero for anonymous but
it can be set to uniquely identify it. links can exchange whos to know their partner. when
resetting a link the who is also reset.
Use a table for pid handlers. perhaps generate it along with the pids. use handler names
derived from the pid names. could also provide an interface to the table so that new
handlers could be swapped in.
Table per link?
multi packet service used a single management pid for setup, acknowledge, request, abort, finish
With 64 packet numbers, an octet could be used to acknowledge received packets.

Consider passing link into all routines. This allows them to be called by anyone
at anytime. More like OO.
work to do bits
 service another link
 send an ack
 send a poll
 send an nps
 send an sps
 timeouts

*/

Long spsRetries = 1; // for backing off on resends

bool giveupSpsLink(linkInfo_t *link) // giveup on sps
{
	Byte sps = SPS;

	switch(link->txSps)
	{
		case ANY_SPS:
			link->txSps = ONLY_SPS0; // set to this state for next statement to work
			if (link->enableSps)
				return sendSecurePacketLink(&sps, 1, link);
			break;
		case ONLY_SPS0:
		case ONLY_SPS1:
			break;
		case WAIT_ACK0:
		case WAIT_ACK1:
			SpsFailed(link->stats);
			link->txSps = ANY_SPS;
			if (link->enableSps)
				setTimeout(STARTUPSPS_TIME, &link->giveupTo);  // startup sps service - use giveup timeout
			stopTimeout(&link->resendTo);
			spsRetries = 1;
			break;
	}
	return true;
}

void resendSpsLink(linkInfo_t *link) // resend an sps packet
{
	switch(link->txSps)
	{
		case ANY_SPS:
		case ONLY_SPS0:
		case ONLY_SPS1:
			stopTimeout(&link->resendTo);
			spsRetries = 1;
			break;
		case WAIT_ACK0:
		case WAIT_ACK1:
			setTimeout(spsRetries * SFP_RESEND_TIME, &link->resendTo);
			spsRetries++;
			Resent(link->stats);
			setSpsSend(link);
			break;
	}
}

void sfpTxSm(linkInfo_t *link) //! continue to send a frame or start a new one or just exit if all done
{
	if (checkTimeout(&link->giveupTo))
	{
		stopTimeout(&link->giveupTo);
		stopTimeout(&link->resendTo);
		setGiveup(link);
	}
	else if (checkTimeout(&link->resendTo))
		setResend(link);

	if (workToDo(link))
	{
		if (testAckReceived(link))
			spsAcknowledgedLink(link);

		if (testAckSend(link))
		{
			if (transmitFrame(ackFrame, link))
			{
				SendFrame(link->stats);
				clearAckSend(link);
			}
		}
		else if (testSpsSend(link))
		{
			if (link->enableSps)
			{
				if (transmitFrame(link->frameOutSps, link))
				{
					SpsSent(link->stats);
					clearSpsSend(link);
				}
			}
			else
				clearSpsSend(link);
		}
		else if (testNpsSend(link))
		{
			if (transmitFrame(link->frameOutNps, link))
			{
				SendFrame(link->stats);
				switchNpsFramesLink(link);
				clearNpsSend(link);
			}
		}
		else if (testPollSend(link))
		{
			Byte n = pollTrain;

			if (bytesToReceive(link) > n)
				n = bytesToReceive(link);
			transmitPoll(n,link);
			clearPollSend(link);
		}
		
		if (testGiveup(link))
		{
			if (giveupSpsLink(link))
				clearGiveup(link);
		}
		
		if (testResend(link))
		{
			clearResend(link);
			resendSpsLink(link);
		}
		link->sfpTxState = TRANSMITTING;
	}
	else if (link->sfpTxState == TRANSMITTING)
		link->sfpTxState = IDLING;
}

