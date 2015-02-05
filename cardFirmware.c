// firmware handler  Robert Chapman III  Sep 30, 2013

#include "memorymaps.h" // use Card types
#include "sfp.h"		// use longsPacket_t for info
#include "version.h"
#include "copyRun.h"
#include "firmwareHandler.h"

Byte whatAmI(void);

Byte addHeaderVersion(Long address, Byte * fwp);
Byte addDateVersion(Long address, Byte * fwp);

void cardFirmwareRequest(Byte to)
{
	Byte response[MAX_PACKET_LENGTH];
	smallSpidPacket_t *sp = (smallSpidPacket_t *)response;
	Byte length;

	sp->pid = FIRMWARE;
	sp->who.to = to;
	sp->who.from = whoami();
	sp->spid = CARD_FW_RESP;
	sp->payload[0] = whatAmI();
	length = 1;
	switch(whatAmI())
	{
		case SWB_CARD:
			length += addDateVersion(SLOT_BOOT, &sp->payload[length]);
			length += addHeaderVersion(SWB_APP, &sp->payload[length]);
			break;
		case IO_CARD:
			length += addDateVersion(SLOT_BOOT, &sp->payload[length]);
			length += addHeaderVersion(IO_APP, &sp->payload[length]);
			length += addHeaderVersion(IO_HIBOOT, &sp->payload[length]);
			break;
		case MAIN_CARD:
			length += addDateVersion(MAIN_BOOT, &sp->payload[length]);
			length += addHeaderVersion(UBOOT_LEFT, &sp->payload[length]);
			length += addHeaderVersion(UBOOT_RIGHT, &sp->payload[length]);
			length += addHeaderVersion(LAUNCHER_LEFT, &sp->payload[length]);
			length += addHeaderVersion(LAUNCHER_RIGHT, &sp->payload[length]);
			length += addHeaderVersion(MAIN_APP_LEFT, &sp->payload[length]);
			length += addHeaderVersion(MAIN_APP_RIGHT, &sp->payload[length]);
			length += addHeaderVersion(IO_APP_LEFT, &sp->payload[length]);
			length += addHeaderVersion(IO_APP_RIGHT, &sp->payload[length]);
			length += addHeaderVersion(SWB_APP_LEFT, &sp->payload[length]);
			length += addHeaderVersion(SWB_APP_RIGHT, &sp->payload[length]);
			break;
	}
	
	length += sizeof(smallSpidPacket_t);
	sendNpTo(response, length, sp->who.to);
}

Byte addDateVersion(Long address, Byte * fwp)
{
	Byte length = 0;
	Long mmb = 0;
	
	longToBytes(address, fwp), length += sizeof(Long);
	
	mmb = extractVersion(address+VERSION_OFFSET);

	longToBytes(mmb, &fwp[length]), length += sizeof(Long);
	
	return length;
}

Byte addHeaderVersion(Long address, Byte * fwp)
{
	Byte length = 0;
	Long mmb = 0;
	
	longToBytes(address, fwp), length += sizeof(Long);
	
	if (checkHeader(address) == CHECK_OK)
		mmb = ((imageHead_t *)(address))->mmb.version;

	longToBytes(mmb, &fwp[length]), length += sizeof(Long);
	
	return length;
}
