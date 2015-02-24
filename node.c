#include "node.h"
#include "link.h"

static sfpNode_t *currentNode = NULL;

// Node 
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

void addLink(Long n, sfpLink_t * link)
{
	if (n < NUM_LINKS)
		currentNode->links[n] = link; 
}

Byte whoami(void) // return my identity
{
	return currentNode->whoiam;
}

void setWhoami(Byte who)
{
	if (who >= ROUTING_POINTS)
		return;
	currentNode->whoiam = who;
}

Byte whatAmI(void)
{
	return currentNode->whatiam;
}

void setWhatami(Byte what)
{
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

