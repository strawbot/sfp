#include <QtTest>

extern "C" {
#include "sfp.h"
#include "services.h"
#include "mocks.h"
}

#include "testnetwork.h"
#include "smallNetwork.h"

TestNetwork::TestNetwork(QObject *parent) :
    QObject(parent)
{
}

void TestNetwork::initTestCase()
{
    initRoutes();
    pings = 0;
}

void TestNetwork::testPing()
{
    setPacketHandler(PING_BACK, pingBack);
    selectNode(4);
    sendPacket(PING, 2);
    runNodes(SFP_SPS_TIME/2);
    QCOMPARE(pings, (Long)1);
    QCOMPARE(nodeStat(1,getReRouted), (Long)2);
    QCOMPARE(nodeStat(3,getReRouted), (Long)2);
}

void TestNetwork::testSpsDirect()
{
    sfpFrame spsframe;

    spsframe.pid = PING;
    spsframe.who.from = 1;
    spsframe.who.to = 3;
    runNodes(SFP_SPS_TIME + 1);
    selectNode(1);
    sendSpTo(spsframe.packet, 3, 3);
    runNodes(SFP_SPS_TIME/2);
    selectNode(1);
    QCOMPARE(getSpsSent(nodeLink(1)), (Long)2);
    QCOMPARE(getSpsAcked(nodeLink(1)), (Long)2);
}

void TestNetwork::testSpsInDirect()
{
    sfpFrame spsframe;

    spsframe.pid = PING;
    spsframe.who.from = 1;
    spsframe.who.to = 4;
    runNodes(SFP_SPS_TIME + 1);
    initNetworkStats();
    selectNode(1);
    sendSpTo(spsframe.packet, 3, 4);
    runNodes(SFP_SPS_TIME/2);
    selectNode(1);
    QCOMPARE(getSpsSent(nodeLink(1)), (Long)1);
    QCOMPARE(getSpsAcked(nodeLink(1)), (Long)1);
    selectNode(3);
    QCOMPARE(getSpsSent(nodeLink(1)), (Long)1);
    QCOMPARE(getSpsAcked(nodeLink(1)), (Long)1);
    selectNode(4);
    QCOMPARE(getSpsSent(nodeLink(0)), (Long)0);
    QCOMPARE(getSpsAcked(nodeLink(0)), (Long)0);
}
