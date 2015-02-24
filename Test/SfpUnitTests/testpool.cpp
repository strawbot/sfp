#include <QtTest>

#include "testpool.h"
extern "C" {
#include "sfp.h"
#include "framepool.h"
#include "stats.h"
#include "mocks.h"
}

TestPool::TestPool(QObject *parent) :
    QObject(parent)
{
}

void TestPool::TestGetFrame()
{
    QCOMPARE((Long)getFrame(), (Long)0);
    QVERIFY(1 == getFramePoolEmpty());
}

void TestPool::TestInitPoolEmpty()
{
    initSfpStats();
    initFramePool();
    for (Long i= MAX_FRAMES; i; i--)
        QVERIFY((Long)getFrame() != (Long)0);
    QCOMPARE((Long)getFrame(), (Long)0);
    QVERIFY(1 == getFramePoolEmpty());
}

void TestPool::TestReturnFrame()
{
    initSfpStats();
    initFramePool();
    for (Long i= MAX_FRAMES + 1; i; i--) {
        sfpFrame * frame = getFrame();

        QVERIFY(frame != NULL);
        returnFrame(frame);
    }
    QVERIFY(0 == getFramePoolEmpty());
}
