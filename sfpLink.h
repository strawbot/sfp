// SFP header file  Robert Chapman III  Feb 14, 2012

#ifndef SFP_LINK_H
#define SFP_LINK_H

#include "timeout.h"
#include "sfp.h"
#include "sfpStats.h"

// defines
#define LENGTH_LENGTH 	1
#define SYNC_LENGTH		1
#define PID_LENGTH		1
#define CHECKSUM_LENGTH 2
#define MIN_SFP_LENGTH 	(SYNC_LENGTH + PID_LENGTH + CHECKSUM_LENGTH)
#define MIN_SFP_FRAME 	(LENGTH_LENGTH + MIN_SFP_LENGTH)

#define MAX_SFP_LENGTH	(MAX_PACKET_LENGTH + MIN_SFP_LENGTH)
#define MAX_SFP_FRAME	(MAX_SFP_LENGTH + 1)
#define FRAME_OVERHEAD	(MIN_SFP_LENGTH - 1) // bytes used for transporting frame excluding pid

// one's complement doesn't create a zero while two's complement does
// should be better for checksum coverage
#define sfpSync(length)	((Byte)~(length))	// define sync mechanism

// timeouts
#define SFP_POLL_TIME		(2 TO_MSEC)		// polling in link down
#define SFP_RESEND_TIME	(250 TO_MSECS)	// time between retransmissions
#define SPS_STARTUP_TIME		(300 TO_MSECS)	// time to start sps
#define SFP_GIVEUP_TIME	(50 * SFP_RESEND_TIME)	// time for link to die
#define SFP_FRAME_TIME		(50 TO_MSECS)	// maximum time to wait between bytes for a frame
#define SFP_FRAME_PROCESS	(1000 TO_MSECS)	// maximum time to wait for frame processing

// Link state machine context
typedef enum {HUNTING, SYNCING, RECEIVING, PROCESSING} sfpRxState_t;
typedef enum {IDLING, TRANSMITTING} sfpTxState_t;
typedef enum {ANY_SPS, ONLY_SPS0, ONLY_SPS1, WAIT_ACK0, WAIT_ACK1} spsState_t;

// Link structure
typedef struct {	// Link information
	void *node;							// which node this link belongs to
	// Receiver
	Byte *rxq;							// point to queue of incoming bytes
	Byte *sfpRxPtr;					// point to frame being built
	Byte sfpBytesToRx;					// bytes to receive
	bool (*sfpRx)(void);				// is there something to receive?
	Byte (*sfpGet)(void);				// get the byte
	void (*rxErrFunction)(void);		// called for rx errors; vectored to allow link dependant action
	sfpRxState_t sfpRxState;			// SFP RX states
	spsState_t rxSps;						// which secure pid to look for next
	Byte frameIn[MAX_SFP_FRAME];		// incoming frame; first byte is length

	// Transmitter
	Byte *sfpTxPtr;					// point to frame being sent
	Byte sfpBytesToTx;					// bytes to send
	bool (*sfpTx)(void);				// can something be sent?
	void (*sfpPut)(Long);				// put the byte plus any upper bits
	sfpTxState_t sfpTxState;			// SFP TX states
	spsState_t txSps;						// which secure pid to send next
	Byte *frameOutNps;
	Byte frameOutNps1[MAX_SFP_FRAME];	// outgoing frame; first byte is length
	Byte frameOutNps2[MAX_SFP_FRAME];	// outgoing frame; first byte is length
	Byte frameOutSps[MAX_SFP_FRAME];	// outgoing frame; first byte is length
	Long txFlags;						// Pending Tx actions
	Byte enableSps;						// on to enable SPS

	// Both
	Timeout resendTo;	// resend timeout for SPS transmitter
	Timeout giveupTo;	// giveup timeout for SPS transmitter
	Timeout frameTo;	// maximum time between bytes when framebuilding
	Timeout packetTo;	// max time for processing a packet
	linkStats_t *stats;	// link to structure for stats
	Byte inFrameState;	// track where the inframe is
	char *name;			// link name
	Byte linkOwner;		// who owns the linke
	Byte routeTo;		// which link to route to if linkOwner is ROUTE_LINK
} sfpLink_t;

enum {NO_LINK, SFP_LINK, SERIAL_LINK, ROUTE_LINK}; // link owners
/*
default: set linkOwner as SFP_LINK
		 set routeTo as 0
		 if routeTo is 0, then that is dev/null
		 routeTo is only used when OTHER_LINK is engaged

if linkOwner is SERIAL_LINK then data in goes to ?; data out goes straight out
if linkOwner is NO_LINK then in and out data is tossed 
if linkOwner is ROUTE_LINK then data in is pushed to link specified in routeTo
if linkOwner is SFP_LINK then data in and data out are pushed to sfp protocol
*/

enum {FRAME_EMPTY, FRAME_FULL, FRAME_QUEUED, FRAME_PROCESSED, FRAME_REQUEUED};

// External use
void setSfpLink(sfpLink_t *l);
void initSfp(sfpLink_t *);
void calculateFletcherCheckSum(Byte *c1, Byte *c2, Byte length, Byte *data);
void buildSfpFrame(Byte length, Byte *data, Byte pid, Byte *f);
void rxLinkError(sfpLink_t *link);

typedef struct { // ''
	Byte length; // covers following bytes
	Byte sync;	// two's complement of length used to sync start of frame
	Byte pid;  // packet id upper two bits are for ack and sps
	Byte payload[];
} sfpFrame;

#endif
