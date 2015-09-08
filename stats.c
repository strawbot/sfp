// Stats  Robert Chapman III  Feb 21, 2015

#include "stats.h"
#include "node.h"
#include "link.h"
#include "printers.h"
#include "timeout.h"
#include "sfpTxSm.h"
#include "framepool.h"

#define SET_NODE_STAT(stat) \
void stat(void) { sfpNode_t * node = getNode(); node->stat++; }

#define SET_LINK_STAT(stat) \
void stat(sfpLink_t *link) { link->stat++; }

FOR_EACH_NODE_STAT(SET_NODE_STAT)
FOR_EACH_LINK_STAT(SET_LINK_STAT)

void LongFrame(Long length, sfpLink_t * link)
{
	link->LongFrame++;
	(void)length;
}

void ShortFrame(Long length, sfpLink_t * link)
{
	link->ShortFrame++;
	(void)length;
}

void initSfpStats(void)
{
    Long i;
    sfpNode_t * node = getNode();
    
#define ZERO_NODE_STAT(stat) node->stat = 0;
#define ZERO_LINK_STAT(stat) link->stat = 0;

	FOR_EACH_NODE_STAT(ZERO_NODE_STAT)

	for (i = 0; i < NUM_LINKS; i++) {
        sfpLink_t *link = nodeLink(i);
        if (link) {
            FOR_EACH_LINK_STAT(ZERO_LINK_STAT)
        }
	}
}

#define PRINT_NODE_STAT(stat) if (node->stat) print("\n"#stat" "), printDec(node->stat);
#define PRINT_LINK_STAT(stat) if (link->stat) print("\n"#stat" "), printDec(link->stat);

void showSfpStats(void)
{
    Long i;
    sfpNode_t * node = getNode();
    
	print("\nSfp Stats");
	FOR_EACH_NODE_STAT(PRINT_NODE_STAT)

	for (i = 0; i < NUM_LINKS; i++) {
        sfpLink_t *link = nodeLink(i);
        if (link) {
            print("\n*** Link#"), printDec(i);
            FOR_EACH_LINK_STAT(PRINT_LINK_STAT)
        }
    }
}

static void printSpsState(spsState_t state)
{
	switch(state) {
	case ANY_SPS: print(" ANY_SPS"); break;
	case ONLY_SPS0: print(" ONLY_SPS0"); break;
	case ONLY_SPS1: print(" ONLY_SPS1"); break;
	case WAIT_ACK0: print(" WAIT_ACK0"); break;
	case WAIT_ACK1: print(" WAIT_ACK1"); break;
	}
}

static void printRxState(sfpRxState_t state)
{
	switch(state) {
	case ACQUIRING: print(" ACQUIRING"); break;
	case HUNTING: print(" HUNTING"); break;
	case SYNCING: print(" SYNCING"); break;
	case RECEIVING: print(" RECEIVING"); break;
	}
}

static void printOwner(linkOwner_t state)
{
	print("\nLink Owner:");
	switch(state) {
	case NO_LINK: print(" NO_LINK"); break;
	case SFP_LINK: print(" SFP_LINK"); break;
	case SERIAL_LINK: print(" SERIAL_LINK"); break;
	case ROUTE_LINK: print(" ROUTE_LINK"); break;
	}
}

static void printTxActions(Long flag)
{
	print("\nTx actions:");
	if (flag & SEND_POLL_BIT) print(" SEND_POLL");
	if (flag &  SEND_ACK_BIT) print(" SEND_ACK");
	if (flag &  SEND_SPS_BIT) print(" SEND_SPS");
	if (flag &  RCVD_ACK_BIT) print(" RCVD_ACK");
}

void showLinkStatus(sfpLink_t * link)
{
	if (link->name)
		print("\n*** Link name: "), print(link->name);
	if (link->disableSps)
		print("\nSPS service disabled");
		
// Receiver
	if ((link->rxq) && (qbq(link->rxq)))
		print("\nBytes in rxq: "), printDec(qbq(link->rxq));
	if (link->sfpBytesToRx)
		print("\nbytes to receive: "), printDec(link->sfpBytesToRx);
	if (queryq(link->frameq))
		print("\nincoming frame queue: "), printDec(queryq(link->frameq));
	print("\nSFP RX state: "), printRxState(link->sfpRxState);
	if (!link->disableSps)
		print("\nRx SPS state: "), printSpsState(link->rxSps);

// Transmitter
	if (link->frameOut)
		print("\nFrame to be returned.");
	if (link->sfpBytesToTx)
		print("\nbytes to send:"), printDec(link->sfpBytesToTx);
	if (queryq(link->npsq))
		print("\nnps frames to send:"), printDec(queryq(link->npsq));
	if (!link->disableSps)
		if (queryq(link->spsq)) {
			print("\nsps frames to send:"), printDec(queryq(link->spsq));
			print("\nTx SPS state: "), printSpsState(link->txSps);
		if (checkTimeout(&link->spsTo))
			print("\nsps timed out");
		if (link->spsRetries)
			print("\nframe has been retried:"), printDec(link->spsRetries);
		}
	if (link->txFlags)
		printTxActions(link->txFlags);

// Both
	if (checkTimeout(&link->frameTo))
		print("\nexceeded maximum time between bytes when framebuilding");
	if (checkTimeout(&link->packetTo))
		print("\nexceeded max time for processing a packet");
	if (link->linkOwner)
		printOwner(link->linkOwner);
	if (link->routeTo)
		print("\nwhich link to route to if linkOwner is ROUTE_LINK"), printDec(link->routeTo);
	if (!link->reroute)
		print("\nrerouting disabled");
	if (link->listTxFrames)
		print("\ndisplay outgoing frames");
	if (link->listRxFrames)
		print("\ndisplay incoming frames");
}

void showNodeStatus(void)
{
    Long i;
    
	print("\nStatus of node links:");
	for (i = 0; i < NUM_LINKS; i++) {
        sfpLink_t *link = nodeLink(i);
        if (link)
        	showLinkStatus(link);
    }
    print("\nRouteable IDs: ");
    for (i = 0; i < ROUTING_POINTS; i++)
    	if (routeTo(i)) printDec(i);
    print("\nNumber of frames in pool:");
    printDec(framePoolLeft());
    print(" out of ");
    printDec(MAX_FRAMES);
}
