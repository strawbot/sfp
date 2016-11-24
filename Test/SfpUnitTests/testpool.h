#ifndef TESTPOOL_H
#define TESTPOOL_H

#include <QObject>

class TestPool : public QObject
{
    Q_OBJECT
public:
    explicit TestPool(QObject *parent = 0);

private slots:
    void init();
    void initTestCase();
    void TestGetFrame();
    void TestPoolEmpty();
    void TestreturnFrame();
    void TestIgetFrame();
    void TestIreturnFrame();
};

#endif // TESTPOOL_H
