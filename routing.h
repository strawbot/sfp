// routing ids  Robert Chapman III  Jun 25, 2012

#ifndef _ROUTING_H_
#define _ROUTING_H_

// routing points
#include "who.h"

// shortcuts
#define MAIN MAIN_CPU
#define DISPLAY DISPLAY_CPU
#define SLOTA SLOTA_CPU
#define SLOTB SLOTB_CPU

#endif

extern Byte talkTo;

linkInfo_t *routeTo(Byte to);
linkInfo_t *routeToLink(Byte to);
bool reRoutePacket(Byte *packet, Byte length, linkInfo_t *link);
Byte whoami(void);
void initRouting(void);
char *routeName(Byte r);

void addRoute(Byte route, linkInfo_t *link);
void deleteRoute(Byte route);
void setMe(Byte who); // create my identy
