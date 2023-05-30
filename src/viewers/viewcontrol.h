#pragma once
#ifndef VIEW_CONTROL_H
#define VIEW_CONTROL_H

#include "settings/filldata.h"
#include "geometry/xform.h"
#include "enums/eviewtype.h"
#include "viewers/view.h"

typedef std::shared_ptr<class Border>           BorderPtr;
typedef std::shared_ptr<class LegacyBorder>     LegacyBorderPtr;
typedef std::shared_ptr<class Tile>             TilePtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class ImageLayer>       ImgLayerPtr;

class ViewControl : public View
{
    Q_OBJECT

public:
    static ViewControl  * getInstance();
    static void           releaseInstance();

    void    init();
    void    viewEnable(eViewType view, bool enable);
    bool    isEnabled(eViewType view)   { return enabledViews[view]; }
    void    disableAllViews();

    void    addImage(ImgLayerPtr image) { images.push_back(image); }
    void    removeAllImages()           { images.clear(); }
    void    removeImage(ImageLayer * img);

    void    setFillData(const FillData & fd) { fillData = fd; }
    const   FillData &  getFillData()        { return fillData; }

    const   Xform & getCurrentXform();
    void    setCurrentXform(const Xform & xform);     // use with care

protected:
    void     setupViewers();
    void     refreshView();

    void     viewDesign();
    void     viewMosaic();
    void     viewPrototype();
    void     viewMotifMaker();
    void     viewTiling();
    void     viewTilingMaker();
    void     viewMapEditor();

signals:
    void    sig_viewUpdated();
    void    sig_identifyYourself();

public slots:
    void    slot_refreshView() override;
    void    slot_updateView();
    void    slot_unloadView();
    void    slot_dontPaint(bool enb);
    void    slot_unloadAll();

private:
    ViewControl();
    ~ViewControl();

    static  ViewControl     * mpThis;
    class   Configuration   * config;
    class   ControlPanel    * panel;
    class   DesignMaker     * designMaker;
    class   MosaicMaker     * mosaicMaker;
    class   PrototypeMaker  * prototypeMaker;

    class TilingMakerView   * tilingMakerView;
    class TilingView        * tilingView;
    class PrototypeView     * prototypeView;
    class MotifView         * motifView;
    class MapEditorView     * mapedView;
    class MeasureView       * measureView;
    class GridView          * gridView;
    class CropViewer        * cropViewer;
    class BorderView        * borderView;

    bool                    dontPaint;
    bool                    enabledViews[VIEW_MAX+1];
    QVector<ImgLayerPtr>    images;
    FillData                fillData;
    const Xform             unityXform;
};

#endif
