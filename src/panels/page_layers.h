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

#ifndef PAGE_LAYERS_H
#define PAGE_LAYERS_H

#include "panels/panel_page.h"

typedef std::shared_ptr<class Layer>   LayerPtr;
typedef std::weak_ptr<class Layer>     WeakLayerPtr;

class AQDoubleSpinBox;
class EdgePoly;
class AQTableWidget;

class page_layers : public panel_page
{
    Q_OBJECT

    enum eLayerRow
    {
        LAYER_NAME,
        LAYER_VISIBILITY,
        LAYER_Z,
        LAYER_ALIGN,
        FRAME_SCALE,
        FRAME_ROT,
        FRAME_X,
        FRAME_Y,
        CANVAS_CLEAR,
        CANVAS_SCALE,
        CANVAS_ROT,
        CANVAS_X,
        CANVAS_Y,
        CANVAS_CENTER_X,
        CANVAS_CENTER_Y,
        LAYER_CENTER,
        LAYER_SCALE,
        LAYER_ROT,
        LAYER_X,
        LAYER_Y,
        SUB_LAYERS,
        NUM_LAYER_ROWS
    };

public:
    page_layers(ControlPanel * cpanel);

    void onEnter() override;
    void onExit() override {}
    void refreshPage() override;

private slots:
    void refreshCanvas();
    void slot_selectLayer();

protected:
    void populateLayers();
    void populateLayer(LayerPtr layer, int col);
    LayerPtr getLayer(int col);

    void visibilityChanged(int col);
    void zChanged(AQDoubleSpinBox *dsp, int col);
    void alignPressed(int col);
    void slot_set_deltas(int col);
    void clear_deltas(int col);

private:
    AQTableWidget * layerTable;
};

#endif
