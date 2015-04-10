#include <QString>
#include <QtTest>

#include "testframing.h"
#include "testpool.h"
#include "testservices.h"
#include "testhandlers.h"
#include "testreceiverstatemachine.h"
#include "testtransmitterstatemachine.h"
#include "testsps.h"
#include "testnetwork.h"
#include "testrouting.h"
#include "testspi.h"
#include "testuart.h"
#include "testtalk.h"

// Note: This is equivalent to QTEST_APPLESS_MAIN for multiple test classes.
int main(int argc, char** argv)
{
    int status = 0;

    {
        TestFraming tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestPool tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestServices tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestHandlers tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestReceiverStateMachine tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestTransmitterStateMachine tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestSps tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestRouting tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestNetwork tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

//    {
//        TestSpi tc;
//        status |= QTest::qExec(&tc, argc, argv);
//    }

    {
        TestTalk tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    if (status != 0)
        qDebug() << "Errors occured during testing." << status ;

    return status;
}
