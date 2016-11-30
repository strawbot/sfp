extern "C" {
#include "sfp.h"
#include "services.h"
#include "mocks.h"
#include "link.h"
#include "framepool.h"
#include "sfpRxSm.h"
#include "sfpTxSm.h"
#include "mocks.h"
void processFrames();
}

#include <stdio.h>

#include "smallNetwork.h"

// network of 4 nodes:
// N2 <> N1 <> N3 <> N4
// or
// N1 -- N2
// |
// N3 -- N4
/*
 * Network links consist of a byte queue connecting two end points together. Tx of one end point
 * puts characters into a queue which is checked by the receiver of the connected end point. A
 * link will have two queues for full duplex operation. This is similar to a UART. For SPI, the
 * transmitter can only put a character out when a character comes in.
 *
 * For a SPI slave transmitter, if there is nothing to put in the tx q then a zero should be put
 * out. Only transmit a character when receiving a character.
 *
 * Or make slave txq separate from master receive q and transfer from slave txq to master rxq only
 * when master transmits. Make slave txq short. Either 2 bytes or 16 bytes like a SPI FIFO.  Who puts
 * the zero in the queue? Does it stick? or does it need to be refurbished? If there are no characters
 * in the receive q should it come in as an 0xFF?
 *
 * Master SPI:
 *  o send a character and receive a character at the same time. Must send to receive. So every tx
 *    causes an rx. If there are no characters to rx, then 0xFF is received. If the queue is down to
 *    one, then that character is read repeatedly. If the queue is empty, then a 0xFF is read. The
 *    receive queue is never read below 1.
 * Slave SPI:
 *  o characters are received and sent as UART. Queue size limits how much can be sent. If there is
 *    nothing to be sent, then the last character sent is repeated. Initially it is an 0xFF. When a
 *    frame has been sent and there is no followup frame to be sent, then a zero should be transmitted.
 *
 * A queue joins the master tx to the slave rx.
 */
sfpNode_t nodes[4];

void processFrames(void);

struct qlink links[6];
const char *names[6] = {"link0", "link1", "link2", "link3", "link4", "link5"};

QUEUE(MAX_FRAMES, frame1q);
QUEUE(MAX_FRAMES, nps1q);
QUEUE(MAX_FRAMES, sps1q);
QUEUE(MAX_FRAMES, frame2q);
QUEUE(MAX_FRAMES, nps2q);
QUEUE(MAX_FRAMES, sps2q);
QUEUE(MAX_FRAMES, frame3q);
QUEUE(MAX_FRAMES, nps3q);
QUEUE(MAX_FRAMES, sps3q);
QUEUE(MAX_FRAMES, frame4q);
QUEUE(MAX_FRAMES, nps4q);
QUEUE(MAX_FRAMES, sps4q);
QUEUE(MAX_FRAMES, frame5q);
QUEUE(MAX_FRAMES, nps5q);
QUEUE(MAX_FRAMES, sps5q);
QUEUE(MAX_FRAMES, frame6q);
QUEUE(MAX_FRAMES, nps6q);
QUEUE(MAX_FRAMES, sps6q);

// node 1 has links 0 and 2
// node 2 has link 1
// node 3 has link 3 and 4
// node 4 has link 5
Qtype * frameqs[6] = {frame1q, frame2q, frame3q, frame4q, frame5q, frame6q};
Qtype * npsqs[6] = {nps1q, nps2q, nps3q, nps4q, nps5q, nps6q};
Qtype * spsqs[6] = {sps1q, sps2q, sps3q, sps4q, sps5q, sps6q};

// UART drivers
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

// SPI drivers
// use link->serviceTx(link) to service tx link; normally for UART this is the one in sfpTxSm.c
// For SPI slave, use the one below
//need to set parameters here like NUM_LINKS, ROUTING_POINTS
//can we use undef? but that would have to be before includes or after sfp.f
void initNetworkStats()
{
    for (Long i=0; i<elementsof(nodes); i++) { // setup node who and what
        setNode(&nodes[i]);
        initSfpStats();
    }
}

