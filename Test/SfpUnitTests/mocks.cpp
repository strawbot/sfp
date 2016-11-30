// Mocks

#include <qdebug.h>
#include <string>
#include <sstream>

extern "C" {
#include <stdio.h>
#include "printers.h"
#include "mocks.h"
#include "sfpRxSm.h"
#include "sfpTxSm.h"
#include "framepool.c"
#include "link.c"

bool framePoolFull()
{
    return queryq(poolq) == MAX_FRAMES;
}

#include "stats.c"
#include "stats.h"

#define GET_NODE_STAT(stat) Long get##stat() { sfpNode_t * node = getNode(); return node->stat; }
#define GET_LINK_STAT(stat) Long get##stat(sfpLink_t * link) { return link->stat; }

FOR_EACH_NODE_STAT(GET_NODE_STAT)
FOR_EACH_LINK_STAT(GET_LINK_STAT)

#include "services.c"

void rxFrame(sfpFrame * frame)
{
    frame->timestamp = getTime();
    pushq((Cell)frame, alink.receivedPool);
}

void DOT_PROMPT()
{

}

#include "machines.c"

#include "timeout.c"

#include "byteq.c"

static Long ticktime;
Long getTime()
{
    return ticktime;
}

void setTime(Long t)
{
    ticktime = t;
}

#include "node.c"

Byte packet[3];
sfpLink_t alink;
sfpNode_t anode;
Long acksin, acksout;
bool spsaccept;

QUEUE(MAX_FRAMES, frameinq);
QUEUE(MAX_FRAMES, npsq);
QUEUE(MAX_FRAMES, spsq);

void initTestLink()
{
    setTime(0);
    initLink(&alink, (char *)"Lynx");
    alink.serviceTx = serviceTx;
    initSfpRxSM(&alink, frameinq);
    initSfpTxSM(&alink, npsq, spsq);
    initFramePool();
    initServices();
    initNode(&anode);
    setNode(&anode);
    packet[0] = CONFIG;    // pid
    packet[1] = DIRECT;        // to
    packet[2] = DIRECT;        // from
}

void initTestNode()
{
    initTestLink();
    addLink(DIRECT, &alink);
    initSfpStats();
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

void breakPoint1(void)
{
    exitflag = false;
}

bool verbose = false;

void frameSay(sfpFrame * frame, const char * dir)
{
    Long pid = frame->pid;
    Long to = frame->who.to;
    Long from = frame->who.from;
    Long me = whoami();
    Long time = getTime();
    Long timestamp = frame->timestamp;
    const char * spstag = "";

    if (!verbose) return;

    if (pid & ACK_BIT) {
        if (pid & SPS_BIT)
            spstag = "(SPS1)";
        else
            spstag = "(SPS0)";
    }

    std::ostringstream oss;
    oss << dir << time;
    if (me)
        oss << " Node " << me;
    oss << " Frame@" << timestamp << " " << spstag << " PID:";

    switch(pid&PID_BITS) {
    case CONFIG:		oss<< "CONFIG "; break;
    case PING:			oss<< "PING "; break;
    case PING_BACK:		oss<< "PING_BACK "; break;
    case SPS:			oss<< "SPS "; break;
    case SPS_ACK:		oss<< "SPS_ACK "; break;
    case GET_VERSION:	oss<< "GET_VERSION "; break;
    case VERSION_NO:	oss<< "VERSION_NO "; break;
    case TALK_IN:		oss<< "TALK_IN "; break;
    case TALK_OUT:		oss<< "TALK_OUT "; break;
    case EVAL:			oss<< "EVAL "; break;
    case CALL_CODE:		oss<< "CALL_CODE "; break;
    case MEMORY:		oss<< "MEM_READ "; break;
    case FILES:		    oss<< "MEM_DATA "; break;
    case MAX_PIDS:		oss<< "MAX_PIDS "; break;
    default:			oss<< "- Unknown: " << (pid&PID_BITS); break;
    }
    oss << "t:" << to << " f:" << from;
    qDebug() << oss.str().c_str();
}

void frameOut(sfpFrame * frame)
{
    frameSay(frame, "->@");
}

void frameIn(sfpFrame * frame)
{
    frameSay(frame, "<-@");
}

const char * spsState(spsState_t state)
{
    switch(state) {
    case ANY_SPS: return "ANY_SPS";
    case ONLY_SPS0: return "ONLY_SPS0";
    case ONLY_SPS1: return "ONLY_SPS1";
    case WAIT_ACK0: return "WAIT_ACK0";
    case WAIT_ACK1: return "WAIT_ACK1";
    default: return "unknown state";
    }
}

void txSpsState(sfpLink_t * link, spsState_t state)
{
    if (!verbose) return;

    qDebug() << "Node" << whoami() << link->name << "Tx SPS state changed from:" << spsState(link->txSps) << " to:" << spsState(state);
}

void runSm(Long n) // run the transmitter and receiver state machines n times
{
    while(n-- && !exitflag) {
        setTime(getTime() + 1);
        sfpTxSm(&alink);
        alink.serviceTx(&alink);
        sfpRxSm(&alink);
        processFrames();
    }
}

bool falseHandler(Byte *packet, Byte length)
{
    return false;
    (void)packet;
    (void)length;
}

bool falseSfpRx(sfpLink_t * link)
{
    return false;
    (void)link;
}

}
