#pragma once
#ifndef VIEW_CONTROL_H
#define VIEW_CONTROL_H

#include "model/settings/canvas.h"
#include "sys/geometry/xform.h"
#include "sys/enums/eviewtype.h"
#include "gui/top/view.h"
#include "sys/qt/unique_qvector.h"

typedef std::shared_ptr<class Border>           BorderPtr;
typedef std::shared_ptr<class LegacyBorder>     LegacyBorderPtr;
typedef std::shared_ptr<class Tile>             TilePtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class ImageLayerView>   ImgLayerPtr;

class ViewController : public QObject
{
    Q_OBJECT

    friend class PanelViewSelect;

public:
    ViewController();
    virtual ~ViewController();

    void    init(View * view);
    void    unloadMakers();

    bool    isEnabled(eViewType view);  // Borders and crops are excluded
    uint    enabledPrimaryViews();
    void    disablePrimeViews();
    void    disableAllViews();

    eViewType getMostRecent()           { return mostRecentPrimeView; }

    void    addImage(ImgLayerPtr image) { images.push_back(image); }
    bool    hasImages()                 { return (images.size() > 0);}
    void    removeAllImages()           { images.clear(); }
    void    removeImage(ImageLayerView * img);
    void    procImgViewKey(QKeyEvent * k);

    const   Xform & getCurrentModelXform();
    void    setCurrentModelXform(const Xform & xform, bool update);     // use with care

    QString getBackgroundColor(eViewType vtype);
    void    setBackgroundColor(eViewType vtype, QColor color);
    void    setBackgroundColor(eViewType vtype);

    void    setSize(QSize sz);
    void    setFixedSize(QSize);

    void    clearLayout();

    Canvas &  getCanvas()               { return canvas; }

    bool    viewCanPaint();
    bool    splashCanPaint();

public slots:
    void    slot_reconstructView();
    void    slot_unloadView();
    void    slot_unloadAll();

private:
    void    reconstructView();
    void    setMostRecent(eViewType viewType);
    void    setupEnabledViewLayers();

    bool    isPrimaryView(eViewType viewType);

    void    viewEnable(eViewType view, bool enable);

    void    viewDesign();
    void    viewMosaic();
    void    viewPrototype();
    void    viewMotifMaker();
    void    viewTiling();
    void    viewTilingMaker();
    void    viewMapEditor();
    void    viewBackgroundImage();
    void    viewBMPImages();
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
