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

#include "base/shared.h"
#include "base/filldata.h"
#include "base/model_settings.h"
#include "base/configuration.h"

enum eProtoViewMode
{
    PROTO_DRAW_MAP       =  0x01,
    PROTO_DRAW_FEATURES  =  0x02,
    PROTO_DRAW_FIGURES   =  0x04
};

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

    // selections
    void                selectFeature(WeakFeaturePtr fp);
    FeaturePtr          getSelectedFeature();

    void                setProtoViewMode(eProtoViewMode mode, bool enb);
    int                 getProtoViewMode() {  return  protoViewMode; }

    void                setFillData(FillData & fd) { fillData = fd; }
    FillData          & getFillData() { return fillData; }

protected:
    void     setupViewers();
    void     refreshView();

    void     viewDesign();
    void     viewMosaic();
    void     viewPrototype();
    void     viewDesignElement();
    void     viewMotifMaker();
    void     viewTiling();
    void     viewTilingMaker();
    void     viewMapEditor();
    void     viewFaceSet();

    void     setTitle(TilingPtr tp);
    void     setBackgroundImg(BkgdImgPtr bkgd);
    void     setBorder(BorderPtr bp);

    ModelSettingsPtr getMosaicOrTilingSettings();

signals:
    void    sig_selected_dele_changed();
    void    sig_selected_proto_changed();
    void    sig_viewUpdated();

public slots:
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
    class   TilingMaker     * tilingMaker;

    bool    enabledViews[VIEW_MAX+1];

    WeakFeaturePtr              selectedFeature;

    FillData                    fillData;

    int                         protoViewMode;
};

#endif
