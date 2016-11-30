#ifndef TESTHANDLERS_H
#define TESTHANDLERS_H

#include <QObject>

class TestHandlers : public QObject
{
    Q_OBJECT
public:
    explicit TestHandlers(QObject *parent = 0);

private slots:
    void init();
    void TestGetHandler();
    void TestSetHandler();
    void TestPacketIn();
    void TestPacketRetry();
    void TestStaleFrame();
    void TestAckPacket();
    void TestSpsAckRequest();
    void TestLinkLevelFrame();
    void TestNoHandler();
};

#endif // TESTHANDLERS_H
