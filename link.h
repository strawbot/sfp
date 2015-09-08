#include "statGroups.h"
#include "sfp.h"

#ifndef _LINK_H_
#define _LINK_H_

// Link states
typedef enum {ACQUIRING, HUNTING, SYNCING, RECEIVING} sfpRxState_t;
typedef enum {ANY_SPS, NO_SPS = ANY_SPS, ONLY_SPS0, ONLY_SPS1, WAIT_ACK0, WAIT_ACK1} spsState_t;
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

#define LINK_STAT(stat) Long stat;

// Link structure
typedef struct sfpLink_t{	// Link information
	// Receiver
	Byte * rxq;							// point to queue of incoming bytes
	sfpFrame * frameIn;					// pointer to frame used for receiving
	Byte * sfpRxPtr;					// point to frame being built
	Byte sfpBytesToRx;					// bytes to receive
    Qtype * frameq;						// incoming frame queue
	bool (* sfpRx)(struct sfpLink_t *);	// is there something to receive?
	Byte (* sfpGet)(struct sfpLink_t *);// get the byte
	sfpRxState_t sfpRxState;			// SFP RX states
	spsState_t rxSps;					// which secure pid to look for next
	bool reroute;						// set to true if rerouting can be done
	bool listRxFrames;	// if set, then list the frames in frameq

	// Transmitter
	Byte * txq;							// point to queue of outgoing bytes
	sfpFrame * frameOut;				// point to frame being sent if it is to be returned
	Byte *sfpTxPtr;						// point to byte to be sent
    Qtype *npsq;						// point to queue of nps frames to send
    Qtype *spsq;						// queue of SPS frames to send
	bool (*sfpTx)(struct sfpLink_t *);	// can something be sent?
	void (*sfpPut)(Long, struct sfpLink_t *);// put the byte plus any upper bits
	void (*serviceTx)(struct sfpLink_t *); // service routine for transmitter
	Byte sfpBytesToTx;					// bytes to send
	spsState_t txSps;					// which secure pid to send next
	Timeout spsTo;						// sps timeout
	Long spsRetries;					// how many times sps frame has been retried
	Long txFlags;						// Pending Tx actions
	Byte disableSps;					// turn off sps service
	bool listTxFrames;					// display frames if enabled

	// Both
	Timeout frameTo;	// maximum time between bytes when framebuilding
	Timeout packetTo;	// max time for processing a packet
	Timeout pollTo;		// for polling links
	Byte inFrameState;	// track where the inframe is
	char *name;			// link name
	linkOwner_t linkOwner;		// who owns the linke
	Byte routeTo;		// which link to route to if linkOwner is ROUTE_LINK

	// stats
	Long LongFrame;
	Long ShortFrame;
	FOR_EACH_LINK_STAT(LINK_STAT)
} sfpLink_t;

#endif

void initLink(sfpLink_t * link, char * name);
