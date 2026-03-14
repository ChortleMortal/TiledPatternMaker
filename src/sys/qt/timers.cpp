#include <QDebug>

#include "sys/qt/timers.h"

AQElapsedTimer::AQElapsedTimer(bool bstart) : QElapsedTimer()
{
    if (bstart)
    {
        start();
    }
}

QString AQElapsedTimer::getElapsed()
{
    qreal time    = elapsed();
    double qdelta = time /1000.0;
    QString sDelta = QString("%1").arg(qdelta, 8, 'f', 3);
    return sDelta;
}

int AQElapsedTimer::getElapsedSeconds()
{
    qreal time    = elapsed();
    double qdelta = time /1000.0;
    return qRound(qdelta);
}

void TimerResults::add(VersionedName  name, int seconds)
{
    QMutexLocker  locker(&mutex);
    insert(seconds,name.get());
}


void TimerResults::dump()
{
    qInfo() << "=============================== RESULTS START";
    for (auto it = begin(); it != end(); it++)
    {
        qInfo() << it.key() << it.value();
    }
    qInfo() << "=============================== RESULTS END";
}