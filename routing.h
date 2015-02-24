// routing ids  Robert Chapman III  Jun 25, 2012

#ifndef _ROUTING_H_
#define _ROUTING_H_

extern Byte talkTo;

sfpLink_t *routeTo(Byte to);
sfpLink_t *routeToLink(Byte to);
bool reRoutePacket(Byte *packet, Byte length, sfpLink_t *link);
Byte whoami(void);
void initRouting(void);
char *routeName(Byte r);

void addRoute(Byte route, sfpLink_t *link);
void deleteRoute(Byte route);
void setMe(Byte who); // create my identy

#endif
