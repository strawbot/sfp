// SFP Routing  Robert Chapman III  Jun 25, 2012

/*
  - each end of a link is connected to the other end by hardware
  - to connect to other end points routing is involved
  - routing to a destination is straight forward and can be
    accomplished by routing tables on each link
  - the routing tables should have default settings
  - the tables should be readable and writeable
  - some packets require responses end to end
  - the weak links are at routing points when a packet cannot
    be forwarded
  - SPS works between points but not over routing
  - end to end routing with confirmation is beyond SPS
  - when rerouting, the source packet service is not known and
    can't be used to choose a destination service
*/

#include "bktypes.h"
#include "timeout.h"
#include "machines.h"
#include "pids.h"
#include "sfpStats.h"
#include "sfpLink.h"
#include "packets.h"
#include "routing.h"
#include "node.h"
#include "printers.h"

#include <stdlib.h>

Byte whatAmI(void);

Byte talkTo = YOU; // set by incoming packets; used by TALK_OUT

// for redirecting main to slots
void talktoSlota(void);
void talktoSlota(void)
{
	print("\nPress escape to return ");
	talkTo = SLOTA_CPU;
}

void talktoSlotb(void);
void talktoSlotb(void)
{
	print("\nPress escape to return ");
	talkTo = SLOTB_CPU;
}

// This belongs in a file which is specific to the network -> nexusNetwork.c
Long reRoutes = 0;
char *routeNames[] = {
	"Direct",		// 0
	"Main",			// 1
	"Display",		// 2
	"Slota",		// 3
	"Slotb",		// 4
	"Main_Host",	// 5
	"Display_Host",	// 6
	"Slota_Host",	// 7
	"Slotb_Host"	// 8
};

char *routeName(Byte r) // resolve to a name
{
	if (r >= ROUTING_POINTS)
		return "who unknown";
	return routeNames[r];
}

linkInfo_t *routeToLink(Byte to) // link to send it to
{
	linkInfo_t *link = routeTo(to);
	
	if (link)
		return link;
	return sfpNode->defaultLink;
}

bool reRoutePacket(Byte *packet, Byte length, linkInfo_t *link)
{
	reRoutes++;
	return sendNormalPacketLink(packet, length, link);
}

Byte whoami(void) // return my identity
{
	return sfpNode->whoiam;
}

Byte whatAmI(void)
{
	return sfpNode->whatiam;
}

void setMe(Byte who) // create my identity
{
	if (who >= ROUTING_POINTS)
		return;
	sfpNode->whoiam = who;
}

void addRoute(Byte route, linkInfo_t *link)
{
	if (route >= ROUTING_POINTS)
		return;
	sfpNode->routes[route] = link;
}

void deleteRoute(Byte route)
{
	if (route >= ROUTING_POINTS)
		return;
	sfpNode->routes[route] = NULL;
}
