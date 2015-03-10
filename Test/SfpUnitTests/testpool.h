#ifndef TESTPOOL_H
#define TESTPOOL_H

#include <QObject>

class TestPool : public QObject
{
    Q_OBJECT
public:
    explicit TestPool(QObject *parent = 0);

private slots:
    void initTestCase();
    void TestGetFrame();
    void TestInitPoolEmpty();
    void TestReturnFrame();
};

#endif // TESTPOOL_H
