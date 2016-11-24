#include <QtTest>

#include "testservices.h"

extern "C" {
#include "sfp.h"
#include "services.h"
#include "mocks.h"
#include "link.h"
#include "node.h"
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
        QCOMPARE(sendNpTo(packet, sizeof(packet), 0), true);
    QCOMPARE(getNoDest(), (Long) (MAX_FRAMES + 1));
    QVERIFY(0 == getFramePoolEmpty());
    QVERIFY(0 == getPacketSizeBad());
}

void TestServices::TestNpsLink()
{
    initNodeServices();

    for (Long i= MAX_FRAMES; i; i--) // send all frames
        QCOMPARE(sendNpTo(packet, sizeof(packet), DIRECT), true);

    QCOMPARE(getNoDest(), (Long) (0));
    QCOMPARE(sendNpTo(packet, sizeof(packet), DIRECT), false); // try to send one more
    QCOMPARE(getFramePoolEmpty(), (Long) (1));
}

void TestServices::TestNpsNull()
{
    initNodeServices();
    QCOMPARE(sendNpTo(packet, 0, DIRECT), false);
    QCOMPARE(getPacketSizeBad(), (Long)1);
    QVERIFY(framePoolFull());
}

void TestServices::TestNpsOversize()
{
    initNodeServices();
    QCOMPARE(sendNpTo(packet, MAX_PACKET_LENGTH + 1, DIRECT), false);
    QCOMPARE(getPacketSizeBad(), (Long)1);
    QVERIFY(framePoolFull());
}

void TestServices::TestSendSpsNoLink()
{
    initNodeServices();
    setRouteTo(DIRECT, NULL);

    for (Long i= MAX_FRAMES + 1; i; i--)
        QCOMPARE(sendSpTo(packet, sizeof(packet), 0), true);
    QCOMPARE(getNoDest(), (Long) (MAX_FRAMES + 1));
    QVERIFY(0 == getFramePoolEmpty());
    QVERIFY(0 == getPacketSizeBad());
}

void TestServices::TestSpsLinkDown()
{
    initTestNode();
    QCOMPARE(sendSpTo(packet, sizeof(packet), DIRECT), false); // try to send one more
}

void TestServices::TestSpsLink()
{
    initNodeServices();

    for (Long i= MAX_FRAMES; i; i--) // send all frames
        QCOMPARE(sendSpTo(packet, sizeof(packet), DIRECT), true);

    QCOMPARE(getNoDest(), (Long) (0));
    QCOMPARE(sendSpTo(packet, sizeof(packet), DIRECT), false); // try to send one more
    QCOMPARE(getFramePoolEmpty(), (Long) (1));
}

void TestServices::TestSpsNull()
{
    initNodeServices();
    QCOMPARE(sendSpTo(packet, 0, DIRECT), false);
    QCOMPARE(getPacketSizeBad(), (Long)1);
    QVERIFY(framePoolFull());
}

void TestServices::TestSpsOversize()
{
    initNodeServices();
    QCOMPARE(sendSpTo(packet, MAX_PACKET_LENGTH + 1, DIRECT), false);
    QCOMPARE(getPacketSizeBad(), (Long)1);
    QVERIFY(framePoolFull());
}

/*
 * need a check for multiple frames in the retry q and first one times out but the next one doesn't until next timeout
 */
