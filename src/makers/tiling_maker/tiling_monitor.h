#ifndef TILINGMONITOR_H
#define TILINGMONITOR_H

#include <QThread>
#include <QMutex>

#include "tile/tiling.h"

typedef std::weak_ptr<class Tiling>     WeakTilingPtr;
typedef std::shared_ptr<class Tiling>   TilingPtr;

class TilingMaker;

class TilingMonitor : public QThread
{
    Q_OBJECT

public:
    static TilingMonitor * getInstance();

    void   run() override;

    bool    hasChanged(TilingPtr tp);

public slots:
    void    slot_tileChanged();
    void    slot_tilingChanged();
    void    slot_monitor(bool reset);

signals:
    void  sig_update();

protected:
    TilingMonitor();

    void determineOverlapsAndTouching();

private:
    static TilingMonitor*   mpThis;

    TilingMaker *           tilingMaker;
    bool                    doit;

    WeakTilingPtr           wTiling;
    bool                    tileChanged;
};

#endif // TILINGMONITOR_H
