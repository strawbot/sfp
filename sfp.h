// SFP interface  Robert Chapman III  Feb 13, 2015

#include "timbre.h"
#include "pids.h"

#ifndef _SFP_H_
#define _SFP_H_

// defines - lengths in bytes
#define MIN_FRAME_LENGTH 	(SYNC_LENGTH + PID_LENGTH + CHECKSUM_LENGTH)
#define MAX_SFP_SIZE		(LENGTH_LENGTH + MAX_FRAME_LENGTH)
#define MIN_SFP_SIZE 		(LENGTH_LENGTH + MIN_FRAME_LENGTH)
#define MAX_PACKET_LENGTH	(MAX_FRAME_LENGTH - MIN_FRAME_LENGTH)
#define MAX_PAYLOAD_LENGTH  (MAX_PACKET_LENGTH - PID_LENGTH)
#define WHO_LENGTH			sizeof(who_t)
#define WHO_HEADER_SIZE		(PID_LENGTH + WHO_LENGTH)
#define SPID_HEADER_SIZE		(WHO_HEADER_SIZE + 1)
#define MAX_WHO_PAYLOAD_LENGTH  (MAX_PACKET_LENGTH - WHO_HEADER_SIZE)
#define MAX_SPID_PAYLOAD_LENGTH  (MAX_WHO_PAYLOAD_LENGTH - SPID_LENGTH)
#define FRAME_OVERHEAD		(MIN_FRAME_LENGTH - PID_LENGTH)
#define FRAME_HEADER        (LENGTH_LENGTH + SYNC_LENGTH)
#define PACKET_HEADER       (PID_LENGTH)

#define LENGTH_LENGTH 	1
#define SYNC_LENGTH		1
#define PID_LENGTH		1
#define SPID_LENGTH		1
#define CHECKSUM_LENGTH 2

// one's complement doesn't create a zero while two's complement does
// one's complement should be better for checksum coverage
#define sfpSync(length)	((Byte)(~(length)))	// define sync mechanism

// for routing when part of packet
typedef struct {
    Byte to;
    Byte from;
} who_t;

// Generic frame containter
#define packet_union \
union { \
	struct { \
		Byte pid;  /* packet id upper two bits are for ack and sps */ \
		union { /* for packet with and without routing header who_t */ \
			Byte payload[MAX_PAYLOAD_LENGTH]; \
			struct { \
				who_t who;	/* routing ids */ \
				union { \
					Byte whoload[MAX_WHO_PAYLOAD_LENGTH]; \
					struct { \
						Byte spid; \
						Byte spidload[MAX_SPID_PAYLOAD_LENGTH]; \
					}; \
				}; \
			}; \
		}; \
	}; \
	Byte packet[MAX_PAYLOAD_LENGTH]; \
}

//#pragma anon_unions

typedef packet_union packet_t;

typedef struct sfpFrame { // ''
	struct sfpFrame * list; // used to link frames in a list
	Long timestamp; // added to frame when received
	union {
	    Byte content[1];
        struct {
            Byte length; // covers following bytes
            Byte sync;	// two's complement of length used to sync start of frame
            packet_union;
        };
	};
} sfpFrame;

// trailer
typedef struct {
    Byte sum;
    Byte sumsum;
} checkSum_t;

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

// Packet types
typedef struct { // 'BBB0B'
	Byte pid;
	who_t who;
	Byte payload[];
} whoPacket_t;

typedef struct { // 'BBBB0B'
	Byte pid;
	who_t who;
	Byte spid;
	Byte payload[];
} spidPacket_t;

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

typedef struct { // 32bit addr, 8bit length, bytes // 'BBPB1B'
	Byte pid;
	who_t who;
	Byte addr[sizeof(void *)];
	Byte length;
	Byte data[1];
} memoryPacket_t;

typedef struct { // 32bit data bytes // 'BBBLLB'
	Byte pid;
	who_t who;
	long_t start;
	long_t end;
	Byte flag;
} erasePacket_t;

typedef struct { // 32bit data bytes // 'BBB0L'
	Byte pid;
	who_t who;
	long_t longs[];
} longsPacket_t;

#endif

// define for system
void initSfp(void);
