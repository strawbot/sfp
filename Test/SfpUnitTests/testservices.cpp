#include <QtTest>

#include "testservices.h"

extern "C" {
#include "sfp.h"
#include "services.h"
#include "mocks.h"
#include "link.h"
#include "node.h"
#include "framePool.h"
}

void initNodeServices(void)
{
    initTestNode();
    alink.txSps = ONLY_SPS0;
}

TestServices::TestServices(QObject *parent) :
    QObject(parent)
{
}

void TestServices::TestSendNpsNoLink()
{
    initTestLink();
    initSfpStats(); // called in initTestNode but we don't want a link yet

    for (Long i= MAX_FRAMES + 1; i; i--)
        QVERIFY(sendNpTo(packet, sizeof(packet), 0) == true);
    QCOMPARE(getNoDest(), (Long) (MAX_FRAMES + 1));
    QVERIFY(0 == getFramePoolEmpty());
    QVERIFY(0 == getPacketSizeBad());
}

void TestServices::TestNpsLink()
{
    initNodeServices();

    for (Long i= MAX_FRAMES; i; i--) // send all frames
        QVERIFY(sendNpTo(packet, sizeof(packet), DIRECT) == true);

    QCOMPARE(getNoDest(), (Long) (0));
    QVERIFY(sendNpTo(packet, sizeof(packet), DIRECT) == false); // try to send one more
    QCOMPARE(getFramePoolEmpty(), (Long) (1));
}

void TestServices::TestNpsNull()
{
    initNodeServices();
    QVERIFY(sendNpTo(packet, 0, DIRECT) == false);
    QCOMPARE(getPacketSizeBad(), (Long)1);
    QVERIFY(framePoolFull());
}

void TestServices::TestNpsOversize()
{
    initNodeServices();
    QVERIFY(sendNpTo(packet, MAX_PACKET_LENGTH + 1, DIRECT) == false);
    QCOMPARE(getPacketSizeBad(), (Long)1);
    QVERIFY(framePoolFull());
}

void TestServices::TestSendSpsNoLink()
{
    initNodeServices();
    setRouteTo(DIRECT, NULL);

    for (Long i= MAX_FRAMES + 1; i; i--)
        QVERIFY(sendSpTo(packet, sizeof(packet), 0) == true);
    QCOMPARE(getNoDest(), (Long) (MAX_FRAMES + 1));
    QVERIFY(0 == getFramePoolEmpty());
    QVERIFY(0 == getPacketSizeBad());
}

void TestServices::TestSpsLinkDown()
{
    initTestNode();
    QVERIFY(sendSpTo(packet, sizeof(packet), DIRECT) == false); // try to send one more
}

void TestServices::TestSpsLink()
{
    initNodeServices();

    for (Long i= MAX_FRAMES; i; i--) // send all frames
        QVERIFY(sendSpTo(packet, sizeof(packet), DIRECT) == true);

    QCOMPARE(getNoDest(), (Long) (0));
    QVERIFY(sendSpTo(packet, sizeof(packet), DIRECT) == false); // try to send one more
    QCOMPARE(getFramePoolEmpty(), (Long) (1));
}

void TestServices::TestSpsNull()
{
    initNodeServices();
    QVERIFY(sendSpTo(packet, 0, DIRECT) == false);
    QCOMPARE(getPacketSizeBad(), (Long)1);
    QVERIFY(framePoolFull());
}

void TestServices::TestSpsOversize()
{
    initNodeServices();
    QVERIFY(sendSpTo(packet, MAX_PACKET_LENGTH + 1, DIRECT) == false);
    QCOMPARE(getPacketSizeBad(), (Long)1);
    QVERIFY(framePoolFull());
}

void TestServices::TestPidList()
{
    Long ts = 0;

    initTestNode();
    alink.disableSps = true;
    alink.sfpRx = falseSfpRx;
    setPacketHandler(CONFIG, falseHandler);

    for (Byte i=0; i<3; i++)
        sendNpTo(packet, sizeof(packet), DIRECT);
    QCOMPARE(queryq(alink.npsq), (Long)3);

    while (queryq(alink.npsq)) { // queue up frames and add separated timestamps
        sfpFrame * frame = (sfpFrame * )pullq(alink.npsq);

        ts += 10;
        frame->timestamp = ts; // need to stagger timestamps to test each one separately
        pushq((Cell)frame, alink.receivedPool);
    }
    setTime(ts);
    QCOMPARE(framePoolLeft(), (Long)(MAX_FRAMES-3));
    runSm(STALE_RX_FRAME-ts+1);
    QCOMPARE(framePoolLeft(), (Long)(MAX_FRAMES-3));
    runSm(10);
    QCOMPARE(framePoolLeft(), (Long)(MAX_FRAMES-2));
    runSm(10);
    QCOMPARE(framePoolLeft(), (Long)(MAX_FRAMES-1));
    runSm(10);
    QCOMPARE(framePoolLeft(), (Long)(MAX_FRAMES));
}

void TestServices::TestPing()
{
    sfpFrame pingframe;

    pingframe.pid = PING;
    initNodeServices();
    rxFrame(&pingframe);
    QCOMPARE(getNoDest(), (Long)0); // no destination since it can't route it to self
    processFrames();
    QCOMPARE(getNoDest(), (Long)1); // no destination since it can't route it to self
}
