#include <QHeaderView>
#include <QCheckBox>

#include "gui/panels/page_layer_alignmentl.h"
#include "gui/panels/panel_misc.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view.h"
#include "model/makers/mosaic_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/makers/tiling_maker.h"
#include "model/settings/configuration.h"
#include "model/styles/style.h"
#include "sys/sys.h"
#include "sys/geometry/transform.h"

using std::string;

page_layer_algnment::page_layer_algnment(ControlPanel * cpanel)  : panel_page(cpanel,PAGE_LAYER_STATUS,"Layer Align")
{
    qcb = new QComboBox();
    qcb->addItem("Model Xform",DISP_XFORM);
    qcb->addItem("Model Xform HASH",DISP_XFORM_HASH);
    qcb->addItem("Model Transform",DISP_MODEL);
    qcb->addItem("Canvas Transform",DISP_CANVAS);
    qcb->addItem("Layer Transform",DISP_LAYER);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(qcb);

    layerTable   = new AQTableWidget(this);
    layerTable->setColumnCount(NUM_LAYER_COLS);
    layerTable->setSelectionBehavior(QAbstractItemView::SelectItems);
    layerTable->setSelectionMode(QAbstractItemView::SingleSelection);

    QStringList qslH;
    qslH << "Layer" << "Visible" << "SYS" << "INDP" << "LOCK" << "SOLO" << "Transform";

    layerTable->setHorizontalHeaderLabels(qslH);
    layerTable->verticalHeader()->setVisible(false);
    layerTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layerTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    vbox->addLayout(hbox);
    vbox->addWidget(layerTable);
    vbox->addStretch();

    transformDisplay = DISP_XFORM;

    connect(qcb,        &QComboBox::currentIndexChanged,     this, &page_layer_algnment::slot_dispTypeChanged);
    connect(layerTable, &QTableWidget::itemSelectionChanged, this, &page_layer_algnment::slot_selectCell);
}

void page_layer_algnment::onEnter()
{
    populateLayers();
}

void page_layer_algnment::onRefresh()
{
    populateLayers();
}

void page_layer_algnment::populateLayers()
{
    int row = 0;

    layerTable->clearContents();
    layerTable->setRowCount(0);

    const QVector<Layer *> active = Sys::viewController->getActiveLayers();

    auto pri = viewControl->getSelectedPrimaryLayer();

    // populate mosaic styles
    auto mosaic = Sys::mosaicMaker->getMosaic();
    const StyleSet & sset = mosaic->getStyleSet();
    for (auto & style : sset)
    {
        Layer * layer = (Layer*)style.get();
        populateLayer(layer,row++,active.contains(layer),(style ==pri));
    }

    // populate other
    Layer * layer;
    layer = (Layer*)Sys::prototypeView.get();
    if (layer) populateLayer(layer,row++,active.contains(layer),false);

    layer = (Layer*)Sys::mapEditorView.get();
    if (layer) populateLayer(layer,row++,active.contains(layer),false);

    layer = (Layer*)Sys::motifMakerView.get();
    if (layer) populateLayer(layer,row++,active.contains(layer),false);

    // populate tilings
    const QVector<TilingPtr> &  tilings = Sys::tilingMaker->getTilings();
    for (const TilingPtr & tiling : tilings)
    {
        Layer * layer = tiling.get();
        populateLayer(layer, row++, active.contains(layer),(tiling==pri));
    }

    layer = (Layer*)Sys::tilingMakerView.get();
    if (layer) populateLayer(layer,row++,active.contains(layer),false);

    layer = (Layer*)Sys::getBackgroundImageFromSource().get();
    if (layer) populateLayer(layer,row++,active.contains(layer),false);

    layer = (Layer*)Sys::cropViewer.get();
    if (layer) populateLayer(layer,row++,active.contains(layer),false);

    layer = (Layer*)Sys::gridViewer.get();
    if (layer) populateLayer(layer,row++,active.contains(layer),false);

    if (Sys::config->insightMode)
    {
        layer = (Layer*)Sys::debugView.get();
        if (layer) populateLayer(layer,row++,active.contains(layer),false);

        layer = (Layer*)Sys::imageViewer.get();
        if (layer) populateLayer(layer,row++,active.contains(layer),false);
    }

    layerTable->resizeColumnsToContents();
    layerTable->adjustTableSize();
}

