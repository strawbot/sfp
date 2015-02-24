#include "timbre.h"

#ifndef _LINK_H_
#define _LINK_H_

// Link states
typedef enum {HUNTING, SYNCING, RECEIVING} sfpRxState_t;
typedef enum {IDLING, TRANSMITTING} sfpTxState_t;
typedef enum {ANY_SPS, ONLY_SPS0, ONLY_SPS1, WAIT_ACK0, WAIT_ACK1} spsState_t;
typedef enum {NO_LINK, SFP_LINK, SERIAL_LINK, ROUTE_LINK} linkOwner_t;
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

// Link structure
typedef struct {	// Link information
	void *node;							// which node this link belongs to
	// Receiver
	Byte *rxq;							// point to queue of incoming bytes
	Byte *sfpRxPtr;						// point to frame being built
	Byte sfpBytesToRx;					// bytes to receive
    Qtype *frameq;					// incoming frame queue
    Qtype *packetq;					// packets waiting to be processed
	bool (*sfpRx)(void);				// is there something to receive?
	Byte (*sfpGet)(void);				// get the byte
	void (*rxErrFunction)(void);		// called for rx errors; vectored to allow link dependant action
	sfpRxState_t sfpRxState;			// SFP RX states
	spsState_t rxSps;					// which secure pid to look for next

	// Transmitter
	Byte *sfpTxPtr;						// point to frame being sent
	Byte sfpBytesToTx;					// bytes to send
    Qtype *npsq;						// point to queue of nps frames to send
    Qtype *spsq;						// queue of SPS frames to send
	bool (*sfpTx)(void);				// can something be sent?
	void (*sfpPut)(Long);				// put the byte plus any upper bits
	sfpTxState_t sfpTxState;			// SFP TX states
	spsState_t txSps;					// which secure pid to send next
	Long txFlags;						// Pending Tx actions
	Byte enableSps;						// on to enable SPS

	// Both
	Timeout resendTo;	// resend timeout for SPS transmitter
	Timeout giveupTo;	// giveup timeout for SPS transmitter
	Timeout frameTo;	// maximum time between bytes when framebuilding
	Timeout packetTo;	// max time for processing a packet
//	linkStats_t *stats;	// link to structure for stats
	Byte inFrameState;	// track where the inframe is
	char *name;			// link name
	linkOwner_t linkOwner;		// who owns the linke
	Byte routeTo;		// which link to route to if linkOwner is ROUTE_LINK
} sfpLink_t;

#endif

// External use
void setSfpLink(sfpLink_t *l);
void initSfp(sfpLink_t *);

