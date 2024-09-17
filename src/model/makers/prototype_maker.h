#pragma once
#ifndef PROTOTYPE_MAKER_H
#define PROTOTYPE_MAKER_H

#include <QObject>
#include <QFrame>
#include "sys/enums/estatemachineevent.h"
#include "model/prototypes/prototype_data.h"

class PrototypeMaker : public QObject, public ProtoMakerData
{
    Q_OBJECT

    enum eMMState
    {
        MM_EMPTY,
        MM_SINGLE,
        MM_MULTI
    };

public:
    PrototypeMaker();

    void        init();
    void        unload();
    void        setPropagate(bool val);

    // state machine
    void        sm_takeDown(QVector<ProtoPtr> &prototypes);
    void        sm_takeUp(const TilingPtr & tiling, ePROM_Event event, const TilePtr tile = TilePtr());
    void        sm_resetMaps();

    // operations
    bool        duplicateDesignElement();
    MotifPtr    duplicateMotif(MotifPtr motif);
    void        selectDesignElement(DesignElementPtr delp);
    void        erasePrototypes();
    
signals:
    void        sig_updateView();

protected:
    eMMState    sm_getState();
    void        sm_eraseAllAdd(const ProtoPtr & prototype);
    void        sm_eraseCurrentAdd(const ProtoPtr & prototype);
    void        sm_Add(const ProtoPtr & prototype);
    void        sm_replaceTiling(const ProtoPtr & prototype,const TilingPtr & tiling);
    void        sm_buildMaps();

    ProtoPtr    createPrototypeFromTililing(const TilingPtr & tiling);
    void        recreatePrototypeFromTiling(const TilingPtr & tiling, ProtoPtr prototype);
    bool        askNewProto();

private:
    class Configuration     * config;
    class TilingMaker       * tilingMaker;
    class MosaicMaker       * mosaicMaker;
    class ViewController    * vcontrol;

    bool                      propagate;
};

#endif
