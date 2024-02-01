#pragma once
#ifndef PAGE_LAYERS_H
#define PAGE_LAYERS_H

#include "panels/panel_page.h"

typedef std::shared_ptr<class Layer>   LayerPtr;
typedef std::weak_ptr<class Layer>     WeakLayerPtr;

class AQDoubleSpinBox;
class AQTableWidget;
class EdgePoly;

class page_layers : public panel_page
{
    Q_OBJECT

    enum eLayerRow
    {
        LAYER_NAME,
        LAYER_VISIBILITY,
        LAYER_Z,
        LAYER_ALIGN,
        LAYER_ALIGN_CENTER,
        CANVAS_SCALE,
        CANVAS_ROT,
        CANVAS_X,
        CANVAS_Y,
        MODEL_CLEAR,
        MODEL_SCALE,
        MODEL_ROT,
        MODEL_X,
        MODEL_Y,
        MODEL_CENTER_X,
        MODEL_CENTER_Y,
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
    void onExit() override;
    void onRefresh() override;

private slots:
    void slot_selectLayer();
    void slot_deSelectLayer();

protected:
    void doRefresh();
    void populateLayers();
    void populateLayer(Layer *layer, int col);
    void refreshLayer( Layer *layer, int col);

    Layer * getLayer(int col);

    void visibilityChanged(int col);
    void zChanged(AQDoubleSpinBox *dsp, int col);
    void alignPressed(int col);
    void alignCenterPressed(int col);
    void slot_set_deltas(int col);
    void slot_set_center(int col);
    void clear_deltas(int col);

private:
    AQTableWidget * layerTable;
    QLabel        * refreshLabel;
    QVector<Layer *> wlayers;
    QTableWidgetItem * selectedItem;
};

#endif
