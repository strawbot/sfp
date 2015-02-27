// Mocks

extern "C" {
#include <stdio.h>
#include "printers.h"
#include "mocks.h"
#include "sfpRxSm.h"
#include "sfpTxSm.h"

void print(const char * message)
{
    printf("%s", message);
}

void printHex(unsigned int hex)
{
    printf("%8X ", hex);
}

void printnHex(unsigned int n, unsigned int hex)
{
    if (n)
        printf("%*X ", n, hex);
    else
        printf("%X ", hex);
}

void printDec(unsigned int dec)
{
    printf("%d ", dec);
}

#include "framepool.c"

bool framePoolFull()
{
    return queryq(poolq) == MAX_FRAMES;
}

#include "stats.c"
#include "stats.h"

#define GET_STAT(stat) Long get##stat() { return stat##Stat; }
#define GET_LINK_STAT(stat) Long get##stat(sfpLink_t * link) { return link->stat; }

FOR_EACH_STAT(GET_STAT)
FOR_EACH_LINK_STAT(GET_LINK_STAT)

#include "services.c"

void rxFrame(sfpFrame * frame)
{
    pushq((Cell)frame, alink.frameq);
}

#include "machines.c"

#include "timeout.c"

#include "byteq.c"

static Long time;
Long getTime()
{
    return time;
}

void setTime(Long t)
{
    time = t;
}

#include "node.c"

Byte packet[3];
sfpLink_t alink;
sfpNode_t anode;
Long acksin, acksout;
bool spsaccept;

void initLink()
{
    setTime(0);
    initSfpRxSM(&alink);
    initSfpTxSM(&alink);
    initSfpStats();
    initFramePool();
    packet[0] = TEST_FRAME;    // pid
    packet[1] = DIRECT;        // to
    packet[2] = DIRECT;        // from
}

void initNode()
{
    initLink();
    setNode(&anode);
    addLink(DIRECT, &alink);
    setRouteTo(DIRECT, &alink);
    setWhoami(DIRECT);
    setWhatami(0);
    acksin = acksout = 0;
    spsaccept = true;
    for (int i = 0; i < MAX_PIDS; i++)
        setPacketHandler(i, NULL);
}

#include "sfpRxSm.c"
#include "sfpTxSm.c"

Long acksIn()
{
    sfpLink_t * link = &alink;

    if (testAckReceived(link)) {
        acksin++;
        clearAckReceived(link);
    }
    return acksin;
}

Long acksOut()
{
    sfpLink_t * link = &alink;

    if (testAckSend(link)) {
        acksout++;
        clearAckSend(link);
    }
    return acksout;
}

}
