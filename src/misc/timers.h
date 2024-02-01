#pragma once
#ifndef AQELAPSEDTIMER_H
#define AQELAPSEDTIMER_H

#include <QElapsedTimer>

class AQElapsedTimer : public QElapsedTimer
{
public:
    AQElapsedTimer(bool bstart = true);

    QString getElapsed();
};

#endif // AQELAPSEDTIMER_H
