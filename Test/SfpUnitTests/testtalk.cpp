#include <QtTest>

extern "C" {
#include "sfp.h"
#include "services.h"
#include "byteq.h"

#include "talkHandler.c"

Byte eq[100];

void setTalkOut(vector talkOutPtr)
{
    (void)talkOutPtr;
}

void evaluate(Byte *string)
{
    (void)string;
}

BQUEUE(100, keyq);

void keyin(Byte c)
{
    pushbq(c, keyq);
}

void safe_emit(Byte c)
{
    (void)c;
}

}

#include "smallNetwork.h"

#include "testtalk.h"

TestTalk::TestTalk(QObject *parent) :
    QObject(parent)
{
}

void TestTalk::TestKeyin()
{
    packet_t *p = (packet_t *)packet;

    initRoutes();
    memcpy(p->whoload, "hello", 6);
    p->who.from = whoami();
    p->who.to = 2;
    p->pid = TALK_IN;
    initTalkHandler();
    selectNode(4);
    sendNpTo(packet, 9, 2);
    verbose = true;
    runNodes(SFP_SPS_TIME/2);
    QCOMPARE(qbq(keyq), (Byte)6);
}
