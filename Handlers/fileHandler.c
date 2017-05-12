// File transfer over SFP  Robert Chapman III  May 10, 2017

#include "services.h"
#include "transferSpids.h"
#include "printers.h"
#include "fileTransfer.h"
#include "valueTransfer.h"

typedef struct {
	Byte pid;
	who_t who;
	Byte spid;
	long_t address;
	Byte data[];
} dataPacket_t;

typedef struct {
	Byte pid;
	who_t who;
	Byte spid;
	long_t size;
	Byte type;
	char name[];
} requestPacket_t;

/*
Downloads:
 - initiated from host end
 
  Host         Target          Parameters
   Request----->               size, name, type
      <--------Response
   Data-------->               address, data
      <--------OK
         ...
   Done-------->               checksum
      <--------Complete

Uploads:
 - initiated from host end
 
  Host         Target          Parameters
   File-------->               name
      <--------Data            address, data
   OK---------->
         ...
      <--------Done            checksum
   Complete---->
*/

/* Application API
   initTransfer() - needs to be called by App before use

  Implemented in App:
   bool openUpload(char * name);
   int  readUpload(int n);
   void closeUpload(void);
   
   bool openDownload(char * name, int size);
   bool writeDownload(Byte * data, int length);
   void closeDownload(void);
*/
// uploads
#define DATA_SIZE (MAX_PACKET_LENGTH - sizeof(dataPacket_t))
static Long address;

void transferFinish(void) {
    Byte packet[] = {FILES, DIRECT, DIRECT, TRANSFER_DONE, 0, 0, 0, 0};

    print(" SL: Upload finished");
    sendNpTo(packet, sizeof(packet), DIRECT);
}

void transferChunk(void) { // spid (1), address (4), size (4), data
    int n;
    Byte packet[MAX_PACKET_LENGTH] = {FILES, DIRECT, DIRECT, TRANSFER_DATA};
    dataPacket_t * dp = (dataPacket_t *)packet;
    
    longToBytes(address, dp->address.data);
    n = readUpload(dp->data, DATA_SIZE);
    print(" read file "), printHex((Cell)dp->data), printDec(n);

    if (n) {
        address += n;
        sendNpTo(packet, sizeof(dataPacket_t) + n, DIRECT);
    } else
        transferFinish();
}

void transferFile(char * name) {
    print(" SL: Request to transfer a file named: '"), print(name), print("'");
    address = 0;
    if (openUpload(name))
        transferChunk();
    else {
        Byte data[] = {FILES, DIRECT, DIRECT, FILE_UNAVAILABLE};

        sendNpTo(data, sizeof(data), DIRECT);
    }
}

// download
// pid, to, from, spid, size(4), type, name0
void transferRequest(Byte * inpacket) {
    requestPacket_t * rp = (requestPacket_t *)inpacket;
    Byte packet[] = {FILES, DIRECT, DIRECT, TRANSFER_REPLY, REQUEST_OK};
    
    print(" SL: Transfer request");
    if (!openDownload(rp->name, bytesToLong(rp->size.data)))
        packet[4] = REQUEST_DENIED;
    sendNpTo(packet, sizeof(packet), DIRECT);
}

void transferData(Byte * inpacket, Byte length) {
    dataPacket_t * dp = (dataPacket_t *)inpacket;
    Byte packet[] = {FILES, DIRECT, DIRECT, TRANSFER_RESULT, TRANSFER_OK};
    Byte size = length - sizeof(dataPacket_t);
    
    print(" SL: Transfer data");
    if (!writeDownload(dp->data, size))
        packet[4] = REQUEST_DENIED;
    sendNpTo(packet, sizeof(packet), DIRECT);
}

void transferDone(void) {
    Byte packet[] = {FILES, DIRECT, DIRECT, TRANSFER_COMPLETE};

    print(" SL: Transfer complete");
    closeDownload();
    sendNpTo(packet, sizeof(packet), DIRECT);
}

// pid who spid ... spidPacket_t
bool fileHandler (Byte *packet, Byte length) {
    spidPacket_t * sp = (spidPacket_t *)packet;

    switch (sp->spid) {
    // downloads
	case TRANSFER_REQUEST:
	    transferRequest(packet);
		break;
	case TRANSFER_DATA:
	    print(" SL: Got data");
	    transferData(packet, length);
		break;
	case TRANSFER_DONE:
	    transferDone();
		break;

    // uploads
	case TRANSFER_FILE:
	    transferFile((char *)sp->payload);
		break;
	case TRANSFER_RESULT:
	    transferChunk();
		break;
	case TRANSFER_COMPLETE:
	    print(" SL: unhandled spid TRANSFER_COMPLETE");
		break;
	case TRANSFER_ABORT:
	    print(" SL: unhandled spid TRANSFER_ABORT");
		break;

	default:
	    print(" SL: unknown spid "), printDec(sp->spid);
	    break;
	}
    return true;
}

void initTransfer(void) {
    setPacketHandler(FILES, fileHandler);
}
