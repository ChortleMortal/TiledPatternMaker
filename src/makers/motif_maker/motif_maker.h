#ifndef MOTIF_MAKER_H
#define MOTIF_MAKER_H

#include <QObject>
#include <QFrame>
#include "misc/unique_qvector.h"
#include "enums/estatemachineevent.h"

class page_motif_maker;
class ViewControl;
class TiledPatternMaker;

typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class TilingMaker>      TilingMakerPtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;
typedef std::shared_ptr<class Prototype>        PrototypePtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Tile>             TilePtr;

enum eMMState
{
    MM_EMPTY,
    MM_SINGLE,
    MM_MULTI
};


class MotifMaker : public QObject
{
    Q_OBJECT

public:
    static MotifMaker * getInstance();

    void         init();
    void         unload();

    // state machine
    void         sm_takeDown(const PrototypePtr & prototype);
    void         sm_takeUp(const TilingPtr & tiling, eSM_Event event);

    // setters.getters
    const QVector<PrototypePtr> & getPrototypes();
    PrototypePtr        findPrototypeByName(const TilingPtr & tiling);
    PrototypePtr        getSelectedPrototype();
    void                setSelectedPrototype(const PrototypePtr & pp);
    void                resetSelectedPrototype() { _selectedPrototype.reset(); _selectedDesignElement.reset(); }

    QVector<DesignElementPtr> getSelectedDesignElements() { return _selectedDesignElements; }
    DesignElementPtr    getSelectedDesignElement() { return _selectedDesignElement; }
    void                setSelectedDesignElement(const DesignElementPtr & del);

    void                setActiveTile(const TilePtr & tile) { _activeTile = tile; }
    TilePtr             getActiveTile() { return _activeTile; }

    void                setPropagate(bool val) { propagate = val; }

    // operations
    bool                dupolicateMotif();

    void                erasePrototypes();
    void                removePrototype(TilingPtr tiling);
    void                deleteActiveTile();

signals:
    void                sig_tilingChoicesChanged();
    void                sig_tilingChanged();
    void                sig_tileChanged();

protected:
    eMMState            sm_getState();
    void                sm_eraseAllCreateAdd(const TilingPtr & tiling);
    void                sm_eraseCurrentCreateAdd(const TilingPtr & tiling);
    void                sm_createAdd(const TilingPtr & tiling);
    void                sm_replaceTiling(const PrototypePtr & prototype,const TilingPtr & tiling);
    void                sm_resetMaps();

    PrototypePtr        createPrototype(const TilingPtr & tiling);
    void                recreatePrototype(const TilingPtr & tiling);
    void                recreateMotifs(const TilingPtr & tiling);
    bool                askNewProto();

private:
    MotifMaker();

    static MotifMaker * mpThis;

    class Configuration    * config;
    ViewControl            * vcontrol;
    TiledPatternMaker      * maker;
    TilingMakerPtr           tilingMaker;
    class MosaicMaker      * mosaicMaker;

    UniqueQVector<PrototypePtr>     _prototypes;
    PrototypePtr                    _selectedPrototype;
    UniqueQVector<DesignElementPtr> _selectedDesignElements;
    DesignElementPtr                _selectedDesignElement;
    TilePtr                         _activeTile;

    bool                    propagate;
};

#endif
