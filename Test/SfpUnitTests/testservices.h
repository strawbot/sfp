#ifndef TESTSERVICES_H
#define TESTSERVICES_H

#include <QObject>

class TestServices : public QObject
{
    Q_OBJECT
public:
    explicit TestServices(QObject *parent = 0);

private slots:
    void TestSendNpsNoLink(void);
    void TestNpsLink(void);
    void TestNpsNull();
    void TestNpsOversize();
    void TestSendSpsNoLink(void);
    void TestSpsLink(void);
    void TestSpsNull();
    void TestSpsOversize();
};

#endif // TESTSERVICES_H
