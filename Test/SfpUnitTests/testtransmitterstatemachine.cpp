#include <QtTest>

#include "testtransmitterstatemachine.h"

extern "C" {
#include "sfp.h"
#include "services.h"
#include "mocks.h"
#include "link.h"
#include "node.h"
#include "framepool.h"
#include "frame.h"
#include "sfpTxSm.h"
}

TestTransmitterStateMachine::TestTransmitterStateMachine(QObject *parent) :
    QObject(parent)
{
}

Long lasttx;
bool txok;

bool txOk(sfpLink_t * link)
{
    return txok;
    (void) link;
}

void putTx(Long x, sfpLink_t * link)
{
    lasttx = x;
    (void) link;
}

void initTxSm()
{
    initTestNode();
    alink.sfpTx = txOk;
    alink.sfpPut = putTx;
    txok = true;
    lasttx = 0x1FF;
}

void qNpsFrame()
{

}

void TestTransmitterStateMachine::TestSendFrame()
{
    sfpFrame frame;
    Byte * pp = &frame.length;

    buildSfpFrame(sizeof(packet) - 1, &packet[1], packet[0], &frame);

    initTxSm();
    QCOMPARE(sendNpTo(packet, sizeof(packet), DIRECT), true);
    sfpTxSm(&alink);
    for(Long i = frame.length + LENGTH_LENGTH; i; i--) {
        serviceTx(&alink);
        QCOMPARE((Byte)lasttx, *pp++);
    }
    lasttx = 0x1FF;
    serviceTx(&alink);
    QVERIFY(lasttx == 0x1FF);
}

void TestTransmitterStateMachine::TestSendAck()
{
    sfpFrame frame;
    Byte * pp = &frame.length, packet[3] = {SPS_ACK, 0, 0};

    buildSfpFrame(sizeof(packet) - 1, &packet[1], packet[0], &frame);

    initTxSm();
    spsReceived(&alink); // induce an ACK txmssion
    sfpTxSm(&alink);
    for(Long i = frame.length + LENGTH_LENGTH; i; i--) {
        serviceTx(&alink);
        QCOMPARE((Byte)lasttx, *pp++);
    }
}

void TestTransmitterStateMachine::TestSendPoll()
{
    sfpLink_t * link = &alink;

    initTxSm();
    setPollSend(link); // induce an poll txmssion
    sfpTxSm(&alink);
    serviceTx(&alink);
    QCOMPARE(lasttx, (Long)0);
    while(alink.sfpBytesToTx) {
        serviceTx(&alink);
        QCOMPARE(lasttx, (Long)0);
    }
}

void TestTransmitterStateMachine::TestSpsInit()
{
    sfpFrame frame;
    Byte * pp = &frame.length, packet[3] = {SPS|ACK_BIT, 0, 0};

    buildSfpFrame(sizeof(packet) - 1, &packet[1], packet[0], &frame);

    initTxSm();
    sfpTxSm(&alink);
    setTime(SFP_SPS_TIME + 1);
    sfpTxSm(&alink);
    sfpTxSm(&alink);
    for(Long i = frame.length + LENGTH_LENGTH; i; i--) {
        serviceTx(&alink);
        QCOMPARE((Byte)lasttx, *pp++);
    }
}

void TestTransmitterStateMachine::TestFramePriority()
{
    sfpLink_t * link = &alink;

    initFramePool();
    QVERIFY(framePoolFull());

    initTxSm();
    link->txSps = ONLY_SPS0;                    // need to put into sps ready state

    sendNpTo(packet, sizeof(packet), DIRECT);   // induce an NPS txmssion
    sendSpTo(packet, sizeof(packet), DIRECT);   // induce an SPS txmssion
    spsReceived(link);                        // induce an ACK txmssion
    setPollSend(link);                          // induce a poll txmssion

    for(Long i = MAX_SFP_SIZE * 4; i; i--) { // run enough times to handle 4 full frames
        sfpTxSm(link);
        serviceTx(link);
    }
    QCOMPARE(getSpsSent(link), (Long)1);
    QCOMPARE(getSendFrame(link), (Long)2);
    QCOMPARE(getPollFrame(link), (Long)1);

    QCOMPARE(getSpsAcked(link), (Long)0);
    spsAcknowledged(link);
    sfpTxSm(link);
    QCOMPARE(getSpsAcked(link), (Long)1);

    QVERIFY(framePoolFull());
}
