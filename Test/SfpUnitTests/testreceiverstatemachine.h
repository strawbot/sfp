#ifndef TESTRECEIVERSTATEMACHINE_H
#define TESTRECEIVERSTATEMACHINE_H

#include <QObject>

class TestReceiverStateMachine : public QObject
{
    Q_OBJECT
public:
    explicit TestReceiverStateMachine(QObject *parent = 0);

private slots:
    void TestAcquiring();
    void TestHunting();
    void TestSyncing();
    void TestReceiving();
    void TestOverflow();
};

#endif // TESTRECEIVERSTATEMACHINE_H
