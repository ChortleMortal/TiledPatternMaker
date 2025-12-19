#pragma once
#ifndef PAGE_LAYER_ALIGNMENT_H
#define PAGE_LAYER_ALIGNMENT_H

#include "gui/panels/panel_page.h"

typedef std::shared_ptr<class Layer>   LayerPtr;
typedef std::weak_ptr<class Layer>     WeakLayerPtr;

class AQDoubleSpinBox;
class AQTableWidget;
class EdgePoly;

class page_layer_algnment : public panel_page
{
    enum eLayerCol
    {
        LAYER_NAME,
        LAYER_ACTIVE,
        LAYER_SMX,
        LAYER_BREAKAWAY,
        LAYER_LOCKED,
        LAYER_SOLO,
        LAYER_MODEL_XFORM,
        NUM_LAYER_COLS
    };

    enum eTransformDisplay
    {
        DISP_XFORM,     // model
        DISP_XFORM_HASH,
        DISP_MODEL,
        DISP_CANVAS,
        DISP_LAYER
    };

public:
    page_layer_algnment(ControlPanel * cpanel);

    void onEnter()          override;
    void onExit()           override {}
    void onRefresh()        override;

private slots:
    void slot_dispTypeChanged(int);
    void slot_selectCell();

protected:
    void doRefresh();
    void populateLayers();
    void populateLayer(Layer *layer, int col, bool active, bool primary);

private:
    AQTableWidget     * layerTable;
    QComboBox         * qcb;
    eTransformDisplay   transformDisplay;
};

#endif
