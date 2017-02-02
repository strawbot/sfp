// Simple network: two nodes sharing a link  Robert Chapman III  Apr 7, 2015

#include "node.h"

sfpNode_t myNode;
sfpLink_t uartLink;
BQUEUE(MAX_FRAME_LENGTH-3, rxq);
BQUEUE(MAX_FRAME_LENGTH-3, txq);
QUEUE(MAX_FRAMES, frameq);
QUEUE(MAX_FRAMES, npsq);
QUEUE(MAX_FRAMES, spsq);

// queue drivers
static bool rxAvailable(sfpLink_t * link)
{
    return (qbq(link->rxq) != 0);
}

static Byte rxGet(sfpLink_t * link)
{
    BytesIn(link);
    return pullbq(link->rxq);
}

static bool txOk(sfpLink_t * link)
{
    return !fullbq(link->txq);
}

static void txPut(Long x, sfpLink_t * link)
{
    pushbq((Byte)x, link->txq);
    BytesOut(link);
}

void sfpMachine(void)
{
	Byte i;

	for (i=0; i < NUM_LINKS; i++) {
		sfpLink_t *link = nodeLink(i);
		if (link) {
			sfpTxSm(link);
			link->serviceTx(link);
			sfpRxSm(link);
		}
	}
	processFrames();
	activate(sfpMachine);
}


void initSfp(void)
{
	sfpLink_t *link;

	// initialize pool of frame buffers
    initFramePool();

	// initialize the node
    initNode(&myNode);
    setNode(&myNode);
    setWhoami(MAIN_CPU);
    setWhatami(0);
 
	// initialize UART link
	link = &uartLink;
	initLink(link, "UART Link");
	link->rxq = rxq;
	link->txq = txq; // would be loopback if use same q

	// initialize state machines
	initSfpRxSM(link, frameq);
	initSfpTxSM(link, npsq, spsq);

	link->sfpRx = rxAvailable;
	link->sfpGet = rxGet;
	link->sfpTx = txOk;
	link->sfpPut = txPut;

	addLink(0, link); // attached links
	setRouteTo(DIRECT, link);
    setRouteTo(MAIN_HOST, link); // routes for other nodes

	// initialize services and stats
    initSfpStats();
	initServices();
    
    activateOnce(sfpMachine);
}