#ifndef TESTNETWORK_H
#define TESTNETWORK_H

#include <QObject>

class TestNetwork : public QObject
{
    Q_OBJECT
public:
    explicit TestNetwork(QObject *parent = 0);

private slots:
    void initTestCase();
    void testPing();
    void testSpsDirect();
    void testSpsInDirect();
};

#endif // TESTNETWORK_H
