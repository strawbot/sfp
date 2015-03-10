#ifndef TESTROUTING_H
#define TESTROUTING_H

#include <QObject>

class TestRouting : public QObject
{
    Q_OBJECT
public:
    explicit TestRouting(QObject *parent = 0);

private slots:
    void TestInitRoutes();
    void TestNoRoutes();
    void TestRoutes();
    void TestNoDest();
};

/*
 * route to self, nps, sps
 * route to non existant node, nps, sps
 * route between 4 nodes; one with two links
 */
#endif // TESTROUTING_H
