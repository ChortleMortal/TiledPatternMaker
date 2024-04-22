#pragma once
#ifndef VIEW_CONTROL_H
#define VIEW_CONTROL_H

#include "settings/canvas.h"
#include "geometry/xform.h"
#include "enums/eviewtype.h"
#include "viewers/view.h"
#include "misc/unique_qvector.h"

typedef std::shared_ptr<class Border>           BorderPtr;
typedef std::shared_ptr<class LegacyBorder>     LegacyBorderPtr;
typedef std::shared_ptr<class Tile>             TilePtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class ImageLayerView>   ImgLayerPtr;

class ViewController : public QObject
{
    Q_OBJECT

public:
    ViewController();
    virtual ~ViewController();

    void    init(View * view);
    void    unloadMakers();

    void    viewEnable(eViewType view, bool enable);
    bool    isEnabled(eViewType view);
    void    disablePrimeViews();
    void    disableAllViews();

    eViewType getMostRecent()           { return mostRecentPrimeView; }

    void    addImage(ImgLayerPtr image) { images.push_back(image); }
    void    removeAllImages()           { images.clear(); }
    void    removeImage(ImageLayerView * img);

    const   Xform & getCurrentModelXform();
    void    setCurrentModelXform(const Xform & xform, bool update);     // use with care

    QString getBackgroundColor(eViewType vtype);
    void    setBackgroundColor(eViewType vtype, QColor color);
    void    setBackgroundColor(eViewType vtype);

    Canvas &  getCanvas()               { return canvas; }

public slots:
    void    slot_reconstructView();
    void    slot_unloadView();
    void    slot_unloadAll();

protected:

private:
    void    refreshView();
    void    setMostRecent(eViewType viewType);
    void    setupEnabledViewLayers();

    void    viewDesign();
    void    viewMosaic();
    void    viewPrototype();
    void    viewMotifMaker();
    void    viewTiling();
    void    viewTilingMaker();
    void    viewMapEditor();
    void    viewBackgroundImage();
    void    viewGrid();
    void    viewDebug();

    class   Configuration       * config;
    class   ControlPanel        * panel;
    class   DesignMaker         * designMaker;
    class   MosaicMaker         * mosaicMaker;
    class   PrototypeMaker      * prototypeMaker;
    class   TilingMaker         * tilingMaker;

    class TilingMakerView       * tilingMakerView;
    class PrototypeView         * prototypeView;
    class MotifView             * motifView;
    class MapEditorView         * mapedView;
    class MeasureView           * measureView;
    class GridView              * gridView;
    class BackgroundImageView   * bimageView;

    View                        * theView;

    Canvas                      canvas;
    eViewType                   mostRecentPrimeView;
    UniqueQVector<eViewType>    enabledViewers;
    QVector<ImgLayerPtr>        images;
};

#endif
