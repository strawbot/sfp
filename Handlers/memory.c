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

#include "bktypes.h"
#include "timeout.h"
#include "stats.h"
#include "sfp.h"
#include "link.h"
#include "sfpTxSm.h"
#include "pids.h"

// external
Byte flashWriteMemory(Byte *source, Long destination, Byte length);
Byte flashEraseMemory(Long start, Long end);

void intDisable(void);
void intEnable(void);

// internal
void initFlash(void);
bool flashWrite(Byte *packet, Byte length);
bool checkMem(Byte *packet, Byte length);
bool writeConfirm(Byte *packet, Byte flag);
bool eraseMem(Byte *packet, Byte length);
bool eraseConfirm(Byte *packet, Byte flag);
Long fletcher32(Byte *addr, Long len);
bool executeCode(Byte *packet, Byte);
bool memRead(Byte *packet, Byte);
bool ramWrite(Byte *packet, Byte);

// definitions
void initFlash(void)
{
	setPacketHandler(FLASH_WRITE, flashWrite);
	setPacketHandler(CHECK_MEM, checkMem);
	setPacketHandler(ERASE_MEM, eraseMem);
	setPacketHandler(CALL_CODE, executeCode);
	setPacketHandler(MEM_READ, memRead);
	setPacketHandler(RAM_WRITE, ramWrite);
}

bool writeConfirm(Byte *packet, Byte flag) // confirm operation
{
	Byte confPacket[sizeof(memoryPacket_t)+1];
	memoryPacket_t *cp = (memoryPacket_t *)confPacket;
	memoryPacket_t *mp = (memoryPacket_t *)packet;
	
	cp->pid = WRITE_CONF;
	cp->who.to = mp->who.from;
	cp->who.from = mp->who.to;
	longToBytes(bytesToLong(&mp->addr[0]), &cp->addr[0]);
	cp->length = mp->length;
	cp->data[0] = flag;

	return sendNpTo(confPacket, sizeof(confPacket), cp->who.to);
}

bool flashWrite(Byte *packet, Byte)
{
	memoryPacket_t *mp = (memoryPacket_t *)packet;
	Long a = bytesToLong(&mp->addr[0]);
	
	if (0 == memcmp((const void*)mp->data, (const void*)a, mp->length)) // see if already programmed
		return writeConfirm(packet, 0);
	return writeConfirm(packet, flashWriteMemory(mp->data, a, mp->length));
}

bool eraseConfirm(Byte *packet, Byte flag) // confirm operation
{
	Byte confPacket[sizeof(longsPacket_t)+3*sizeof(long)];
	longsPacket_t *cp = (longsPacket_t *)confPacket;
	longsPacket_t *mp = (longsPacket_t *)packet;
	
	cp->pid = ERASE_CONF;
	cp->who.to = mp->who.from;
	cp->who.from = mp->who.to;
	longToBytes(bytesToLong(&mp->longs[0].data[0]), &cp->longs[0].data[0]);
	longToBytes(bytesToLong(&mp->longs[1].data[0]), &cp->longs[1].data[0]);
	longToBytes((Long)flag, &cp->longs[2].data[0]);

	return sendNpTo(confPacket, sizeof(confPacket), cp->who.to);
}

bool eraseMem(Byte *packet, Byte)
{
	longsPacket_t *lp = (longsPacket_t *)packet;
	Long start = bytesToLong(&lp->longs[0].data[0]);
	Long end = bytesToLong(&lp->longs[1].data[0]);

	return eraseConfirm(packet, flashEraseMemory(start, end));
}

bool checkMem(Byte *packet, Byte) // check memory contents
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
}

bool executeCode(Byte *packet, Byte) // execute code
{
	longsPacket_t *mp = (longsPacket_t *)packet;
	
	intDisable();
	((vector)bytesToLong(&mp->longs[0].data[0]))();
	intEnable();
	return true;
}

bool memRead(Byte *packet, Byte) // read memory
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
}

bool ramWrite(Byte *packet, Byte) // write memory
{
	memoryPacket_t *mp = (memoryPacket_t *)packet;
	Byte *address = (Byte *)bytesToLong(&mp->addr[0]);
	
	memcpy(address, mp->data, mp->length);
	return writeConfirm(packet, 0);
}

// command line
#include "botkernl.h"

void writeFlashCmd(void);
void writeFlashCmd(void) // ( source dest length - result )
{
	Byte length = (Byte)*sp++;
	Long dest = *sp++;
	Byte *source = (Byte *)*sp++;
	
	*--sp = (Long)flashWriteMemory(source, dest, length);
}

void eraseFlashCmd(void);
void eraseFlashCmd(void) // ( start end - result )
{
	Long end = *sp++, start = *sp;
	
	*sp = (Long)flashEraseMemory(start, end);
}