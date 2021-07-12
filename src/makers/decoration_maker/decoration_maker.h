#ifndef DECORATIONMAKER_H
#define DECORATIONMAKER_H

#include <memory>
#include "enums/estyletype.h"
#include "enums/estatemachineevent.h"

typedef std::shared_ptr<class TilingMaker>      TilingMakerPtr;
typedef std::shared_ptr<class Mosaic>           MosaicPtr;
typedef std::shared_ptr<class Prototype>        PrototypePtr;
typedef std::shared_ptr<class Style>            StylePtr;
typedef std::shared_ptr<class ModelSettings>    ModelSettingsPtr;


class DecorationMaker
{
public:
    static DecorationMaker * getInstance();

    void           init();

    void           takeDown(MosaicPtr mosaic);
    void           sm_takeUp(QVector<PrototypePtr> prototypes, eSM_Event mode);

    MosaicPtr      getMosaic();
    void           resetMosaic();
    ModelSettingsPtr getMosaicSettings();

    StylePtr       makeStyle(eStyleType type, StylePtr oldStyle);

protected:
    void sm_resetStyles();
    void sm_createMosaic(const QVector<PrototypePtr> prototypes);
    void sm_addPrototype(const QVector<PrototypePtr> prototypes);
    void sm_replacePrototype(PrototypePtr prototype);

private:
    DecorationMaker();
    static DecorationMaker * mpThis;

    class MotifMaker  * motifMaker;
    TilingMakerPtr      tilingMaker;
    class ViewControl * viewControl;

    MosaicPtr       mosaic;

};

#endif
