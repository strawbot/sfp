// Default SFP Parameters  Robert Chapman III  Feb 21, 2015

// make a copy in application directory
// change parameters for application
// make sure application copy is first in the include list

#ifndef _PARAMETERS_H_
#define _PARAMETERS_H_

#define MAX_FRAME_LENGTH 255	// no more than 255; set according to link resources
#define MAX_FRAMES 10			// number of frames floating around system
#define STALE_RX_FRAME 1000		// number of milliseconds to hang onto a received frame

#define NUM_LINKS 1				// number of links in this node

// Routing
enum {
	HOST = 0x0,
	ME = 0x0,
	YOU = 0x0,
	DIRECT = 0x0,
	ROUTING_POINTS,
};

#endif