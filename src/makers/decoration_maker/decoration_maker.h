#ifndef DECORATIONMAKER_H
#define DECORATIONMAKER_H

#include "base/shared.h"
#include "base/configuration.h"
#include "style/style.h"

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
    void sm_resetStyles(QVector<PrototypePtr> prototypes);
    void sm_createMosaic(const QVector<PrototypePtr> prototypes);
    void sm_addPrototype(const QVector<PrototypePtr> prototypes);
    void sm_replacePrototype(PrototypePtr prototype);

private:
    DecorationMaker();
    static DecorationMaker * mpThis;

    class MotifMaker  * motifMaker;
    class TilingMaker * tilingMaker;
    class ViewControl * viewControl;

    MosaicPtr       mosaic;

};

#endif
