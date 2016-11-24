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

void TestPool::initTestCase()
{
    setNode(&anode);
}

void TestPool::init()
{
    initFramePool();
}

void TestPool::TestGetFrame()
{
    for (Long i= MAX_FRAMES; i; i--)
        QVERIFY(getFrame() != NULL);
    QVERIFY(getFrame() == NULL);
}

void TestPool::TestPoolEmpty()
{
    QVERIFY(framePoolLeft() != 0);
    for (Long i= MAX_FRAMES; i; i--)
        getFrame();
    QVERIFY(framePoolLeft() == 0);
}

void TestPool::TestreturnFrame()
{
    for (Long i= MAX_FRAMES + 1; i; i--) {
        sfpFrame * frame = getFrame();

        QVERIFY(frame != NULL);
        returnFrame(frame);
    }
    QVERIFY(framePoolLeft() != 0);
}

void TestPool::TestIgetFrame()
{
    for (Long i= MAX_FRAMES - 1; i; i--)
        QVERIFY(igetFrame() != NULL);
    QVERIFY(igetFrame() == NULL);
}

void TestPool::TestIreturnFrame()
{
    for (Long i= MAX_FRAMES; i; i--)
        ireturnFrame(igetFrame());
    QVERIFY(igetFrame() != NULL);
}
