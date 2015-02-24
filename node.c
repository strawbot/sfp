#include "node.h"
#include "link.h"

static sfpNode_t *currentNode = NULL;
static sfpLink_t * routingTable[ROUTING_POINTS];

sfpLink_t *nodeLink(Long n)
{
	if (currentNode)
		if (currentNode->numLinks < n)
			return currentNode->links[n]; 
	return NULL;
}

sfpLink_t * routeTo(Byte to)
{
	if (to < ROUTING_POINTS)
    	return routingTable[to];
    return NULL;
}

void setRouteTo(Byte to, sfpLink_t * link)
{
	if (to < ROUTING_POINTS)
    	routingTable[to] = link;
}

Byte whoami(void) // return my identity
{
	return currentNode->whoiam;
}

Byte whatAmI(void)
{
	return currentNode->whatiam;
}

void setMe(Byte who) // create my identity
{
	if (who >= ROUTING_POINTS)
		return;
	currentNode->whoiam = who;
}
