#include <QtTest>

#include "testrouting.h"

extern "C" {
#include "sfp.h"
#include "services.h"
#include "mocks.h"
#include "link.h"
#include "node.h"
#include "framepool.h"
#include "frame.h"
#include "sfpRxSm.h"
#include "sfpTxSm.h"
}

#include "smallNetwork.h"

TestRouting::TestRouting(QObject *parent) :
    QObject(parent)
{
}

void TestRouting::TestInitRoutes()
{
    initRoutes();
    runNodes(SFP_SPS_TIME*3);

    Long j = elementsof(links);

    while (j--) {
        sfpLink_t * link = &links[j].link;
        QCOMPARE(link->rxSps, ONLY_SPS1);
        QCOMPARE(link->txSps, ONLY_SPS1);
    }
}

void TestRouting::TestNoRoutes()
{
    Long numnodes = elementsof(nodes);

    initRoutes();
    for (Long n = 1; n <= numnodes; n++) // Have every node send to all other nodes with a nonroutable frame
        for (Long i = 1; i <= numnodes; i++) {
            selectNode(n);
            if (i != n)
                sendPacket(CONFIG, i);
        }
    runNodes(SFP_SPS_TIME/2);
//    networkStats();

    QCOMPARE(nodeStat(2,getGoodFrame), (Long)1);
    QCOMPARE(nodeStat(1,getGoodFrame), (Long)numnodes+1);
    QCOMPARE(nodeStat(3,getGoodFrame), (Long)numnodes+1);
    QCOMPARE(nodeStat(4,getGoodFrame), (Long)1);

    QCOMPARE(nodeStat(2,getSendFrame), (Long)numnodes-1);
    QCOMPARE(nodeStat(1,getSendFrame), (Long)numnodes-1);
    QCOMPARE(nodeStat(3,getSendFrame), (Long)numnodes-1);
    QCOMPARE(nodeStat(4,getSendFrame), (Long)numnodes-1);
}

void TestRouting::TestRoutes()
{
    initRoutes();
    for (Long n = 1; n <= elementsof(nodes); n++)
        for (Long i = 1; i <= elementsof(nodes); i++) {
            selectNode(n);
            if (i != n)
                sendPacket(GET_VERSION, i);
        }
    runNodes(SFP_SPS_TIME/2);
//    networkStats();

    QCOMPARE(nodeStat(2,getGoodFrame), (Long)3);
    QCOMPARE(nodeStat(1,getGoodFrame), (Long)7);
    QCOMPARE(nodeStat(3,getGoodFrame), (Long)7);
    QCOMPARE(nodeStat(4,getGoodFrame), (Long)3);

    QCOMPARE(nodeStat(2,getSendFrame), (Long)3);
    QCOMPARE(nodeStat(1,getSendFrame), (Long)7);
    QCOMPARE(nodeStat(3,getSendFrame), (Long)7);
    QCOMPARE(nodeStat(4,getSendFrame), (Long)3);

    QCOMPARE(nodeStat(2,getReRouted), (Long)0);
    QCOMPARE(nodeStat(1,getReRouted), (Long)4);
    QCOMPARE(nodeStat(3,getReRouted), (Long)4);
    QCOMPARE(nodeStat(4,getReRouted), (Long)0);
}

void TestRouting::TestNoDest()
{
    initRoutes();
    for (Long n = 1; n <= elementsof(nodes); n++)
        for (Long i = 1; i <= elementsof(nodes); i++) {
            selectNode(n);
            if (i != n)
                sendPacket(GET_VERSION, i+4);
        }
    runNodes(SFP_SPS_TIME/2);
//    networkStats();

    QCOMPARE(nodeStat(2,getGoodFrame), (Long)0);
    QCOMPARE(nodeStat(1,getGoodFrame), (Long)0);
    QCOMPARE(nodeStat(3,getGoodFrame), (Long)0);
    QCOMPARE(nodeStat(4,getGoodFrame), (Long)0);

    QCOMPARE(nodeStat(2,getReRouted), (Long)0);
    QCOMPARE(nodeStat(1,getReRouted), (Long)0);
    QCOMPARE(nodeStat(3,getReRouted), (Long)0);
    QCOMPARE(nodeStat(4,getReRouted), (Long)0);

    QCOMPARE(nodeStat(2,getNoDest), (Long)3);
    QCOMPARE(nodeStat(1,getNoDest), (Long)3);
    QCOMPARE(nodeStat(3,getNoDest), (Long)3);
    QCOMPARE(nodeStat(4,getNoDest), (Long)3);
}
