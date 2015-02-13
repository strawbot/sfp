// Test words for SFP  Robert Chapman III  Apr 19, 2012

/*
	need to:
	 > list and clear stats
	 > generate a test frame
	 > generate a frame with a pid
	 > status of link
	 > run tx or rx state machine
*/

#include "botkernl.h"
#include "timeout.h"
#include "machines.h"
#include "sfpStats.h"
#include "sfpLink.h"
#include "sfpTxSm.h"
#include "sfpStats.h"
#include "pids.h"
#include "routing.h"
#include "who.h"
#include "packets.h"
#include "node.h"
#include "printers.h"

void loadTest(Long n, Byte to);

void sfpLoadTest(void);
void sfpLoadTest(void)
{
	Long n;
	Byte to;

	n = *sp++;
	to = (Byte)*sp++;
	loadTest(n, to);
}

void pingTest(char *name, linkInfo_t *l, int p, int s);
Long numLinks(sfpNode_t *node);

void sfpPingTest(void);
void sfpPingTest(void)
{
	Long n = *sp++, i = *sp++ - 1; // number of pings, link number
	if (i < numLinks(sfpNode))
		pingTest(sfpNode->links[i]->name, sfpNode->links[i], (int)n, 0);
	else
		print(" Bad link number. Ain't gonna happen.\n");
}
