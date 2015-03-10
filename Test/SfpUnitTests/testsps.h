#ifndef TESTSPS_H
#define TESTSPS_H

#include <QObject>

class TestSps : public QObject
{
    Q_OBJECT
public:
    explicit TestSps(QObject *parent = 0);

private slots:
    void TestInitSps();
    void TestAckDropped();
    void TestSpsDropped();
};

#endif // TESTSPS_H
