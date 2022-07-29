#ifndef AQELAPSEDTIMER_H
#define AQELAPSEDTIMER_H

#include <QElapsedTimer>

class AQElapsedTimer : public QElapsedTimer
{
public:
    AQElapsedTimer();

    QString getElapsed();
};

#endif // AQELAPSEDTIMER_H
