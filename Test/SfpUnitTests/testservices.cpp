#include <QtTest>

#include "testservices.h"

extern "C" {
#include "sfp.h"
#include "services.h"
#include "mocks.h"
#include "link.h"
#include "node.h"
}

TestServices::TestServices(QObject *parent) :
    QObject(parent)
{
}

void TestServices::TestSendNpsNoLink()
{
    initLink();

    for (Long i= MAX_FRAMES + 1; i; i--)
        QCOMPARE(sendNpTo(packet, sizeof(packet), 0), true);
    QCOMPARE(getNoDest(), (Long) (MAX_FRAMES + 1));
    QVERIFY(0 == getFramePoolEmpty());
    QVERIFY(0 == getPacketSizeBad());
}

void TestServices::TestNpsLink()
{
    initNode();

    for (Long i= MAX_FRAMES; i; i--) // send all frames
        QCOMPARE(sendNpTo(packet, sizeof(packet), DIRECT), true);

    QCOMPARE(getNoDest(), (Long) (0));
    QCOMPARE(sendNpTo(packet, sizeof(packet), DIRECT), false); // try to send one more
    QCOMPARE(getFramePoolEmpty(), (Long) (1));
}

void TestServices::TestNpsNull()
{
    initNode();
    QCOMPARE(sendNpTo(packet, 0, DIRECT), false);
    QCOMPARE(getPacketSizeBad(), (Long)1);
    QVERIFY(framePoolFull());
}

void TestServices::TestNpsOversize()
{
    initNode();
    QCOMPARE(sendNpTo(packet, MAX_PACKET_LENGTH + 1, DIRECT), false);
    QCOMPARE(getPacketSizeBad(), (Long)1);
    QVERIFY(framePoolFull());
}

void TestServices::TestSendSpsNoLink()
{
    initNode();
    setRouteTo(DIRECT, NULL);

    for (Long i= MAX_FRAMES + 1; i; i--)
        QCOMPARE(sendSpTo(packet, sizeof(packet), 0), true);
    QCOMPARE(getNoDest(), (Long) (MAX_FRAMES + 1));
    QVERIFY(0 == getFramePoolEmpty());
    QVERIFY(0 == getPacketSizeBad());
}

void TestServices::TestSpsLink()
{
    initNode();

    for (Long i= MAX_FRAMES; i; i--) // send all frames
        QCOMPARE(sendSpTo(packet, sizeof(packet), DIRECT), true);

    QCOMPARE(getNoDest(), (Long) (0));
    QCOMPARE(sendSpTo(packet, sizeof(packet), DIRECT), false); // try to send one more
    QCOMPARE(getFramePoolEmpty(), (Long) (1));
}

void TestServices::TestSpsNull()
{
    initNode();
    QCOMPARE(sendSpTo(packet, 0, DIRECT), false);
    QCOMPARE(getPacketSizeBad(), (Long)1);
    QVERIFY(framePoolFull());
}

void TestServices::TestSpsOversize()
{
    initNode();
    QCOMPARE(sendSpTo(packet, MAX_PACKET_LENGTH + 1, DIRECT), false);
    QCOMPARE(getPacketSizeBad(), (Long)1);
    QVERIFY(framePoolFull());
}
