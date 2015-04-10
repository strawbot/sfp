#ifndef TESTTALK_H
#define TESTTALK_H

#include <QObject>

class TestTalk : public QObject
{
    Q_OBJECT
public:
    explicit TestTalk(QObject *parent = 0);

private slots:
    void TestKeyin();
};

#endif // TESTTALK_H
