#include <QtTest>

#include "testreceiverstatemachine.h"

extern "C" {
#include "sfp.h"
#include "services.h"
#include "mocks.h"
#include "link.h"
#include "node.h"
}

TestReceiverStateMachine::TestReceiverStateMachine(QObject *parent) :
    QObject(parent)
{
}

void TestReceiverStateMachine::TestHunting()
{

}

/*
 * Frame states: Hunting, Syncing, Receiving
 * Long frames
 * Short frames
 * SPI line states 0 and FF
 * Check sum errors
 * too many frames
 * SPS frame
 */