void page_layer_algnment::populateLayer(Layer * layer, int row, bool active, bool primary)
{
    layerTable->setRowCount(layerTable->rowCount() + 1);
    QTableWidgetItem * twi = new QTableWidgetItem(layer->layerName());
    twi->setTextAlignment(Qt::AlignCenter);
    QVariant var(QVariant::fromValue(static_cast<void*>(layer)));
    twi->setData(Qt::UserRole,var);
    layerTable->setItem(row,LAYER_NAME,twi);

    // visible
    twi = new QTableWidgetItem();
    twi->setText(" ");  // can be overwritten byu makes empty cell clickable
    if (active)
    {
        twi->setText("active");
        if (Sys::isDarkTheme)
            twi->setForeground(Qt::green);
        else
            twi->setForeground(Qt::blue);
    }
    layerTable->setItem(row,LAYER_ACTIVE,twi);

    twi = new QTableWidgetItem();
    if (layer->isBreakaway() == false)
    {
        twi->setText("YES");
    }
    layerTable->setItem(row,LAYER_SMX,twi);

    twi = new QTableWidgetItem();
    if (layer->isBreakaway())
    {
        twi->setText("YES");
    }
    layerTable->setItem(row,LAYER_BREAKAWAY,twi);

    twi = new QTableWidgetItem();
    if (layer->isLocked())
    {
        twi->setText("YES");
    }
    layerTable->setItem(row,LAYER_LOCKED,twi);

    twi = new QTableWidgetItem();
    if (layer->isSolo())
    {
        twi->setText("YES");
    }
    layerTable->setItem(row,LAYER_SOLO,twi);

    Xform layXf = layer->getModelXform();
    switch (transformDisplay)
    {
    case DISP_XFORM:
        twi = new QTableWidgetItem(layXf.info(6));
        break;

    case DISP_XFORM_HASH:
        twi = new QTableWidgetItem(QString::number(layXf.hash()));
        break;

    case DISP_MODEL:
    {
        QTransform t = layer->getModelTransform();
        twi = new QTableWidgetItem(Transform::info(t));
    }   break;

    case DISP_CANVAS:
    {
        QTransform t = layer->getCanvasTransform();
        twi = new QTableWidgetItem(Transform::info(t));
    }   break;

    case DISP_LAYER:
    {
        QTransform t = layer->getLayerTransform();
        twi = new QTableWidgetItem(Transform::info(t));
    }   break;

    }

    if (primary)
    {
        twi->setForeground(Qt::red);
    }
    layerTable->setItem(row,LAYER_MODEL_XFORM,twi);
}

void page_layer_algnment::slot_dispTypeChanged(int i)
{
    int val = qcb->itemData(i).toInt();
    qDebug() << "val=" << val;
    transformDisplay = (eTransformDisplay)val;
    populateLayers();
}

void page_layer_algnment::slot_selectCell()
{
    QList<QTableWidgetItem *> selected = layerTable->selectedItems();
    if (selected.size() == 0)
        return;

    auto item = selected.first();

    item->setSelected(item->isSelected());

    uint row  = item->row();
    uint col  = item->column();

    auto twi = layerTable->item(row,LAYER_NAME);
    QVariant var  = twi->data(Qt::UserRole);
    Layer * layer = static_cast<Layer*>(var.value<void*>());

    switch (col)
    {
    case LAYER_LOCKED:
        if (layer->isLocked())
            emit viewControl->sig_lock(layer,false);
        else
            emit viewControl->sig_lock(layer,true);
        break;

    case LAYER_SOLO:
        if (layer->isSolo())
            emit viewControl->sig_solo(layer,false);
        else
            emit viewControl->sig_solo(layer,true);
        break;

    case LAYER_BREAKAWAY:
    case LAYER_SMX:
        if (layer->isBreakaway())
            emit viewControl->sig_breakaway(layer,false);
        else
            emit viewControl->sig_breakaway(layer,true);
        break;

    default:
        break;
    }
}


