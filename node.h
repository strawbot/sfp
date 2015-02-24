// Define node of links  Robert Chapman  Apr 21, 2013

//#include "memorymaps.h"
#include "sfp.h"

#ifndef node_h
#define node_h

typedef struct  {
	Byte whoiam; // node number
	Byte whatiam; // card type 
	Byte numLinks; // number of links
	sfpLink_t **routes;
	char *name;  // my name
	sfpLink_t *defaultLink; // when there is no one to send to or null
	sfpLink_t *links[];
}sfpNode_t;

sfpNode_t *setNode(sfpNode_t *node);
sfpLink_t *nodeLink(Long n);

sfpLink_t * routeTo(Byte to);
void setRouteTo(Byte to, sfpLink_t * link);

#endif

