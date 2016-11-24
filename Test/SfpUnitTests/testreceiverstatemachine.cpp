#include <QtTest>

#include "testreceiverstatemachine.h"

extern "C" {
#include "sfp.h"
#include "services.h"
#include "mocks.h"
#include "link.h"
#include "node.h"
#include "sfpRxSm.h"
#include "framepool.h"
#include "frame.h"
}

TestReceiverStateMachine::TestReceiverStateMachine(QObject *parent) :
    QObject(parent)
{
}

static BQUEUE(10, testrxq);

static bool rxqAvailable(sfpLink_t * link)
{
    return (qbq(testrxq) != 0);
    (void) link;
}

static Byte rxqGet(sfpLink_t * link)
{
    return pullbq(testrxq);
    (void) link;
}

static void initRxSm()
{
    initTestNode();
    alink.sfpRx = rxqAvailable;
    alink.sfpGet = rxqGet;
    zerobq(testrxq);
}

void TestReceiverStateMachine::TestAcquiring()
{
    initRxSm();
    QCOMPARE(alink.sfpRxState, ACQUIRING); // initial state
    QCOMPARE(sfpRxSm(&alink), false);
    QCOMPARE(alink.sfpRxState, ACQUIRING); // got frame looking for data
    pushbq(0, testrxq);
    QCOMPARE(sfpRxSm(&alink), false);
    QCOMPARE(alink.sfpRxState, HUNTING); // data and frame buffer available -> start receiving
}

void TestReceiverStateMachine::TestHunting()
{
    initRxSm();
    pushbq(0, testrxq);
    QCOMPARE(sfpRxSm(&alink), false);
    QCOMPARE(sfpRxSm(&alink), true);
    QVERIFY(qbq(testrxq) == 0);
    QCOMPARE(alink.sfpRxState, HUNTING); // Test 0
    pushbq(MIN_FRAME_LENGTH - 1, testrxq);
    QCOMPARE(sfpRxSm(&alink), true);
    QCOMPARE(alink.sfpRxState, HUNTING); // Test lower limit
    pushbq(0xFF, testrxq);
    QCOMPARE(sfpRxSm(&alink), true);
    QCOMPARE(alink.sfpRxState, HUNTING); // Test FF
    pushbq(MAX_FRAME_LENGTH + 1, testrxq);
    QCOMPARE(sfpRxSm(&alink), true);
    QCOMPARE(alink.sfpRxState, HUNTING); // Test upper limit
    pushbq(MIN_FRAME_LENGTH, testrxq);
    QCOMPARE(sfpRxSm(&alink), true);
    QCOMPARE(alink.sfpRxState, SYNCING); // Test minimimum
    alink.sfpRxState = HUNTING;
    pushbq(MAX_FRAME_LENGTH, testrxq);
    QCOMPARE(sfpRxSm(&alink), true);
    QCOMPARE(alink.sfpRxState, SYNCING); // Test maximimum
}

void TestReceiverStateMachine::TestSyncing()
{
    initRxSm();
    pushbq(MIN_FRAME_LENGTH, testrxq); // min size followed by another minsize
    pushbq(MIN_FRAME_LENGTH, testrxq);
    sfpRxSm(&alink);
    sfpRxSm(&alink);
    sfpRxSm(&alink);
    QCOMPARE(alink.sfpRxState, SYNCING); // Test bad sync
    QCOMPARE(getBadSync(&alink), (Long)1);
    pushbq(0, testrxq);
    sfpRxSm(&alink);
    QCOMPARE(alink.sfpRxState, HUNTING); // Test bad length
}

void TestReceiverStateMachine::TestReceiving()
{
    initRxSm();

    sfpFrame * frame = getFrame();
    Byte * fp = &frame->length, i;

    buildSfpFrame(sizeof(packet) - 1, &packet[1], packet[0], frame);
    for (i = 0; i < frame->length + LENGTH_LENGTH; i++) // good frame
        pushbq(fp[i], testrxq);
    while (qbq(testrxq))
        sfpRxSm(&alink);
    QCOMPARE(alink.sfpRxState, HUNTING);
    QCOMPARE(getGoodFrame(&alink), (Long)1);

    for (i = 0; i < frame->length; i++) // bad frame
        pushbq(fp[i], testrxq);
    pushbq(~fp[i], testrxq);
    while (qbq(testrxq))
        sfpRxSm(&alink);
    QCOMPARE(alink.sfpRxState, HUNTING);
    QCOMPARE(getBadCheckSum(&alink), (Long)1);

    for (i = 0; i < frame->length; i++)  // frame timeout
        pushbq(fp[i], testrxq);
    while (qbq(testrxq))
        sfpRxSm(&alink);
    setTime(SFP_FRAME_TIME+1);
    sfpRxSm(&alink);
    QCOMPARE(alink.sfpRxState, HUNTING);
    QCOMPARE(getRxTimeout(&alink), (Long)1);
    setTime(2*SFP_FRAME_TIME+1);
    QCOMPARE(getGoodFrame(&alink), (Long)1);
    for (i = 0; i < frame->length + LENGTH_LENGTH; i++) { // good frame after timeout
        pushbq(fp[i], testrxq);
        sfpRxSm(&alink);
        sfpRxSm(&alink);
    }
    QCOMPARE(alink.sfpRxState, HUNTING);
}

void TestReceiverStateMachine::TestOverflow()
{
    initRxSm();

    sfpFrame * frame = getFrame();
    Byte * fp = &frame->length, i;

    buildSfpFrame(sizeof(packet) - 1, &packet[1], packet[0], frame);
    for (int n = MAX_FRAMES - 1; n != 0; n--) { // receive frames until we run out of frame buffers
        for (i = 0; i < frame->length + LENGTH_LENGTH; i++) // good frame
            pushbq(fp[i], testrxq);
        while (qbq(testrxq))
            sfpRxSm(&alink);
    }
    for (i = 0; i < frame->length + LENGTH_LENGTH; i++) // receive one more
        pushbq(fp[i], testrxq);
    QCOMPARE(alink.sfpRxState, ACQUIRING);
    sfpRxSm(&alink);
    QCOMPARE(alink.sfpRxState, ACQUIRING); // no more frame buffers available; stay here
}
