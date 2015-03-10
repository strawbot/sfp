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
    initNode();
    alink.txSps = ONLY_SPS0;
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
    initNode();
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
 *
 * NOTE: frames could get out of order if:
 *  o frame a for handler a is pushed to the retryq because handler a is busy
 *  o frame b for handler a comes in just as handler a is free and frame b is handled first
 *  o frame a is handled later when handler a becomes free.
 *  o the way to handle this is not to transfer to a retryq. keep frame in linkq and use timeout on frame in linkq
 *  o this unfortuneately blocks other pid frames in the frameq from being handled
 *  o it is if almost the pids need to come into play with the frameq. go through the entire frameq of frames. if there
 *    is a block, note it for a pid. if there are multiple frames for the pid, only the first one is tried. there also
 *    needs to be a timer on the first one for each pid in waiting. perhaps we go back to the retry q but add a pid
 *    in waiting flag. preventing the out of order thing. then timeout is only for frame at front of retryq.
 */
