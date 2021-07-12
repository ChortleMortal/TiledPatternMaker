/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VIEW_CONTROL_H
#define VIEW_CONTROL_H

#include "settings/filldata.h"
#include "geometry/xform.h"
#include "enums/eviewtype.h"

typedef std::weak_ptr<class Feature>            WeakFeaturePtr;
typedef std::shared_ptr<class BackgroundImage>  BkgdImgPtr;
typedef std::shared_ptr<class Border>           BorderPtr;
typedef std::shared_ptr<class Feature>          FeaturePtr;
typedef std::shared_ptr<class MapEditor>        MapEditorPtr;
typedef std::shared_ptr<class ModelSettings>    ModelSettingsPtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class TilingView>       TilingViewPtr;
typedef std::shared_ptr<class FigureView>       FigureViewPtr;
typedef std::shared_ptr<class PrototypeView>    PrototypeViewPtr;
typedef std::shared_ptr<class TilingMaker>      TilingMakerPtr;
typedef std::shared_ptr<class Grid>             GridPtr;
typedef std::shared_ptr<class ImageLayer>       ImgLayerPtr;

class ViewControl : public QObject
{
    Q_OBJECT

public:
    static ViewControl  * getInstance();
    static void         releaseInstance();

    void    init();
    void    resetAllMakers();
    void    viewEnable(eViewType view, bool enable);
    void    disableAllViews();

    void    addImage(ImgLayerPtr image) { images.push_back(image); }
    void    removeAllImages()           { images.clear(); }
    void    removeImage(ImageLayer * img);

    // selections
    void        selectFeature(WeakFeaturePtr fp);
    FeaturePtr  getSelectedFeature();

    void        setFillData(FillData & fd) { fillData = fd; }
    FillData &  getFillData() { return fillData; }

    const Xform & getCurrentXform();

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

    void     setTitle(TilingPtr tp);
    void     setBorder(BorderPtr bp);

    ModelSettingsPtr getMosaicOrTilingSettings();

signals:
    void    sig_selected_dele_changed();
    void    sig_viewUpdated();

public slots:
    void    slot_updateView();
    void    slot_refreshView();
    void    slot_clearView();
    void    slot_clearMakers();

private:
    static ViewControl      * mpThis;

    ViewControl();
    ~ViewControl();

    class   View            * view;
    class   Configuration   * config;
    class   ControlPanel    * panel;
    class   DesignMaker     * designMaker;
    class   DecorationMaker * decorationMaker;
    class   MotifMaker      * motifMaker;

    TilingMakerPtr            tilingMaker;
    MapEditorPtr              mapEditor;
    TilingViewPtr             tilingView;
    PrototypeViewPtr          pfview;
    FigureViewPtr             figView;
    GridPtr                   gridView;
    BkgdImgPtr                imageView;

    bool                    enabledViews[VIEW_MAX+1];
    QVector<ImgLayerPtr>    images;
    WeakFeaturePtr          selectedFeature;
    FillData                fillData;
    const Xform             unityXform;
};

#endif