void initRoutes()
{
    setTime(0);
    initFramePool();

    for (Long i=0; i<elementsof(links); i++) { // initialize link SM's and queues
        sfpLink_t * link = &links[i].link;

        initSfpRxSM(link, frameqs[i]);
        initSfpTxSM(link, npsqs[i], spsqs[i]);
        INIT_BQ(links[i].byteq);
        link->rxq = links[i].byteq;
        link->sfpRx = rxAvailable;
        link->sfpGet = rxGet;
        link->sfpTx = txOk;
        link->sfpPut = txPut;
        link->serviceTx = serviceTx;
        link->name = (char *)names[i];
    }

    for (Long i=0; i<elementsof(nodes); i++) { // setup node who and what
        setNode(&nodes[i]);
        setWhoami(i+1);
        setWhatami(0);
    }

    initNetworkStats();
    setNode(&nodes[0]);    // Node 1 connections
    addLink(0, &links[0].link); // attached links
    addLink(1, &links[2].link);
    setRouteTo(2, &links[0].link); // routes for other nodes
    setRouteTo(3, &links[2].link);
    setRouteTo(4, &links[2].link);
    setNode(&nodes[1]);    // Node 2 connections
    addLink(0, &links[1].link);
    setRouteTo(1, &links[1].link);
    setRouteTo(3, &links[1].link);
    setRouteTo(4, &links[1].link);
    setNode(&nodes[2]);    // Node 3 connections
    addLink(0, &links[3].link);
    addLink(1, &links[4].link);
    setRouteTo(1, &links[3].link);
    setRouteTo(2, &links[3].link);
    setRouteTo(4, &links[4].link);
    setNode(&nodes[3]);    // Node 4 connections
    addLink(0, &links[5].link);
    setRouteTo(1, &links[5].link);
    setRouteTo(2, &links[5].link);
    setRouteTo(3, &links[5].link);

    //link 0 1>2
    //link 1   2>1
    //link 2 1>3
    //link 3   3>1
    //link 4 3>4
    //link 5   4>3

    links[0].link.txq = links[1].link.rxq; // cross connect links for simulating connections
    links[1].link.txq = links[0].link.rxq;

    links[2].link.txq = links[3].link.rxq;
    links[3].link.txq = links[2].link.rxq;

    links[4].link.txq = links[5].link.rxq;
    links[5].link.txq = links[4].link.rxq;
}

Long clockRatio = 1; // how many clock ticks per loop of network

void timeTick()
{
    static Long ticks = 1;

    if (--ticks == 0) {
        ticks = clockRatio;
        setTime(getTime() + 1);
    }
}

void setClockRatio(Long n)
{
    clockRatio = n;
}

void runNodes(Long i) // how many milliseconds to cycle through nodes in network
{
    Long end = getTime() + i;

    while (getTime() < end) {
        Long j = elementsof(nodes);

        timeTick();
        while (j--) {
            setNode(&nodes[j]);
            for (Long k = 0; k < NUM_LINKS; k++) {
                sfpLink_t * link = nodeLink(k);

                if (link) {
                    sfpTxSm(link);
                    link->serviceTx(link);
                    sfpRxSm(link);
                }
            }
            processFrames();
        }
    }
}

void selectNode(Long n)
{
    setNode(&nodes[n-1]);
}

sfpFrame test;

void sendPacket(Byte pid, Byte to)
{
    test.pid = pid;
    test.who.from = whoami();
    test.who.to = to;
    sendNpTo(test.packet, 3, to);
}

void networkStats()
{
    for (Long n = 0; n < elementsof(nodes); n++) {
        selectNode(n+1);
        printf("\n****** NODE: %lu ********",n+1);
        showNodeStatus();
        showSfpStats();
    }
}

Long nodeStat(Long node, Long (*stat)(sfpLink_t *link))
{
    sfpLink_t * link;
    Long sum = 0;

    selectNode(node);
    for (Long l=0; l<NUM_LINKS; l++)
        if ( (link = nodeLink(l)) != 0 )
            sum += stat(link);
    return sum;
}

Long nodeStat(Long node, Long (*stat)())
{
    selectNode(node);
    return stat();
}

Long pings;

bool pingBack(Byte * packet, Byte length)
{
    pings++;
    (void)packet;
    (void)length;
    return true;
}

