#pragma once
#ifndef PROTOTYPE_MAKER_H
#define PROTOTYPE_MAKER_H

#include <QObject>
#include <QFrame>
#include "sys/enums/estatemachineevent.h"
#include "model/makers/prototype_maker_data.h"

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

    // state machine
    void        sm_takeDown(QVector<ProtoPtr> &prototypes);
    void        sm_takeUp(ProtoEvent & protoEvent);
    void        sm_resetProtoMaps();
    void        sm_resetMotifMaps();

    // operations
    bool        duplicateDesignElement();
    MotifPtr    duplicateMotif(MotifPtr motif);
    void        selectDesignElement(DesignElementPtr delp);
    void        erasePrototypes();
    
    void        setWidget(MotifMakerWidget * widget) { motifMakerWidget = widget; }
    MotifMakerWidget * getWidget()                   { return motifMakerWidget; }

    inline bool forceWidgetRefresh()            { return _forceWidgetRefresh; }
    void        setForceWidgetRefresh(bool set) { _forceWidgetRefresh = set; }

signals:
    void        sig_updateView();

public slots:
    void        slot_propagateChanged(bool val);

protected:
    eMMState    sm_getState();
    void        sm_eraseAllAdd(const ProtoPtr & prototype);
    void        sm_eraseCurrentAdd(const ProtoPtr & prototype);
    void        sm_Add(const ProtoPtr & prototype);
    void        sm_replaceTiling(const ProtoPtr & prototype,const TilingPtr & tiling);
    ProtoPtr    sm_mergeProtos(const TilingPtr &tiling);
    void        sm_buildMaps();

    ProtoPtr    createPrototypeFromTililing(const TilingPtr & tiling);
    void        recreatePrototypeFromTiling(const TilingPtr & tiling, ProtoPtr prototype);
    bool        askNewProto();

private:
    void  setPropagate(bool enb)    { _pm_propagate = enb; }
    bool  getPropagate()            { return _pm_propagate; }

    class MosaicMaker       * mosaicMaker;

    MotifMakerWidget        * motifMakerWidget;

    bool                      _pm_propagate;
    bool                      _forceWidgetRefresh;
};

#endif
