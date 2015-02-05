// Define node of links  Robert Chapman  Apr 21, 2013

#include "memorymaps.h"
#include "bktypes.h"

#ifndef node_h
#define node_h

typedef struct  {
	sfpLink_t **routes;
	Byte whoiam; // node number
	Byte whatiam; // card type 
	char *name;  // my name
	sfpLink_t *defaultLink; // when there is no one to send to or null
	sfpLink_t *links[];
}sfpNode_t;

extern sfpNode_t *sfpNode;
sfpNode_t *setNode(sfpNode_t *node);

#endif

