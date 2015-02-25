// SFP interface  Robert Chapman III  Feb 13, 2015

#include "timbre.h"
#include "parameters.h"
#include "pids.h"

#ifndef _SFP_H_
#define _SFP_H_

// defines - lengths in bytes
#define MIN_FRAME_LENGTH 	(SYNC_LENGTH + PID_LENGTH + CHECKSUM_LENGTH)
#define MAX_SFP_SIZE		(LENGTH_LENGTH + MAX_FRAME_LENGTH)
#define MIN_SFP_SIZE 		(LENGTH_LENGTH + MIN_FRAME_LENGTH)
#define MAX_PACKET_LENGTH	(MAX_FRAME_LENGTH - MIN_FRAME_LENGTH)
#define MAX_PAYLOAD_LENGTH  (MAX_PACKET_LENGTH - PID_LENGTH)
#define FRAME_OVERHEAD		(MIN_FRAME_LENGTH - PID_LENGTH)
#define FRAME_HEADER        (LENGTH_LENGTH + SYNC_LENGTH)
#define PACKET_HEADER       (PID_LENGTH)

#define LENGTH_LENGTH 	1
#define SYNC_LENGTH		1
#define PID_LENGTH		1
#define CHECKSUM_LENGTH 2

// one's complement doesn't create a zero while two's complement does
// one's complement should be better for checksum coverage
#define sfpSync(length)	((Byte)~(length))	// define sync mechanism

// Generic frame containter
typedef struct {
    Byte sum;
    Byte sumsum;
} checkSum_t;

typedef struct {
	Byte to;
	Byte from;
} who_t;

typedef struct { // ''
	Byte length; // covers following bytes
	Byte sync;	// two's complement of length used to sync start of frame
	union {
		struct {
			Byte pid;  // packet id upper two bits are for ack and sps
            union {
            Byte payload[MAX_PAYLOAD_LENGTH];
            who_t who;	// first part of payload if present
            };
        };
        Byte packet[MAX_PAYLOAD_LENGTH];
    };
} sfpFrame;

#endif
