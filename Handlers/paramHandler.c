// Parameter setting and querying  Robert Chapman III  Aug 31, 2012

#include "bktypes.h"
#include "timeout.h"
#include "pids.h"
#include "stats.h"
#include "link.h"
#include "sfp.h"

// external
void setParam(Long tag, Long value);
Long getParam(Long tag);

bool setParamHandler(Byte *packet, Byte length); // set parameters
bool getParamHandler(Byte *packet, Byte length); // get parameters
void initParamHandler(void);

bool setParamHandler(Byte *packet, Byte length) // set parameters
{
	longsPacket_t *lp;
	long_t *l, *end;
	
	lp = (longsPacket_t *)packet;
	l = &lp->longs[0];
	end = (long_t *)&packet[length];
	
	while(l < end)
	{
		Long tag, value;
		
		tag = bytesToLong(&l->data[0]);
		l++;
		value = bytesToLong(&l->data[0]);
		l++;
		setParam(tag, value);
	}
	return true;
}

bool getParamHandler(Byte *packet, Byte length) // get parameters
{
	Byte reply[MAX_PACKET_LENGTH];
	longsPacket_t *rlp, *lp;
	long_t *l, *end, *rl, *rend;
	
	rlp = (longsPacket_t *)reply;
	lp = (longsPacket_t *)packet;

	// set up packet header
	rlp->pid = PARAM;
	rlp->who.from = lp->who.to;
	rlp->who.to = lp->who.from;
	
	// set up for packet parsing
	l = &lp->longs[0];
	end = (long_t *)&packet[length];
	
	// set up for packet filling
	rl = &rlp->longs[0];
	rend = (long_t *)&reply[MAX_PACKET_LENGTH - 2*sizeof(long_t)];
	
	while(l < end)
	{
		Long tag, value;
		
		tag = bytesToLong(&l->data[0]);
		l++;
		value = getParam(tag);

		longToBytes(tag, &rl->data[0]);
		rl++;
		longToBytes(value, &rl->data[0]);
		rl++;

		if (rl > rend) // send packet if full; TODO: refactor like guidHandler into queues
		{
			length = (Byte)((Byte *)rl - &reply[0]);
			while (!sendNpTo(reply, length, rlp->who.to))
				runMachines(); // TODO: if spi is not running, then this will hang here
			rl = &rlp->longs[0]; // reset to beginning
		}
	}
	// send packet if not empty
	length = (Byte)((Byte *)rl - &reply[0]);
	if (length > WHO_PACKET_OVERHEAD)
		while (!sendNpTo(reply, length, rlp->who.to))
			runMachines(); // TODO: if spi is not running, then this will hang here
	return true;
}

void initParamHandler(void)
{
	setPacketHandler(SET_PARAM, setParamHandler);
	setPacketHandler(GET_PARAM, getParamHandler);
}