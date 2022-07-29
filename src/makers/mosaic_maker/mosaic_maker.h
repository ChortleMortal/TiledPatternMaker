#ifndef MOSAICMAKER_H
#define MOSAICMAKER_H

#include <memory>
#include "enums/estyletype.h"
#include "enums/estatemachineevent.h"
#include "settings/model_settings.h"

typedef std::shared_ptr<class TilingMaker>      TilingMakerPtr;
typedef std::shared_ptr<class Mosaic>           MosaicPtr;
typedef std::shared_ptr<class Prototype>        PrototypePtr;
typedef std::shared_ptr<class Style>            StylePtr;

class MosaicMaker
{
public:
    static MosaicMaker * getInstance();

    void           init();

    void           takeDown(MosaicPtr mosaic);
    void           sm_takeUp(QVector<PrototypePtr> prototypes, eSM_Event mode);

    MosaicPtr      getMosaic();
    void           resetMosaic();
    ModelSettings& getMosaicSettings();

    StylePtr       makeStyle(eStyleType type, StylePtr oldStyle);

protected:
    void sm_resetStyles();
    void sm_createMosaic(const QVector<PrototypePtr> prototypes);
    void sm_addPrototype(const QVector<PrototypePtr> prototypes);
    void sm_replacePrototype(PrototypePtr prototype);

private:
    MosaicMaker();
    static MosaicMaker * mpThis;

    class MotifMaker  * motifMaker;
    class ViewControl * viewControl;
    TilingMakerPtr      tilingMaker;

    MosaicPtr           _mosaic;
};

#endif
