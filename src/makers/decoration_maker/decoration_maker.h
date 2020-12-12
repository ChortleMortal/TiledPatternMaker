#ifndef DECORATIONMAKER_H
#define DECORATIONMAKER_H

#include "base/shared.h"
#include "panels/panel_misc.h"
#include "style/style.h"
#include "base/configuration.h"

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

    void           selectStyle(StylePtr sp) { selectedStyle = sp; }
    StylePtr       getSelectedStyle()  { return selectedStyle; }

    StylePtr       makeStyle(eStyleType type, StylePtr oldStyle);

    void           setCurrentEditor(StylePtr style, AQTableWidget * table, QVBoxLayout * vbox);
    StyleEditorPtr currentEditor() { return  currentStyleEditor; }

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

    StyleEditorPtr  currentStyleEditor;
    StylePtr        selectedStyle;
    MosaicPtr       mosaic;

};

#endif
