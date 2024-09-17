#ifndef STEPPING_ENGINE_H
#define STEPPING_ENGINE_H

#include <QObject>
#include "sys/sys/versioning.h"

struct sCycling
{
    VersionFileList files;
    QStringList     fileFilter;
    int             cIndex;
    int             cCount;     // 4 ticks
};

class ImageEngine;

class SteppingEngine : public QObject
{
    Q_OBJECT

public:
    SteppingEngine(ImageEngine * parent);

    virtual bool begin() = 0;
    virtual bool tick()  = 0;
    virtual bool next()  = 0;
    virtual bool prev()  = 0;
    virtual bool end()   = 0;

    static bool isRunning() { return (runCount > 0); }

    static void pause()           { paused = !paused; }
    static void setPaused(bool p) { paused = p; }
    static bool isPaused()        { return paused; }

           bool isStarted()       { return started; }

protected:
    class Configuration * config;
    class ControlPanel  * panel;

    void  start(bool enb);

    void  finish(QString name);

    sCycling cydata;        // cycle data

    ImageEngine * parent;

private:
    bool        started;
    static bool paused;
    static int  runCount;
};

#endif // STEPPING_ENGINE_H


