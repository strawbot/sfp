#ifndef PARAMETERS_H
#define PARAMETERS_H

// Default SFP Parameters  Robert Chapman III  Feb 21, 2015

// make a copy in application directory
// change parameters for application
// make sure application copy is first in the include list

#ifndef _PARAMETERS_H_
#define _PARAMETERS_H_

#define MAX_FRAME_LENGTH 254	// no more than 254; set according to link resources
#define MAX_FRAMES 100			// number of frames floating around system

// timeouts
#define SFP_POLL_TIME		(   2 TO_MSEC)		// polling in link down
#define SFP_SPS_TIME		( 250 TO_MSECS)	// time between retransmissions
#define SFP_FRAME_TIME		(  50 TO_MSECS)	// maximum time to wait between bytes for a frame
#define SFP_FRAME_PROCESS	(1000 TO_MSECS)	// maximum time to wait for frame processing
#define STALE_RX_FRAME 		(1000 TO_MSECS)	// number of milliseconds to hang onto a received frame

#define SPS_RETRIES			5

#define NUM_LINKS 2				// number of links in this node

// Routing
enum {
    HOST = 0x0,
    ME = 0x0,
    YOU = 0x0,
    DIRECT = 0x0,
    NODE1,
    NODE2,
    NODE3,
    NODE4,
    ROUTING_POINTS,
};

#endif
#endif // PARAMETERS_H
