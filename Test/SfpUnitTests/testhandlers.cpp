#include <QtTest>

#include "testhandlers.h"

extern "C" {
#include "sfp.h"
#include "services.h"
#include "mocks.h"
#include "link.h"
#include "node.h"
#include "framepool.h"
#include "frame.h"
}

TestHandlers::TestHandlers(QObject *parent) :
    QObject(parent)
{
}

void TestHandlers::TestGetHandler()
{
    QCOMPARE((Long)getPacketHandler(0), (Long)NULL);
    QCOMPARE((Long)getPacketHandler(MAX_PIDS), (Long)NULL);
}

void TestHandlers::TestSetHandler()
{
    QCOMPARE((Long)setPacketHandler(0, (packetHandler_t)1), (Long)NULL);
    QCOMPARE((Long)setPacketHandler(0, (packetHandler_t)2), (Long)1);
    QCOMPARE((Long)getPacketHandler(MAX_PIDS), (Long)NULL);
}

Long testframes;
bool acceptframe;

bool rxTestFrame(Byte *packet, Byte length)
{
    testframes++;
    (void)packet;
    (void)length;
    return acceptframe;
}

void qFrame()
{
    sfpFrame *frame;

    frame = getFrame();
    buildSfpFrame(sizeof(packet), packet, packet[0], frame);
    setPacketHandler(TEST_FRAME, rxTestFrame);
    rxFrame(frame);
    testframes = 0;
    acceptframe = true;
}

void setupFrame()
{
    initNode();
    qFrame();
}

void TestHandlers::TestPacketIn()
{
    setupFrame();
    callProcessFrames();
    QCOMPARE(testframes, (Long)1);
}

void TestHandlers::TestPacketRetry()
{
    setupFrame();
    acceptframe = false;
    callProcessFrames();
    QCOMPARE(testframes, (Long)1);
    acceptframe = true;
    callretryFrames();
    QCOMPARE(testframes, (Long)2);
}

void TestHandlers::TestStaleFrame()
{
    setupFrame();
    acceptframe = false;
    callProcessFrames();
    QCOMPARE(testframes, (Long)1);
    setTime(STALE_RX_FRAME);
    callretryFrames();
    QCOMPARE(testframes, (Long)2);
    callretryFrames();
    QCOMPARE(testframes, (Long)2);
}

void TestHandlers::TestAckPacket()
{
    initNode();
    packet[0] = SPS_ACK;
    qFrame();
    callProcessFrames();
    QCOMPARE(acksin, (Long)1);
}

void TestHandlers::TestSpsAckRequest()
{
    initNode();
    packet[0] |= ACK_BIT;
    qFrame();
    callProcessFrames();
    QCOMPARE(acksout, (Long)1);
    QCOMPARE(testframes, (Long)1);
}

void TestHandlers::TestSpsAckRequestIgnorePacket()
{
    initNode();
    packet[0] |= ACK_BIT;
    qFrame();
    spsaccept = false;
    callProcessFrames();
    QCOMPARE(acksout, (Long)1);
    QCOMPARE(testframes, (Long)0);
}

void TestHandlers::TestLinkLevelFrame()
{
    initNode();
    packet[0] = SPS;
    qFrame();
    setPacketHandler(SPS, rxTestFrame);
    callProcessFrames();
    QCOMPARE(testframes, (Long)1);
}

void TestHandlers::TestNoHandler()
{
    initNode();
    packet[0] = SPS;
    qFrame();
    callProcessFrames();
    QCOMPARE(testframes, (Long)0);
    QCOMPARE(getIgnoreFrame(), (Long)1);
}
