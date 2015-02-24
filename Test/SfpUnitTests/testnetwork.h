#ifndef TESTNETWORK_H
#define TESTNETWORK_H

#include <QObject>

class TestNetwork : public QObject
{
    Q_OBJECT
public:
    explicit TestNetwork(QObject *parent = 0);

signals:

public slots:

};

#endif // TESTNETWORK_H
