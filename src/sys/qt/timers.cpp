#include "sys/qt/timers.h"
#include <QString>

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
