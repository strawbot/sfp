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
	sfpLink_t *link = &uartLink;

	sfpTxSm(link);
	link->serviceTx(link);
	sfpRxSm(link);
	processFrames();
	activate(sfpMachine);
}

void initSfp(void)
{
	sfpLink_t *link = &uartLink;
	
	// initialize link
	link->rxq = rxq;
	link->txq = txq; // would be loopback if use same q
	
	link->sfpRx = rxAvailable;
	link->sfpGet = rxGet;
	link->sfpTx = txOk;
	link->sfpPut = txPut;

	link->serviceTx = serviceTx;
	link->name = "Simple Network";

	// initialize pool of frame buffers
    initFramePool();

	// initialize state machines
	initSfpRxSM(link, frameq);
	initSfpTxSM(link, npsq, spsq);

	// initialize services and stats
    initSfpStats();
	initServices();
    
    // initialize the node
    setNode(&myNode);
    setWhoami(1);
    setWhatami(0);

    addLink(0, link); // attached links
    setRouteTo(0, link); // routes for other nodes
    
    activateOnce(sfpMachine);
}