// Flash write interface  Robert Chapman III  Jun 22, 2012

/*
	Uses processor expert component to take care of the internals and an external
	file for the flash specifics. This file handles the protocol aspect.

	This provides the driver code for the packet IDs:
	CHECK_MEM	++		check memory contents; who, addr32, len32, method8
	MEM_CHECK	++		value returned from checking memory contents; who, addr32, len32, method8, code

	FLASH_WRITE	++		request write to flash; who, 32bit addr, 8bit length, bytes
	WRITE_CONF	++		confirm a write to memory; who, addr32, len8

	ERASE_MEM	++		erase memory range froms start to end
	ERASE_CONF	++		confirm memory erased (0) or not (!0)

	CALL_CODE	++		call code starting at this location; who, addr32

memory packet:
typedef struct { // 32bit addr, 8bit length, bytes // 'BBPB0B'
	Byte pid;
	who_t who;
	Byte addr[sizeof(void *)];
	Byte length;
	Byte data[];
} memoryPacket_t;

general longs packet:
typedef struct { // 32bit data bytes // 'BBB0L'
	Byte pid;
	who_t who;
	long_t longs[];
} longsPacket_t;

TODO:
-	FILL_MEM	++		fill memory chunks with pattern of length bytes; who, addr32, len32, len8, pattern

*/

#include <string.h>

#include "timeout.h"
#include "stats.h"
#include "sfp.h"
#include "link.h"
#include "sfpTxSm.h"
#include "pids.h"
#include "flash.h"
#include "services.h"
#include "checksum.h"
#include "valueTransfer.h"

// definitions
/* writing flash
	green paths:
	1. flash was written (and verified) - all good
	2. flash was already programmed - all good
	red paths:
	1. flash write fails
	2. flash write ok but mem compare fails (assume flash write returns error)
   States:
    1. received packet
    2. writing
    3. replying
*/
static memoryPacket_t wconfirm;

static void writeReply(void)
{
	if (false == sendNpTo((Byte *)&wconfirm, sizeof(wconfirm), wconfirm.who.to))
		activate(writeReply);
}

static void writeConfirm(void)
{
	if (flashBusy())
		activate(writeConfirm);
	else {
		wconfirm.data[0] = flashError();
		writeReply();
	}
}

static bool flashWrite(Byte *packet, Byte length)
{
	memoryPacket_t *mp = (memoryPacket_t *)packet;
	Long a = bytesToLong(&mp->addr[0]);
	
	wconfirm.pid = WRITE_CONF;
	wconfirm.who.from = mp->who.to;
	wconfirm.who.to = mp->who.from;
	longToBytes(a, wconfirm.addr);
	wconfirm.length = mp->length;
	
	if (0 == memcmp(mp->data, (Byte *)a, mp->length)) { // see if already programmed ?bury in flashWriteMemory
		wconfirm.data[0] = 0; // indicate no errors
		writeReply();
	}
	else {
		flashWriteMemory((Long)mp->data, a, (Long)mp->length);
		writeConfirm();
	}

	return true;
}
	
static erasePacket_t econfirm;

static void eraseReply(void)
{
	if (false == sendNpTo((Byte *)&econfirm, sizeof(econfirm), econfirm.who.to))
		activate(eraseReply);
}

static void eraseConfirm(void)
{
	if (flashBusy())
		activate(eraseConfirm);
	else {
		econfirm.flag = flashError();
		eraseReply();
	}
}

static bool eraseMem(Byte *packet, Byte length)
{
	erasePacket_t *ep = (erasePacket_t *)packet;
	Long start = bytesToLong((Byte *)&ep->start);
	Long end = bytesToLong((Byte *)&ep->end);
	
	econfirm.pid = ERASE_CONF;
	econfirm.who.from = ep->who.to;
	econfirm.who.to = ep->who.from;
	longToBytes(start, (Byte *)&econfirm.start);
	longToBytes(end, (Byte *)&econfirm.start);
	
	flashEraseMemory(start, end);
	eraseConfirm();

	return true;
}
	
static bool checkMem(Byte *packet, Byte length) // check memory contents
{
	longsPacket_t *mp = (longsPacket_t *)packet;
	Byte checkPacket[sizeof(longsPacket_t) + 3 * sizeof(long)];
	longsPacket_t *rp = (longsPacket_t *)checkPacket;
	Long checksum, len;
	Byte *addr;

	addr = (Byte *)bytesToLong(&mp->longs[0].data[0]);
	len = bytesToLong(&mp->longs[1].data[0]);
	checksum = fletcher32(addr, len);

	rp->pid = MEM_CHECK;
	rp->who.to = mp->who.from;
	rp->who.from = mp->who.to;
	longToBytes((Long)addr, &rp->longs[0].data[0]);
	longToBytes(len, &rp->longs[1].data[0]);
	longToBytes(checksum, &rp->longs[2].data[0]);

	return sendNpTo(checkPacket, sizeof(checkPacket), rp->who.to);
	(void) length;
}

static bool memRead(Byte *packet, Byte length) // read memory
{
	Byte dataPacket[MAX_PACKET_LENGTH];
	memoryPacket_t *dp = (memoryPacket_t *)dataPacket;
	memoryPacket_t *mp = (memoryPacket_t *)packet;
	Long address = bytesToLong(mp->addr);
	
	dp->pid = MEM_DATA;
	dp->who.to = mp->who.from;
	dp->who.from = mp->who.to;
	longToBytes(address, dp->addr);
	dp->length = mp->length;
	memcpy(dp->data, (Byte *)address, mp->length);
	
	return sendNpTo(dataPacket, sizeof(memoryPacket_t) + mp->length, dp->who.to);
	(void)length;
}

#include <libarm.h>
//
//#define intDisable()  libarm_disable_irq_fiq() 	// Disable Interrupts
//#define intEnable()  libarm_enable_irq_fiq() 	// Enable Interrupts

static bool executeCode(Byte *packet, Byte length) // execute code
{
	longsPacket_t *mp = (longsPacket_t *)packet;
	vector function = (vector)bytesToLong(&mp->longs[0].data[0]);
	
	//libarm_disable_irq_fiq();
	function();
	//libarm_enable_irq_fiq();
	return true;
	(void) length;
}

static bool ramWrite(Byte *packet, Byte length) // write memory
{
	memoryPacket_t *mp = (memoryPacket_t *)packet;
	Byte *address = (Byte *)bytesToLong(&mp->addr[0]);
	
	memcpy(address, mp->data, mp->length);
	wconfirm.pid = WRITE_CONF;
	wconfirm.who.from = mp->who.to;
	wconfirm.who.to = mp->who.from;
	longToBytes((Long)address, wconfirm.addr);
	wconfirm.length = mp->length;
	wconfirm.data[0] = 0; // indicate no errors
	writeReply();
	return true;
	(void) length;
}

void initFlash(void)
{
	setPacketHandler(FLASH_WRITE, flashWrite);
	setPacketHandler(CHECK_MEM, checkMem);
	setPacketHandler(ERASE_MEM, eraseMem);
	setPacketHandler(CALL_CODE, executeCode);
	setPacketHandler(MEM_READ, memRead);
	setPacketHandler(RAM_WRITE, ramWrite);
}

