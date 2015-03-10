#include <QtTest>

#include "testnetwork.h"
#include "smallNetwork.h"

TestNetwork::TestNetwork(QObject *parent) :
    QObject(parent)
{
}

void TestNetwork::initTestCase()
{
    initRoutes();
}

void TestNetwork::testPing()
{

}
