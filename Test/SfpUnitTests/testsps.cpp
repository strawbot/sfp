#include <QtTest>

#include "testsps.h"

extern "C" {
#include "sfp.h"
#include "services.h"
#include "mocks.h"
#include "link.h"
#include "node.h"
#include "framepool.h"
#include "frame.h"
#include "sfpRxSm.h"
#include "sfpTxSm.h"
}

static BQUEUE(MAX_FRAME_LENGTH-3, loopq);

static bool rxqAvailable(sfpLink_t * link)
{
    return (qbq(loopq) != 0);
    (void)link;
}

static Byte rxqGet(sfpLink_t * link)
{
    BytesIn(link);
    return pullbq(loopq);
}

static bool txOk(sfpLink_t * link)
{
    return !fullbq(loopq);
    (void)link;
}

bool dropbytes = false;

static void putTx(Long x, sfpLink_t * link)
{
    if (!dropbytes)
        pushbq((Byte)x, loopq);
    BytesOut(link);
}

void initSps()
{
    initTestNode();
    alink.sfpRx = rxqAvailable;
    alink.sfpGet = rxqGet;
    alink.sfpTx = txOk;
    alink.sfpPut = putTx;
    zerobq(loopq);
}

bool exitflag = false;

extern "C" void fpf(void)
{
    exitflag = true;
}

TestSps::TestSps(QObject *parent) :
    QObject(parent)
{
}

void TestSps::TestInitSps()
{
    sfpLink_t * link = &alink;

    initSps();
    QCOMPARE(link->rxSps, ANY_SPS);
    QCOMPARE(link->txSps, NO_SPS);
    runSm(SFP_SPS_TIME * 2);
    QCOMPARE(link->rxSps, ONLY_SPS1);
    QCOMPARE(link->txSps, ONLY_SPS1);
}

bool dropFrame(Byte *packet, Byte length)
{
    (void)packet;
    (void)length;
    qDebug() << "ACK dropped";
    return true;
}

Long spsframes = 0;

bool spsframe(Byte *packet, Byte length)
{
    (void)packet;
    (void)length;
    spsframes++;
    return true;
}

void TestSps::TestAckDropped()
{
    initSps();
    setPacketHandler(SPS_ACK, dropFrame);
    setPacketHandler(SPS, spsframe); // monitor frames received
    setTime(SFP_SPS_TIME + 1); // cause sps to be sent
    runSm(SFP_SPS_TIME); // get frame to be sent and ack dropped
    setPacketHandler(SPS_ACK, NULL); // allow next ack to be sent
    runSm(SFP_SPS_TIME); // resend frame
    QCOMPARE(alink.rxSps, ONLY_SPS1);
    QCOMPARE(alink.txSps, ONLY_SPS1);
    QCOMPARE(spsframes, (Long)1); // received only one copy even tho 2 sent
}

void TestSps::TestSpsDropped()
{
    initSps();
    dropbytes = true;
    setTime(SFP_SPS_TIME + 1); // cause sps to be sent
    runSm(SFP_SPS_TIME); // get frame to be sent and dropped
    qDebug() << "SPS dropped";
    dropbytes = false;
    runSm(SFP_SPS_TIME); // get frame to be sent and ack dropped
    QCOMPARE(alink.rxSps, ONLY_SPS1);
    QCOMPARE(alink.txSps, ONLY_SPS1);
}
