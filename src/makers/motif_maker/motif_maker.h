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
typedef std::shared_ptr<class Feature>          FeaturePtr;

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

    void         takeDown(const PrototypePtr & prototype);
    void         sm_take(const TilingPtr & tiling, eSM_Event event);

    void         erasePrototypes();
    void         removePrototype(TilingPtr tiling);

    const QVector<PrototypePtr> & getPrototypes();

    PrototypePtr findPrototypeByName(const TilingPtr & tiling);
    PrototypePtr getSelectedPrototype();
    void         setSelectedPrototype(const PrototypePtr & pp);
    void         resetSelectedPrototype() { _selectedPrototype.reset(); _selectedDesignElement.reset(); }

    QVector<DesignElementPtr> getSelectedDesignElements() { return _selectedDesignElements; }

    DesignElementPtr getSelectedDesignElement() { return _selectedDesignElement; }
    void             setSelectedDesignElement(const DesignElementPtr & del);

    void         duplicateActiveFeature();
    void         deleteActiveFeature();

    MapPtr createExplicitGirihMap(int starSides, qreal starSkip);
    MapPtr createExplicitHourglassMap(qreal d, int s);
    MapPtr createExplicitInferredMap();
    MapPtr createExplicitIntersectMap(int starSides, qreal starSkip, int s, bool progressive);
    MapPtr createExplicitRosetteMap(qreal q, int s, qreal r);
    MapPtr createExplicitStarMap(qreal d, int s);
    MapPtr createExplicitFeatureMap();

    void   setActiveFeature(const FeaturePtr & feature) { _activeFeature = feature; }
    FeaturePtr getActiveFeature() { return _activeFeature; }

signals:
    void    sig_tilingChoicesChanged();
    void    sig_tilingChanged();
    void    sig_featureChanged();

protected:
    eMMState sm_getState();
    void     sm_eraseAllCreateAdd(const TilingPtr & tiling);
    void     sm_eraseCurrentCreateAdd(const TilingPtr & tiling);
    void     sm_createAdd(const TilingPtr & tiling);
    void     sm_replaceTiling(const PrototypePtr & prototype,const TilingPtr & tiling);
    void     sm_resetMaps();

    PrototypePtr createPrototype(const TilingPtr & tiling);
    void         recreatePrototype(const TilingPtr & tiling);
    void         recreateFigures(const TilingPtr & tiling);
    bool         askNewProto();

private:
    MotifMaker();

    static MotifMaker * mpThis;

    class Configuration    * config;
    ViewControl            * vcontrol;
    TiledPatternMaker      * maker;
    TilingMakerPtr           tilingMaker;
    class MosaicMaker      * mosaicMaker;
    MapPtr                   nullMap;

    UniqueQVector<PrototypePtr>     _prototypes;
    PrototypePtr                    _selectedPrototype;
    UniqueQVector<DesignElementPtr> _selectedDesignElements;
    DesignElementPtr                _selectedDesignElement;
    FeaturePtr                      _activeFeature;

};

#endif
