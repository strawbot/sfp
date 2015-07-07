#include <QtTest>

extern "C" {
#include "sfp.h"
#include "services.h"
#include "mocks.h"
#include "sfpTxSm.h"
}

#include "smallNetwork.h"

#include "testspi.h"

TestSpi::TestSpi(QObject *parent) :
    QObject(parent)
{
}

void serviceSlaveTx(sfpLink_t * link)
{
    if (bytesToSend(link)) {
        if (link->sfpTx(link))
            if (qbq(link->txq) < qbq(link->rxq)) {
                link->sfpBytesToTx--;
                link->sfpPut(*link->sfpTxPtr++, link);
            }
    }
    else
        if (qbq(link->txq) < qbq(link->rxq))
            link->sfpPut(0, link);
}

void TestSpi::TestOneSpiLink()
{
    initRoutes();
    pings = 0;
    links[5].link.serviceTx = serviceSlaveTx; // make link between node 3 and 4 as SPI link
    setPacketHandler(PING_BACK, pingBack);
    selectNode(4);
//    verbose = true;
    sendPacket(PING, 2);
    runNodes(SFP_SPS_TIME/2);
    QCOMPARE(pings, (Long)0);
    QCOMPARE(nodeStat(1,getReRouted), (Long)0);
    QCOMPARE(nodeStat(3,getReRouted), (Long)0);
    selectNode(3);
    sendPacket(CONFIG, 4);
    runNodes(SFP_SPS_TIME/2);
    QCOMPARE(pings, (Long)1);
    QCOMPARE(nodeStat(1,getReRouted), (Long)2);
    QCOMPARE(nodeStat(3,getReRouted), (Long)2);
}

void TestSpi::TestSpiPoll()
{
    initRoutes();
    pings = 0;
    links[5].link.serviceTx = serviceSlaveTx; // make link between node 3 and 4 as SPI link
    links[4].link.serviceTx = serviceMasterTx; // add polling to the master side
    setTimeout(SFP_POLL_TIME, &links[4].link.pollTo);
    setPacketHandler(PING_BACK, pingBack);
    selectNode(4);
    sendPacket(PING, 2);
//    verbose = true;
    runNodes(SFP_SPS_TIME/2);
    QCOMPARE(pings, (Long)1);
    QCOMPARE(nodeStat(1,getReRouted), (Long)2);
    QCOMPARE(nodeStat(3,getReRouted), (Long)2);
}

/*
 * test for :
 *  o slaves waiting for poll
 *  o multidrop slaves?
 *  o run clock at n*check rate
 */
