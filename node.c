#include "node.h"

static sfpNode_t *currentNode = NULL;

// Node 
sfpNode_t *getNode(void)
{
	return currentNode;
}

sfpNode_t *setNode(sfpNode_t *node)
{
	sfpNode_t *previous = currentNode;

	currentNode = node;
	return previous;
}
	
sfpLink_t *nodeLink(Long n)
{
	if (currentNode)
		if (n < NUM_LINKS)
			return currentNode->links[n]; 
	return NULL;
}

void addLink(sfpLink_t * link)
{
    if (currentNode)
        for (Byte i = 0; i < NUM_LINKS; i++)
            if (currentNode->links[i] == NULL) {
                currentNode->links[i] = link;
                break;
            }
}

Byte whoami(void) // return my identity
{
    if (currentNode)
        return currentNode->whoiam;
    return ME;
}

void setWhoami(Byte who)
{
	if (who >= ROUTING_POINTS)
		return;
    if (currentNode)
        currentNode->whoiam = who;
}

Byte whatAmI(void)
{
    if (currentNode)
        return currentNode->whatiam;
    return NO_LINK;
}

void setWhatami(Byte what)
{
    if (currentNode)
        currentNode->whatiam = what;
}

// routing
sfpLink_t * routeTo(Byte to)
{
    if (currentNode)
        if (to < ROUTING_POINTS)
            return currentNode->routes[to];
    return NULL;
}

void setRouteTo(Byte to, sfpLink_t * link)
{
    if (currentNode)
        if (to < ROUTING_POINTS)
            currentNode->routes[to] = link;
}

void initNode(sfpNode_t *node)
{
	Byte i;
	
	for (i=0; i<ROUTING_POINTS; i++)
		node->routes[i] = NULL;

	for (i=0; i<NUM_LINKS; i++)
		node->links[i] = NULL;
}
