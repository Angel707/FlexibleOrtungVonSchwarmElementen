#ifndef CAMSYNCTHREAD_H
#define CAMSYNCTHREAD_H

#include <QThread>

class CamSyncThread : public QThread
{
    Q_OBJECT
public:
    explicit CamSyncThread(QObject *parent = 0);

signals:

public slots:

};

#endif // CAMSYNCTHREAD_H
