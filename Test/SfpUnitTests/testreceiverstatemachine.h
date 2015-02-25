#ifndef TESTRECEIVERSTATEMACHINE_H
#define TESTRECEIVERSTATEMACHINE_H

#include <QObject>

class TestReceiverStateMachine : public QObject
{
    Q_OBJECT
public:
    explicit TestReceiverStateMachine(QObject *parent = 0);

private slots:
    void TestHunting();
};

#endif // TESTRECEIVERSTATEMACHINE_H
