#pragma once
#ifndef PROTOTYPE_MAKER_H
#define PROTOTYPE_MAKER_H

#include <QObject>
#include <QFrame>
#include "enums/estatemachineevent.h"
#include "makers/prototype_maker/prototype_data.h"

class PrototypeMaker : public QObject
{
    Q_OBJECT

    enum eMMState
    {
        MM_EMPTY,
        MM_SINGLE,
        MM_MULTI
    };

public:
    static PrototypeMaker * getInstance();
    static void             releaseInstance();

    void         init();
    void         unload();

    // state machine
    void         sm_takeDown(QVector<ProtoPtr> &prototypes);
    void         sm_takeUp(const TilingPtr & tiling, ePROM_Event event, const TilePtr tile = TilePtr());

    void         setPropagate(bool val) { propagate = val; }

    // operations
    bool         duplicateDesignElement();
    MotifPtr     duplicateMotif(MotifPtr motif);

    void         erasePrototypes();
    
    PrototypeData *  getProtoMakerData() { return &protoData; }

signals:

protected:
    eMMState    sm_getState();
    void        sm_eraseAllAdd(const ProtoPtr & prototype);
    void        sm_eraseCurrentAdd(const ProtoPtr & prototype);
    void        sm_Add(const ProtoPtr & prototype);
    void        sm_replaceTiling(const ProtoPtr & prototype,const TilingPtr & tiling);
    void        sm_resetMaps();

    ProtoPtr    createPrototype(const TilingPtr & tiling);
    void        recreatePrototypeFromTiling(const TilingPtr & tiling, ProtoPtr prototype);
    bool        askNewProto();

private:
    PrototypeMaker();

    static PrototypeMaker   * mpThis;

    class Configuration     * config;
    class TiledPatternMaker * maker;
    class TilingMaker       * tilingMaker;
    class MosaicMaker       * mosaicMaker;
    class ViewControl       * vcontrol;

    bool                      propagate;
    
    PrototypeData             protoData;
};

#endif
