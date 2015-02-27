#ifndef TESTTRANSMITTERSTATEMACHINE_H
#define TESTTRANSMITTERSTATEMACHINE_H

#include <QObject>

class TestTransmitterStateMachine : public QObject
{
    Q_OBJECT
public:
    explicit TestTransmitterStateMachine(QObject *parent = 0);

private slots:
    void TestSendFrame();
    void TestSendAck();
    void TestSendPoll();
    void TestSpsInit();
};

#endif // TESTTRANSMITTERSTATEMACHINE_H
