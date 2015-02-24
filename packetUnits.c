// Packet Units  Robert Chapman III  Sep 26, 2013

/*
	This interface provides a generic packet facility for use by any task. The
	modicum of exchange is the packet unit. A task requests a packet unit to
	fill with its data and then sends it into the system when done. A packet
	unit is a packet prefixed with a length byte.
	
	If one were very efficient, the PUs would become frames with room to prepend
	and append the frame structure. This would save copying bytes.
	
	If the system is full or empty, the calling task can block on the call or
	return and decide to do something else. 
	
	would be better if first byte was not count but a pointer into the packet. This
	way data could be added using the pointer and the pointer would be updated as used.
	this would avoid calculating the length at the end and sometimes forgetting to. But
	having typed that, the packet is usually accessed through a structure anyway.
*/

#include "sfp.h"
#include "queue.h"
#include "machines.h"
#include "printers.h"

#include "packetUnits.h"

#define MAX_PACKET_UNITS 100 // number of packet units in the system

pu_t puSpace[MAX_PACKET_UNITS];

QUEUE(MAX_PACKET_UNITS, emptyPuq);
QUEUE(MAX_PACKET_UNITS, fullPuq);

// stats
static Long publocked=0, pusent=0, puoverflow=0, epumin=MAX_PACKET_UNITS, fpumax=0,
			toolong=0, tooshort=0, badaddress=0, emptyqoverflow, fullqoverflow;

// declarations
void initPacketUnits(void);
void disasterRecovery(void);
void packetUnitSender(void);
void puRepair(void);
void printPuStats(void);

// copied and simplified from MQX implementation
pu_t *grabPu(puRequest type) // return a pu from emptyq
{
	pu_t *pu;

	while (queryq(emptyPuq) == 0)
	{
		if (type == PU_NON_BLOCK)
			return NULL;
		runMachines();
	}

	pu = (pu_t *)pullq(emptyPuq); // return a pu
	return pu;
}

void returnPu(pu_t *pu)  // didn't use it
{
	if ((pu < &puSpace[0]) || (pu > &puSpace[MAX_PACKET_UNITS-1]))
		badaddress++;
	else
	{
		if (qleft(emptyPuq) == 0)
			emptyqoverflow++;
		else
			pushq((Cell)pu, emptyPuq); // return a pu and release lock
		puRepair(); // 
	}
}

// TODO: this should use a queue from a link so one link doesn't hold up others
bool shipPu(puRequest type, pu_t *pu)
{
	if (pu->length > MAX_PACKET_LENGTH)
	{
		toolong++;
		returnPu(pu);// don't let it go any further or it will plug up the queue
		return true;
	}
	if (pu->length == 0)
	{
		tooshort++;
		returnPu(pu);// don't let it go any further or it will plug up the queue
		return true;
	}
	while (qleft(fullPuq) == 0)
	{
		if (type == PU_NON_BLOCK)
			return false;
		runMachines();
	}

	if (qleft(fullPuq) == 0)
		fullqoverflow++;
	else
		pushq((Cell)pu, fullPuq); // return a pu and release lock

	puRepair();
	return true;
}

void reboot(void);

void disasterRecovery(void)
{
	reboot();
}

// SFP task side
void packetUnitSender(void)
{
	pu_t *pu;
	
	if (queryq(fullPuq))
	{
		Byte to, pid;
		bool flag;
		
		pu = (pu_t *)q(fullPuq);
		to = ((whoPacket_t *)pu->packet)->who.to;
		pid = ((whoPacket_t *)pu->packet)->pid;

		if (pid & SECURE_BIT) // check for service quality bit
			flag = sendSpTo(pu->packet, pu->length, to);
		else
			flag = sendNpTo(pu->packet, pu->length, to);
		if (flag)
		{
			pusent++;
			pu = (pu_t *)pullq(fullPuq);
			returnPu(pu);
		}
		else
			publocked++;
	}
	activate(packetUnitSender);
}

void puqs(Long *e, Long *f);
void puqs(Long *e, Long *f)
{
	*f = queryq(fullPuq);
	*e = queryq(emptyPuq);
}

// TODO: puRepair could, at worst case, take 50 * 50 cycles to execute. This would
// be very expensive. Instead, each PU could be tagged as empty or full. Then
// when it is put into a queue these flags should be checked to see if it is already in
// another queue. At the very least there should be stats counters for the error
// conditions. If all the PUs were kept in a table the table could be used to know
// if any PUs were missing, perhaps.
// Simpler to call puRepair only when things go wrong. There the queues will be either
// full or empty.
// Note to self: repair is only called when both queues have too many.
void puRepair(void) // monitor and only repair if needed
{
	Long e, f;

	safe(puqs(&e,&f));
	if ( e+f > MAX_PACKET_UNITS )
	{
		Long j = f;

		puoverflow++;
		while (j--)
		{
			Long i = e;
			
			while (i--)
			{
				if (q(emptyPuq) == q(fullPuq))
					pullq(emptyPuq);	// remove from empty if in full
				pushq(pullq(emptyPuq), emptyPuq);
			}
			pushq(pullq(fullPuq), fullPuq);
		}
	}
	if (e < epumin)
		epumin = e;
	if (f > fpumax)
		fpumax = f;
}

// inits
void initPacketUnits(void)
{
	int i;

	zeroq(fullPuq);
	zeroq(emptyPuq);
	for (i=0; i<MAX_PACKET_UNITS; i++)
		pushq((Cell)&puSpace[i], emptyPuq);
	activateOnce(packetUnitSender);
}

// CLI
void printPuStats(void)
{
	if (toolong) print("\nError: too longs: "), printDec(toolong);
	if (tooshort) print("\nError: too shorts: "), printDec(tooshort);
	if (badaddress) print("\nError: bad addresses: "), printDec(badaddress);
	if (emptyqoverflow) print("\nError: emptyqoverflow: "), printDec(emptyqoverflow);
	if (fullqoverflow) print("\nError: full q overflow"), printDec(fullqoverflow);
	if (puoverflow) print("\nError: too many pus: "), printDec(puoverflow);
	if (publocked) print("\npu sends blocked: "), printDec(publocked);
	if (pusent) print("\npu sent: "), printDec(pusent);
	if (epumin) print("minimum emptypuq: "), printDec(epumin);
	if (fpumax) print("maximum fullpuq: "), printDec(fpumax);
	if (queryq(emptyPuq)) print("\nemptyq: "), printDec(queryq(emptyPuq));
	if (queryq(fullPuq)) print("\nfullPuq: "), printDec(queryq(fullPuq));
}