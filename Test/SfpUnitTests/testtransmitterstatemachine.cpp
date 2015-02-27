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

bool txOk()
{
    return txok;
}

void putTx(Long x)
{
    lasttx = x;
}

void initTxSm()
{
    initNode();
    alink.sfpTx = txOk;
    alink.sfpPut = putTx;
    txok = true;
    lasttx = 0x1FF;
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
    Byte * pp = &frame.length, packet[1] = {SPS_ACK};

    buildSfpFrame(sizeof(packet) - 1, &packet[1], packet[0], &frame);

    initTxSm();
    spsReceived(DIRECT); // induce an ACK txmssion
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
    sfpLink_t * link = &alink;
    sfpFrame frame;
    Byte * pp = &frame.length, packet[1] = {SPS};

    buildSfpFrame(sizeof(packet) - 1, &packet[1], packet[0], &frame);


    initTxSm();
    sfpTxSm(&alink);
    QVERIFY(testSpsInit(link) == false);
    setTime(SFP_SPS_TIME + 1);
    sfpTxSm(&alink);
    for(Long i = frame.length + LENGTH_LENGTH; i; i--) {
        serviceTx(&alink);
        QCOMPARE((Byte)lasttx, *pp++);
    }
}

/*
 * Tests:
 * loopback sps test
 */
