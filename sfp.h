#include "bktypes.h"
#include "pids.h"
#include "who.h"
#include "valueTransfer.h"

#define SECURE_BIT	ACK_BIT //	same as ACK_BIT but more appropriate name - used to tag packets

extern Byte packetError; // result of packet error

#ifndef SFP_H
#define SFP_H

enum {PACKET_OK, PACKET_SIZE_BAD, PACKET_LINK_BUSY};

#define MAX_PACKET_LENGTH 100

// basic sizes
typedef struct {
	Byte data[sizeof(long long)];
} octet_t;

typedef struct {
	Byte data[sizeof(long)];
} long_t;

typedef struct {
	Byte data[sizeof(short)];
} short_t;

// packet structures
typedef struct { // 'B0B'
	Byte pid;
	Byte payload[];
} Packet_t;

typedef struct {
	Byte to;
	Byte from;
} who_t;

typedef struct {
	Byte s;
	Byte r;
} tid_t;

typedef struct { // 'BBB0B'
	Byte pid;
	who_t who;
	Byte payload[];
} whoPacket_t;

typedef struct { // for modbus, guids
	Byte pid;
	who_t who;
	tid_t tid;
	Byte spid;
	Byte payload[];
} spidPacket_t;

typedef struct { // like spid packet but without tid
	Byte pid;
	who_t who;
	Byte spid;
	Byte payload[];
} smallSpidPacket_t;

typedef struct { // 'BBB0B'
	Byte pid;
	who_t who;
	Byte payload[];
} evaluatePacket_t;

// Application packet types
typedef struct { // 32bit addr, 8bit length, bytes // 'BBBPB0B'
	Byte pid;
	who_t who;
	Byte addr[sizeof(void *)];
	Byte length;
	Byte data[];
} memoryPacket_t;

typedef struct { // 32bit data bytes // 'BBB0L'
	Byte pid;
	who_t who;
	long_t longs[];
} longsPacket_t;

typedef struct { // guid value pair
	long_t guid;
	short_t value;
} gv_t;

typedef struct { // guid groups
	Byte pid;
	who_t who;
	gv_t gvpairs[];
} gvPacket_t;

typedef struct { // possible guid value pairs with a short guid
	short_t guid;
	Byte data[];	// could be 1, 2, 4, 8 bytes
} sgv_t;

typedef struct { // events for display storage
	Byte pid;
	who_t who;
	Byte spid;
	Byte eventnumber;
	octet_t eventtime; // milliseconds since jan 1, 2000 UTC
    Byte payload[];
} eventPacket_t;

#define EVENT_ENO_MASK 0xFF // for wrapping eno

typedef struct { // Python format: 'B0B'
	Byte pid;
	who_t who;
	Byte long_frame[4];
	Byte short_frame[4];
	Byte tossed[4];
	Byte good_frame[4];
	Byte bad_checksum[4];
	Byte timeouts[4];
	Byte resends[4];
	Byte rx_overflow[4];
	Byte sent_frames[4];
	Byte unknown_packets[4];
	Byte unrouted[4];
} statsPacket;

typedef struct {
	Byte pid;
	who_t who;
	tid_t tid;
	Byte spid;
	Byte payload[];
} filePacket_t;

typedef struct {
	Byte pid;
	who_t who;
	Byte major;
	Byte minor;
	short_t build;
	Byte dateLength;
	Byte date[20];
	Byte nameLength;
	Byte name[];
} versionPacket_t;

typedef struct { // version numbers for all the firmware in the box and which to run
	Byte pid;
	who_t who;
	Byte spid;

	Byte runningChoice; // actual set by boot
	Byte pendingChoice; // preference set by launcher or display

	long_t	mainboot;
	long_t	database;

	long_t packageLeft;
	long_t launcherLeft;
	long_t mainappLeft;
	long_t ubootLeft;
	long_t linuxLeft;
	long_t displayappLeft;
	long_t ioappLeft;
	long_t swbappLeft;

	long_t packageRight;
	long_t launcherRight;
	long_t mainappRight;
	long_t ubootRight;
	long_t linuxRight;
	long_t displayappRight;
	long_t ioappRight;
	long_t swbappRight;

	long_t slotaType;
	long_t slotbType;
} firmwarePacket_t;

#define PACKET_OVERHEAD 1 		// pid
#define WHO_PACKET_OVERHEAD (sizeof(whoPacket_t))   // pid, who
#define SPID_PACKET_OVERHEAD (sizeof(spidPacket_t))   // pid, who, tid, spid
#define EVALUATE_PACKET_OVERHEAD (WHO_PACKET_OVERHEAD) // pid, who
#define MAX_PIDS (PID_BITS) // 64
#define MAX_WHO_PACKET_PAYLOAD (MAX_PACKET_LENGTH - WHO_PACKET_OVERHEAD)
#define MAX_SPID_PACKET_PAYLOAD (MAX_PACKET_LENGTH - SPID_PACKET_OVERHEAD)

// vectorizable packet handlers
// of the form:
// bool ph(Byte *packet, Byte length);
typedef bool (*packetHandler_t)(Byte *packet, Byte length);

#endif

void setPacketHandler(Byte pid, packetHandler_t handler);
packetHandler_t getPacketHandler(Byte pid);
bool sendNpTo(Byte *packet, Byte length, Byte to);
bool sendSpTo(Byte *packet, Byte length, Byte to);
