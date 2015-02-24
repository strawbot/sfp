#include <QtTest>

#include "testservices.h"

extern "C" {
#include "sfp.h"
#include "services.h"
#include "mocks.h"
#include "stats.h"
#include "link.h"
#include "node.h"
#include "queue.h"
#include "framepool.h"
}

TestServices::TestServices(QObject *parent) :
    QObject(parent)
{
}

Byte packet[] = {TEST_FRAME, DIRECT, DIRECT}; // pid, to, from
sfpLink_t alink;
QUEUE(MAX_FRAMES, npsq);
QUEUE(MAX_FRAMES, spsq);

void initLink()
{
    setRouteTo(DIRECT, & alink);
    alink.npsq = npsq;
    zeroq(npsq);
    alink.spsq = spsq;
    zeroq(spsq);
    initSfpStats();
    initFramePool();
}

void TestServices::TestGetHandler()
{
    QCOMPARE((Long)getPacketHandler(0), (Long)NULL);
    QCOMPARE((Long)getPacketHandler(MAX_PIDS), (Long)NULL);
}

void TestServices::TestSetHandler()
{
    QCOMPARE((Long)setPacketHandler(0, (packetHandler_t)1), (Long)NULL);
    QCOMPARE((Long)setPacketHandler(0, (packetHandler_t)2), (Long)1);
    QCOMPARE((Long)getPacketHandler(MAX_PIDS), (Long)NULL);
}

void TestServices::TestSendNpsNoLink()
{
    initLink();
    setRouteTo(DIRECT, NULL);

    for (Long i= MAX_FRAMES + 1; i; i--)
        QCOMPARE(sendNpTo(packet, sizeof(packet), 0), true);
    QCOMPARE(getNoDest(), (Long) (MAX_FRAMES + 1));
    QVERIFY(0 == getFramePoolEmpty());
    QVERIFY(0 == getPacketSizeBad());
}

void TestServices::TestNpsLink()
{
    initLink();

    for (Long i= MAX_FRAMES; i; i--) // send all frames
        QCOMPARE(sendNpTo(packet, sizeof(packet), DIRECT), true);

    QCOMPARE(getNoDest(), (Long) (0));
    QCOMPARE(sendNpTo(packet, sizeof(packet), DIRECT), false); // try to send one more
    QCOMPARE(getFramePoolEmpty(), (Long) (1));
}

void TestServices::TestNpsNull()
{
    initLink();
    QCOMPARE(sendNpTo(packet, 0, DIRECT), false);
    QCOMPARE(getPacketSizeBad(), (Long)1);
    QVERIFY(framePoolFull());
}

void TestServices::TestNpsOversize()
{
    initLink();
    QCOMPARE(sendNpTo(packet, MAX_PACKET_LENGTH + 1, DIRECT), false);
    QCOMPARE(getPacketSizeBad(), (Long)1);
    QVERIFY(framePoolFull());
}

void TestServices::TestSendSpsNoLink()
{
    initLink();
    setRouteTo(DIRECT, NULL);

    for (Long i= MAX_FRAMES + 1; i; i--)
        QCOMPARE(sendSpTo(packet, sizeof(packet), 0), true);
    QCOMPARE(getNoDest(), (Long) (MAX_FRAMES + 1));
    QVERIFY(0 == getFramePoolEmpty());
    QVERIFY(0 == getPacketSizeBad());
}

void TestServices::TestSpsLink()
{
    initLink();

    for (Long i= MAX_FRAMES; i; i--) // send all frames
        QCOMPARE(sendSpTo(packet, sizeof(packet), DIRECT), true);

    QCOMPARE(getNoDest(), (Long) (0));
    QCOMPARE(sendSpTo(packet, sizeof(packet), DIRECT), false); // try to send one more
    QCOMPARE(getFramePoolEmpty(), (Long) (1));
}

void TestServices::TestSpsNull()
{
    initLink();
    QCOMPARE(sendSpTo(packet, 0, DIRECT), false);
    QCOMPARE(getPacketSizeBad(), (Long)1);
    QVERIFY(framePoolFull());
}

void TestServices::TestSpsOversize()
{
    initLink();
    QCOMPARE(sendSpTo(packet, MAX_PACKET_LENGTH + 1, DIRECT), false);
    QCOMPARE(getPacketSizeBad(), (Long)1);
    QVERIFY(framePoolFull());
}
