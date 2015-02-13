#include <stdlib.h>

// Packet Processor  Robert Chapman III  Feb 16, 2012
#include "printers.h"
#include "bktypes.h"
#include "timeout.h"
#include "machines.h"
#include "sfpStats.h"
#include "sfpLink.h"
#include "sfpTxSm.h"
#include "sfpStats.h"
#include "pids.h"
#include "routing.h"
#include "who.h"
#include "packets.h"


// vectorizable packet handlers
static packetHandler_t packetHandlers[MAX_PIDS + 1];

void setPacketHandler(Byte pid, packetHandler_t handler)
{
	packetHandlers[pid] = handler;
}

packetHandler_t getPacketHandler(Byte pid)
{
	return packetHandlers[pid];
}

// default packet handlers
bool processPacket(Byte *packet, Byte length, linkInfo_t *link)
{
	whoPacket_t *p = (whoPacket_t *)packet;
	packetHandler_t handler;

	if (p->pid > WHO_PIDS) // check for routing of packet
		if (p->who.to) // is there a destination?
			if (p->who.to != whoami()) // is it not for me?
			{
				linkInfo_t *linkTo = routeTo(p->who.to);
				
				if (whoami() == ME) // do i need an identity? (multi drop)
					setMe(p->who.to); // become the destination
				else
				{
					if (linkTo) // if we can reroute it
						return reRoutePacket(packet, length, linkTo); // send it on
					UnRouted(link->stats); // no place to go, dump it
					return true;
				}
			}

	// give first crack at packet to vectored handlers
	handler = getPacketHandler(p->pid);
	if (handler != NULL)
		return handler(&p->pid, length);

	switch(p->pid) // intercept link only packets - no destination id
	{
		case SPS: // null packet used for initializing SPS
			break;
		case SPS_ACK: // ack frame for SPS
			setAckReceived(link); // pass notice to transmitter
			break;
		case WHO_ARE_YOU:
			{
				Byte reply[2];
				
				reply[0] = MY_ID;
				reply[1] = whoami();
			
				return sendNormalPacketLink(reply, 2, link);
			}
			break;
		case MY_ID: // should not get this
			break;
		case SET_ID: // set the cpu id
			setMe(packet[1]);
			break;
		case PING: // other end is querying so reply
			{	
				Byte ping[] = {PING_BACK};
				
				sendNormalPacketLink(ping, sizeof(ping), link);
			}
			break;
		case PING_BACK: // ignore reply
			break;
		case NWK_LOAD_FR:
		case TEST_FRAME: // ignore test frames
			break;
		default:
//			print("no packet handler for pid:0x"), printHex(p->pid);
//			print("to: "), print(routeName(p->who.to));
//			print("  from: "), print(routeName(p->who.from)), print("\n");
			UnknownPacket(link->stats);
			break;
	}
	return true;
}

// Packet services
Byte packetError = 0; // result of packet error

bool sendNormalPacketLink(Byte *packet, Byte length, linkInfo_t *link) //! send a packet using NPS
{
	bool result;

	if (link == 0) { BadLink(); return true; }

	if ( (length > MAX_PACKET_LENGTH) || (length == 0) )
	{
		packetError = PACKET_SIZE_BAD;
		return false;
	}
	
	requestNps(result, link);
	if (result == false)
	{
		packetError = PACKET_LINK_BUSY;
		return false;
	}
	
	buildSfpFrame(length-1, &packet[1], packet[0], link->frameOutNps);
	
	return true;
}

bool sendSecurePacketLink(Byte *packet, Byte length, linkInfo_t *link) //! send a packet using SPS
{
	Byte sps = 0, pid;
	bool result;

	if (link == 0) { BadLink(); return true; }

	if ( (length > MAX_PACKET_LENGTH) || (length == 0) )
	{
		packetError = PACKET_SIZE_BAD;
		return false;
	}

	requestSps(result, link); // set bit and get previous setting to see if we are the setters
	if (result == false) // already sending
	{
		packetError = PACKET_LINK_BUSY;
		return false;
	}
	
	if (link->txSps == ONLY_SPS1) // must be in an ONLY_SPSn state to be able to send
	{
		sps = SPS_BIT;
		link->txSps = WAIT_ACK1;
	}
	else if (link->txSps == ONLY_SPS0)
		link->txSps = WAIT_ACK0;
	else
	{
		clearSpsSend(link); // give up the bit
		return false;
	}
	
	pid = ACK_BIT|sps|packet[0];
	buildSfpFrame(length-1, &packet[1], pid, link->frameOutSps);
	setTimeout(SFP_RESEND_TIME, &link->resendTo);
	setTimeout(SFP_GIVEUP_TIME, &link->giveupTo);

	return true;
}

bool sendPidLink(Byte pid, linkInfo_t *link) // send a pid
{
	if (link == 0) { BadLink(); return true; }

	return sendNormalPacketLink(&pid, 1, link);
}

// services using routing tables
Long noDest;

bool sendNpTo(Byte *packet, Byte length, Byte to) //! send a packet using NPS
{
	linkInfo_t *link = routeToLink(to);

	if (link)
		return sendNormalPacketLink(packet, length, link);
	else
	{
		noDest++;
		return true; // TODO: deal with packet. If for me - accept it? if for noone - report it.
	}
}

bool sendSpTo(Byte *packet, Byte length, Byte to) //! send a packet using SPS
{
	linkInfo_t *link = routeToLink(to);

	if (link)
		return sendSecurePacketLink(packet, length, link);
	else
	{
		noDest++;
		return true; // TODO: deal with packet. If for me - accept it? if for noone - report it.
	}
}

bool sendPidTo(Byte pid, Byte to) // send a pid
{
	linkInfo_t *link = routeToLink(to);

	if (link)
		return sendNormalPacketLink(&pid, 1, link);
	else
	{
		noDest++;
		return true; // TODO: deal with packet. If for me - accept it? if for noone - report it.
	}
}
