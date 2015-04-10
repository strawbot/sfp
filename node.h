// Define node of links  Robert Chapman  Apr 21, 2013

//#include "memorymaps.h"
#include "link.h"

#ifndef node_h
#define node_h

#define NODE_STAT(stat) Long stat;

typedef struct  {
	Byte whoiam; // node number
	Byte whatiam; // card type 
	sfpLink_t *routes[ROUTING_POINTS];
	sfpLink_t *links[NUM_LINKS];
	FOR_EACH_NODE_STAT(NODE_STAT)
}sfpNode_t;

sfpNode_t *getNode(void);
sfpNode_t *setNode(sfpNode_t *node);
sfpLink_t *nodeLink(Long n);
void addLink(Long n, sfpLink_t * link);
Byte whoami(void);
void setWhoami(Byte who);
Byte whatAmI(void);
void setWhatami(Byte what);

sfpLink_t * routeTo(Byte to);
void setRouteTo(Byte to, sfpLink_t * link);

#endif

