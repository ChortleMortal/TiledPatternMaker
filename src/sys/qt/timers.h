#pragma once
#ifndef AQELAPSEDTIMER_H
#define AQELAPSEDTIMER_H

#include <QElapsedTimer>
#include <QMultiMap>
#include <QMutex>
#include <QString>
#include "sys/sys/versioning.h"

class AQElapsedTimer : public QElapsedTimer
{
public:
    AQElapsedTimer(bool bstart = true);

    QString getElapsed();
    int     getElapsedSeconds();
};

#endif // AQELAPSEDTIMER_H



class TimerResults : public QMultiMap<int,QString>
{
public:
    TimerResults() {}

    void add(VersionedName name, int seconds);
    void dump();

    QMutex mutex;

};