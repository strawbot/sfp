#ifndef TESTSPI_H
#define TESTSPI_H

#include <QObject>

class TestSpi : public QObject
{
    Q_OBJECT
public:
    explicit TestSpi(QObject *parent = 0);

private slots:
    void TestOneSpiLink();
    void TestSpiPoll();
};

#endif // TESTSPI_H
