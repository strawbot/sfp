// SFP interface  Robert Chapman III  Feb 13, 2015

#include "timbre.h"
#include "parameters.h"
#include "pids.h"

#ifndef _SFP_H_
#define _SFP_H_

// defines - lengths in bytes
#define MIN_FRAME_LENGTH 	(SYNC_LENGTH + PID_LENGTH + CHECKSUM_LENGTH)
#define MAX_SFP_FRAME		(LENGTH_LENGTH + MAX_FRAME_LENGTH)
#define MIN_SFP_FRAME 		(LENGTH_LENGTH + MIN_FRAME_LENGTH)
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

// timeouts
#define SFP_POLL_TIME		(   2 TO_MSEC)		// polling in link down
#define SFP_RESEND_TIME		( 250 TO_MSECS)	// time between retransmissions
#define SFP_GIVEUP_TIME		(SFP_RESEND_TIME * 50)	// time for link to die
#define SPS_STARTUP_TIME	( 300 TO_MSECS)	// time to start sps
#define SFP_FRAME_TIME		(  50 TO_MSECS)	// maximum time to wait between bytes for a frame
#define SFP_FRAME_PROCESS	(1000 TO_MSECS)	// maximum time to wait for frame processing

#endif
