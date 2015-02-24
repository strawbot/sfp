#include <QtTest>

#include "testframing.h"

extern "C" {
#include "sfp.h"
#include "frame.h"
}

Q_DECLARE_METATYPE(Byte)
Q_DECLARE_METATYPE(Byte *)

static Byte zeros[] = {0,0,0,0};
static Byte ones[] = {1,1,1,1};
static Byte onefour[] = {1,2,3,4};
static Byte tens[] = {10,10,10,10};
static Byte ff[] = {0xFF,0xFF,0xFF,0xFF};

TestFraming::TestFraming(QObject *parent) :
    QObject(parent)
{
}

void TestFraming::TestCheckSum_data()
{
    QTest::addColumn<Byte>("Sum");
    QTest::addColumn<Byte>("SumSum");
    QTest::addColumn<Byte>("Length");
    QTest::addColumn<Byte*>("Data");

    QTest::newRow("All Zeroes") << (uchar)0 << (uchar)0 << (uchar)sizeof(zeros) << &zeros[0];
    QTest::newRow("All Ones") << (uchar)4 << (uchar)10 << (uchar)sizeof(ones) << &ones[0];
    QTest::newRow("All OneFour") << (uchar)10 << (uchar)20 << (uchar)sizeof(onefour) << &onefour[0];
    QTest::newRow("All Tens") << (uchar)40 << (uchar)100 << (uchar)sizeof(tens) << &tens[0];
    QTest::newRow("All ffs") << (uchar)252 << (uchar)246 << (uchar)sizeof(ff) << &ff[0];
}

void TestFraming::TestCheckSum()
{
    Byte c1, c2;

    QFETCH(Byte, Sum);
    QFETCH(Byte, SumSum);
    QFETCH(Byte, Length);
    QFETCH(Byte *, Data);

    c1 = c2 = 0;
    calculateFletcherCheckSum(&c1, &c2, Length, Data);

//    qDebug("Sum %d, c1 %d, SumSum %d, c2 %d, Length, %d",Sum,c1,SumSum,c2,Length);

    QCOMPARE(c1, Sum);
    QCOMPARE(c2, SumSum);
}

void TestFraming::TestFrameBuild_data()
{
    Byte x;

    QTest::addColumn<Byte>("Length");
    QTest::addColumn<Byte>("Sync");
    QTest::addColumn<Byte>("Pid");
    QTest::addColumn<Byte>("PayloadSize");
    QTest::addColumn<Byte*>("Payload");
    QTest::addColumn<Byte>("Sum");
    QTest::addColumn<Byte>("SumSum");

    x = sizeof(zeros)+MIN_FRAME_LENGTH;
    QTest::newRow("All Zeroes") << x << (uchar)~x << (uchar)0 << (uchar)sizeof(zeros) << &zeros[0] << (uchar)255 << (uchar)2;
    x = sizeof(ones)+MIN_FRAME_LENGTH;
    QTest::newRow("All Ones") << x << (uchar)~x << (uchar)1 << (uchar)sizeof(ones) << &ones[0] << (uchar)4 << (uchar)17;
    x = sizeof(onefour)+MIN_FRAME_LENGTH;
    QTest::newRow("All OneFour") << x << (uchar)~x << (uchar)5 << (uchar)sizeof(onefour) << &onefour[0] << (uchar)14 << (uchar)47;
    x = sizeof(tens)+MIN_FRAME_LENGTH;
    QTest::newRow("All Tens") << x << (uchar)~x << (uchar)10 << (uchar)sizeof(tens) << &tens[0] << (uchar)49 << (uchar)152;
    x = sizeof(ff)+MIN_FRAME_LENGTH;
    QTest::newRow("All ffs") << x << (uchar)~x << (uchar)0xFF << (uchar)sizeof(ff) << &ff[0] << (uchar)250 << (uchar)243;
}

void TestFraming::TestFrameBuild()
{
    Byte frame[MAX_SFP_FRAME];
    sfpFrame *sf = (sfpFrame *)frame;

    QFETCH(Byte, Length);
    QFETCH(Byte, Sync);
    QFETCH(Byte, Pid);
    QFETCH(Byte, PayloadSize);
    QFETCH(Byte*, Payload);
    QFETCH(Byte, Sum);
    QFETCH(Byte, SumSum);

    buildSfpFrame(PayloadSize, Payload, Pid, sf);

//    qDebug() << sf->length << sf->sync << sf->pid << sf->payload[sizeof(zeros)] << sf->payload[sizeof(zeros)+1];

    QCOMPARE(sf->length,Length);
    QCOMPARE(sf->sync, Sync);
    QCOMPARE(sf->pid, Pid);
    QCOMPARE(sf->payload[sizeof(zeros)], Sum);
    QCOMPARE(sf->payload[sizeof(zeros)+1], SumSum);
}
