// MQX Shell command support  Robert Chapman III  Sep 20, 2012

#include <string.h>

#include "bktypes.h"
#include "timeout.h"
#include "machines.h"
#include "sfpStats.h"
#include "sfpLink.h"
#include "sfpStatus.h"
#include "node.h"

void pingTestCmd(char *option, int number, int service);
void loadTestCmd(int number, char *option);
void pingTest(char *, linkInfo_t *, int, int);
void loadTest(int n, linkInfo_t *link);
linkInfo_t *lookupLinkName(char *name);

linkInfo_t *lookupLinkName(char *name) // find link of name
{
	int i;

	for (i=0; sfpNode->links[i] != NULL; i++)
		if (0 == strcmp(sfpNode->links[i]->name, name))
			return sfpNode->links[i];

	return sfpNode->links[0]; // default to first link
}

void pingTestCmd(char *option, int number, int service)
{			
	linkInfo_t *linkp = lookupLinkName(option);

	pingTest(linkp->name, linkp, number, service);
}

void loadTestCmd(int number, char *option)
{
	loadTest(number, lookupLinkName(option));
}