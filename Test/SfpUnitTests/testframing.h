#ifndef TESTFRAMING_H
#define TESTFRAMING_H

#include "bktypes.h"

#include <QObject>

class TestFraming : public QObject
{
    Q_OBJECT
public:
    explicit TestFraming(QObject *parent = 0);

private slots:
    void TestCheckSum_data();
    void TestCheckSum();
    void TestFrameBuild_data();
    void TestFrameBuild();
};

#endif // TESTFRAMING_H
