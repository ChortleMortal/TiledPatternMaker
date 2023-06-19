#include <QHeaderView>
#include <QCheckBox>

#include "geometry/transform.h"
#include "panels/page_layers.h"
#include "panels/panel.h"
#include "settings/configuration.h"
#include "style/style.h"
#include "viewers/viewcontrol.h"
#include "widgets/layout_sliderset.h"
#include "widgets/panel_misc.h"

using std::string;

page_layers:: page_layers(ControlPanel * cpanel)  : panel_page(cpanel,"Layer Info")
{
    refreshLabel = new QLabel("Refresh: OFF");
    QPushButton * pbDeselect = new QPushButton("De-select");

    layerTable   = new AQTableWidget(this);

    QHBoxLayout * hbox =  new QHBoxLayout;
    hbox->addWidget(refreshLabel);
    hbox->addSpacing(21);
    hbox->addWidget(pbDeselect);
    hbox->addStretch();

    vbox->addLayout(hbox);
    vbox->addWidget(layerTable);
    vbox->addStretch();

    layerTable->setRowCount(NUM_LAYER_ROWS);

    QStringList qslV;
    qslV << "Layer" << "Visible" << "Z-level" << "" << ""
         << "Frame Scale"  << "Frame Rot"  << "Frame Left (X)"  << "Frame Top (Y)" << ""
         << "Canvas Scale" << "Canvas Rot" << "Canvas Left (X)" << "Canvas Top (Y)" << "Canvas CenterX" << "Canvas CenterY"
         << "Layer Center" << "Layer Scale" << "Layer Rot" << "Layer X" << "Layer Y" << "Sub-layers";

    layerTable->setVerticalHeaderLabels(qslV);
    layerTable->horizontalHeader()->setVisible(false);
    layerTable->setMaximumWidth(880);
    layerTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    layerTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layerTable->setSelectionBehavior(QAbstractItemView::SelectColumns);
    layerTable->setStyleSheet("QTableWidget::item:selected { background:yellow; color:red; }");

    connect(layerTable, &QTableWidget::itemSelectionChanged, this, &page_layers::slot_selectLayer);
    connect(pbDeselect, &QPushButton::clicked, this, [this]() { layerTable->setCurrentIndex(QModelIndex()); config->selectedLayer = nullptr; layerTable->update();} );
}

void page_layers::onEnter()
{
    static QString msg("<body>"
                       "<span>Click on layer to select</span>"
                       "</body>");
    panel->pushPanelStatus(msg);

    populateLayers();
    onRefresh();
}

void page_layers::onExit()
{
    panel->popPanelStatus();
}

void page_layers::populateLayers()
{
    wlayers.clear();
    layerTable->clearContents();
    config->selectedLayer = nullptr;

    QVector<Layer *> layers = view->getActiveLayers();
    layerTable->setColumnCount(layers.size());

    int col = 0;
    int selected = -1;
    for (Layer * layer : layers)
    {
        if (layer == config->selectedLayer)
        {
            selected = col;
        }
        wlayers.push_back(layer);
        populateLayer(layer,col++);
    }

    layerTable->adjustTableSize(880);
    updateGeometry();

    if (selected >= 0)
    {
        layerTable->selectColumn(selected);
    }
    else
    {
        config->selectedLayer = nullptr;
    }
}

void page_layers::populateLayer(Layer * layer, int col)
{
    // design number
    QTableWidgetItem * twi = new QTableWidgetItem(layer->getName());
    twi->setTextAlignment(Qt::AlignCenter);
    layerTable->setItem(LAYER_NAME,col,twi);

    // layer number and visibility
    QWidget *cbWidget  = new QWidget();
    QCheckBox *cb      = new QCheckBox();
    QHBoxLayout *cbBox = new QHBoxLayout(cbWidget);
    cbBox->addWidget(cb);
    cbBox->setAlignment(Qt::AlignCenter);
    cbBox->setContentsMargins(0,0,0,0);
    layerTable->setCellWidget(LAYER_VISIBILITY,col,cbWidget);
    cb->setChecked(layer->isVisible());
    connect(cb, &QCheckBox::toggled, this, [this, col] { visibilityChanged(col); });

    // z-level
    qreal z = layer->zValue();
    AQDoubleSpinBox * zBox = new AQDoubleSpinBox;
    zBox->setRange(-10,10);
    zBox->setValue(z);
    zBox->setAlignment(Qt::AlignCenter);
    layerTable->setCellWidget(LAYER_Z,col,zBox);
    connect(zBox, static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this, [this,zBox,col] { zChanged(zBox,col); });

    // align
    QPushButton * align_btn = new QPushButton("Align-to-Selected");
    layerTable->setCellWidget(LAYER_ALIGN,col,align_btn);
    connect(align_btn, &QPushButton::clicked, this, [this,col] { alignPressed(col); });

    // align
    QPushButton * align_center_btn = new QPushButton("Align-to-selected-Center");
    layerTable->setCellWidget(LAYER_ALIGN_CENTER,col,align_center_btn);
    connect(align_center_btn, &QPushButton::clicked, this, [this,col] { alignCenterPressed(col); });

    // view transform
    QTransform t = layer->getLayerTransform();

    QColor bcolor;
    if (config->darkTheme)
        bcolor = QColor(0x777777);
    else
        bcolor = QColor(Qt::yellow);

    QTableWidgetItem * item = new QTableWidgetItem( QString::number(Transform::scalex(t),'f',16));
    layerTable->setItem(FRAME_SCALE,col,item);
    item->setBackground(bcolor);
    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

    item = new QTableWidgetItem( QString::number(Transform::rotation(t),'f',16));
    layerTable->setItem(FRAME_ROT,col,item);
    item->setBackground(bcolor);
    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

    item = new QTableWidgetItem( QString::number(Transform::transx(t),'f',16));
    layerTable->setItem(FRAME_X,col,item);
    item->setBackground(bcolor);
    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

    item = new QTableWidgetItem( QString::number(Transform::transy(t),'f',16));
    layerTable->setItem(FRAME_Y,col,item);
    item->setBackground(bcolor);
    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

    // scene xfrorm
    Xform xf = layer->getCanvasXform();

    AQDoubleSpinBox * dleft  = new AQDoubleSpinBox();
    AQDoubleSpinBox * dtop   = new AQDoubleSpinBox();
    AQDoubleSpinBox * dwidth = new AQDoubleSpinBox();
    AQDoubleSpinBox * drot   = new AQDoubleSpinBox();
    AQDoubleSpinBox * dcenX  = new AQDoubleSpinBox();
    AQDoubleSpinBox * dcenY  = new AQDoubleSpinBox();

    dleft->setRange(-4096.0,4096.0);
    dtop->setRange(-3840.0,3840.0);
    dwidth->setRange(-4096.0,4096.0);
    drot->setRange(-360.0,360.0);
    dcenX->setRange(-4096.0,4096.0);
    dcenY->setRange(-3840.0,3840.0);

    dleft->setDecimals(16);
    dtop->setDecimals(16);
    dwidth->setDecimals(16);
    drot->setDecimals(16);
    dcenX->setDecimals(16);
    dcenY->setDecimals(16);

    dleft->setAlignment(Qt::AlignRight);
    dtop->setAlignment(Qt::AlignRight);
    dwidth->setAlignment(Qt::AlignRight);
    drot->setAlignment(Qt::AlignRight);
    dcenX->setAlignment(Qt::AlignRight);
    dcenY->setAlignment(Qt::AlignRight);

    dleft->setValue(xf.getTranslateX());
    dtop->setValue(xf.getTranslateY());
    dwidth->setValue(xf.getScale());
    drot->setValue(qRadiansToDegrees(xf.getRotateRadians()));

    dwidth->setSingleStep(0.01);
    dcenX->setSingleStep(0.001);
    dcenY->setSingleStep(0.001);

    layerTable->setCellWidget(CANVAS_SCALE,col,dwidth);
    connect(dwidth, static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this, [this,col] { slot_set_deltas(col); });

    layerTable->setCellWidget(CANVAS_ROT,col,drot);
    connect(drot, static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this, [this,col] { slot_set_deltas(col); });

    layerTable->setCellWidget(CANVAS_X,col,dleft);
    connect(dleft,static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this, [this,col] { slot_set_deltas(col); });

    layerTable->setCellWidget(CANVAS_Y,col,dtop);
    connect(dtop, static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this, [this,col] { slot_set_deltas(col); });

    layerTable->setCellWidget(CANVAS_CENTER_X,col,dcenX);
    connect(dcenX,static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this, [this,col] { slot_set_center(col); });

    layerTable->setCellWidget(CANVAS_CENTER_Y,col,dcenY);
    connect(dcenY, static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this, [this,col] { slot_set_center(col); });

    // layer transform
    twi = new QTableWidgetItem();
    t = layer->getLayerTransform();

    layerTable->setItem(LAYER_CENTER,col,twi);
    twi->setBackground(bcolor);
    twi->setTextAlignment(Qt::AlignCenter);

    item = new QTableWidgetItem( QString::number(Transform::scalex(t),'f',16));
    layerTable->setItem(LAYER_SCALE,col,item);
    item->setBackground(bcolor);
    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

    item = new QTableWidgetItem( QString::number(Transform::rotation(t),'f',16));
    layerTable->setItem(LAYER_ROT,col,item);
    item->setBackground(bcolor);
    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

    item = new QTableWidgetItem( QString::number(Transform::transx(t),'f',16));
    layerTable->setItem(LAYER_X,col,item);
    item->setBackground(bcolor);
    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

    item = new QTableWidgetItem( QString::number(Transform::transy(t),'f',16));
    layerTable->setItem(LAYER_Y,col,item);
    item->setBackground(bcolor);
    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

    QPushButton * clearD = new QPushButton("Clear Canvas");
    layerTable->setCellWidget(CANVAS_CLEAR,col,clearD);
    connect(clearD, &QPushButton::clicked, this, [this,col] { clear_deltas(col); });

    item = new QTableWidgetItem(QString::number(layer->numSubLayers()));
    item->setBackground(bcolor);
    item->setTextAlignment(Qt::AlignCenter);
    layerTable->setItem(SUB_LAYERS,col,item);
}

void  page_layers::onRefresh()
{
    if (!refresh)
    {
        refreshLabel->setText("Refresh: OFF");
        return;
    }

    refreshLabel->setText("Refresh: ON");

    QVector<Layer*> view_layers = view->getActiveLayers();
    if (view_layers.size() != wlayers.size())
    {
        populateLayers();
    }
    else
    {
        for (int i= 0; i < wlayers.size(); i++)
        {
            Layer * layer = wlayers[i];
            if (layer != view_layers[i])
            {
                populateLayers();
                break;
            }
        }
    }

    // now refresh the data
    Q_ASSERT(layerTable->columnCount() == wlayers.size());
    for (int col = 0; col < wlayers.size(); col++)
    {
        Layer* layer = getLayer(col);
        if (!layer) continue;

        // design number
        QTableWidgetItem * twi = layerTable->item(LAYER_NAME,col);
        QString str = layer->getName();
        if (layer->getName() == "Style")
        {
            Style * style = dynamic_cast<Style*>(layer);
            if (style)
            {
                str = QString("Style: %1").arg(style->getStyleDesc());
            }
        }
        twi->setText(str);

        // layer number and visibility
        QWidget * w = layerTable->cellWidget(LAYER_VISIBILITY,col);
        QWidget * qw = dynamic_cast<QWidget*>(w);
        Q_ASSERT(qw);
        QCheckBox * cb = qw->findChild<QCheckBox*>();
        Q_ASSERT(cb);
        cb->blockSignals(true);
        cb->setChecked(layer->isVisible());
        cb->blockSignals(false);

        // z-level
        w = layerTable->cellWidget(LAYER_Z,col);
        AQDoubleSpinBox * zBox = dynamic_cast<AQDoubleSpinBox*>(w);
        Q_ASSERT(zBox);
        zBox->blockSignals(true);
        zBox->setValue(layer->zValue());
        zBox->blockSignals(false);

        // frame transform
        QTransform t = layer->getFrameTransform();

        QTableWidgetItem * item = layerTable->item(FRAME_SCALE,col);
        item->setText(QString::number(Transform::scalex(t),'f',16));

        item = layerTable->item(FRAME_ROT,col);
        item->setText(QString::number(Transform::rotation(t),'f',16));

        item = layerTable->item(FRAME_X,col);
        item->setText(QString::number(Transform::transx(t),'f',16));

        item = layerTable->item(FRAME_Y,col);
        item->setText(QString::number(Transform::transy(t),'f',16));


        // canvas transform
        Xform xf = layer->getCanvasXform();

        AQDoubleSpinBox * spin;
        w  = layerTable->cellWidget(CANVAS_SCALE,col);
        spin = dynamic_cast<AQDoubleSpinBox*>(w);
        Q_ASSERT(spin);
        w->blockSignals(true);
        spin->setValue(xf.getScale());
        w->blockSignals(false);

        w = layerTable->cellWidget(CANVAS_ROT,col);
        spin = dynamic_cast<AQDoubleSpinBox*>(w);
        Q_ASSERT(spin);
        w->blockSignals(true);
        spin->setValue(xf.getRotateDegrees());
        w->blockSignals(false);

        w = layerTable->cellWidget(CANVAS_X,col);
        spin = dynamic_cast<AQDoubleSpinBox*>(w);
        Q_ASSERT(spin);
        w->blockSignals(true);
        spin->setValue(xf.getTranslateX());
        w->blockSignals(false);

        w  = layerTable->cellWidget(CANVAS_Y,col);
        spin = dynamic_cast<AQDoubleSpinBox*>(w);
        Q_ASSERT(spin);
        w->blockSignals(true);
        spin->setValue(xf.getTranslateY());
        w->blockSignals(false);

        QPointF center = xf.getModelCenter();

        w = layerTable->cellWidget(CANVAS_CENTER_X,col);
        spin = dynamic_cast<AQDoubleSpinBox*>(w);
        Q_ASSERT(spin);
        w->blockSignals(true);
        spin->setValue(center.x());
        w->blockSignals(false);

        w  = layerTable->cellWidget(CANVAS_CENTER_Y,col);
        spin = dynamic_cast<AQDoubleSpinBox*>(w);
        Q_ASSERT(spin);
        w->blockSignals(true);
        spin->setValue(center.y());
        w->blockSignals(false);

        // layer transform
        t = layer->getLayerTransform();

        twi = layerTable->item(LAYER_CENTER,col);
        LayerController * lp = dynamic_cast<LayerController*>(layer);
        if (lp)
        {
            center = lp->getCenterScreenUnits();
            twi->setText(QString("%1 : %2").arg(QString::number(center.x(),'f',4)).arg(QString::number(center.y(),'f',4)));
        }

        item = layerTable->item(LAYER_SCALE,col);
        item->setText(QString::number(Transform::scalex(t),'f',16));

        item = layerTable->item(LAYER_ROT,col);
        item->setText(QString::number(Transform::rotation(t),'f',16));

        item = layerTable->item(LAYER_X,col);
        item->setText(QString::number(Transform::transx(t),'f',16));

        item = layerTable->item(LAYER_Y,col);
        item->setText(QString::number(Transform::transy(t),'f',16));

        item = layerTable->item(SUB_LAYERS,col);
        item->setText(QString::number(layer->numSubLayers()));

        layerTable->setColumnWidth(col,151);
    }

    layerTable->adjustTableSize(880);
    updateGeometry();
}

void page_layers::visibilityChanged(int col)
{
    QWidget * qw = layerTable->cellWidget(LAYER_VISIBILITY,col);
    Q_ASSERT(qw);
    QCheckBox * cb = qw->findChild<QCheckBox*>();
    Q_ASSERT(cb);
    bool visible   = cb->isChecked();

    auto layer = getLayer(col);
    if (!layer) return;

    qDebug() << "visibility changed: row=" << col << "layer=" << layer->getName();
    layer->setVisible(visible);
    layer->forceRedraw();
}

void page_layers::zChanged(AQDoubleSpinBox * dsp, int col)
{
    Q_ASSERT(dsp);
    qreal z = dsp->value();

    auto layer = getLayer(col);
    if (!layer) return;

    qDebug() << "z-level changed: row=" << col << "layer=" << layer->getName();
    layer->setZValue(z);
    layer->forceRedraw();
}

void page_layers::alignPressed(int col)
{
    auto selected = config->selectedLayer;
    if (!selected)
        return;

    auto layer = getLayer(col);
    if (!layer) return;

    Xform xf =  selected->getCanvasXform();
    layer->setCanvasXform(xf);
    layer->forceLayerRecalc();
    view->update();
    onRefresh();
}

void page_layers::alignCenterPressed(int col)
{
    auto selected = config->selectedLayer;
    if (!selected)
        return;

    auto layer = getLayer(col);
    if (!layer) return;

    Xform xf    =  selected->getCanvasXform();
    QPointF mpt = xf.getModelCenter();

    QPointF spt = selected->worldToScreen(mpt);
    layer->setCenterScreenUnits(spt);
    layer->forceLayerRecalc();
    view->update();
    onRefresh();
}

void page_layers::slot_set_deltas(int col)
{
    qDebug() << "page_layers::slot_set_deltas col =" << col;

    auto layer = getLayer(col);
    if (!layer) return;

    QWidget * w;
    AQDoubleSpinBox * spin;

    w    = layerTable->cellWidget(CANVAS_X,col);
    spin = dynamic_cast<AQDoubleSpinBox*>(w);
    Q_ASSERT(spin);
    qreal dleft = spin->value();

    w    = layerTable->cellWidget(CANVAS_Y,col);
    spin = dynamic_cast<AQDoubleSpinBox*>(w);
    Q_ASSERT(spin);
    qreal dtop = spin->value();

    w    = layerTable->cellWidget(CANVAS_SCALE,col);
    spin = dynamic_cast<AQDoubleSpinBox*>(w);
    Q_ASSERT(spin);
    qreal dwidth = spin->value();

    w    = layerTable->cellWidget(CANVAS_ROT,col);
    spin = dynamic_cast<AQDoubleSpinBox*>(w);
    Q_ASSERT(spin);
    qreal drot = spin->value();

    Xform xf;
    xf.setScale(dwidth);
    xf.setRotateDegrees(drot);
    xf.setTranslateX(dleft);
    xf.setTranslateY(dtop);
    layer->setCanvasXform(xf);
    view->update();
}

void page_layers::slot_set_center(int col)
{
    qDebug() << "page_layers::slot_set_deltas col =" << col;

    auto layer = getLayer(col);
    if (!layer) return;

    QWidget * w;
    AQDoubleSpinBox * spin;

    w    = layerTable->cellWidget(CANVAS_CENTER_X,col);
    spin = dynamic_cast<AQDoubleSpinBox*>(w);
    Q_ASSERT(spin);
    qreal cenx = spin->value();

    w    = layerTable->cellWidget(CANVAS_CENTER_Y,col);
    spin = dynamic_cast<AQDoubleSpinBox*>(w);
    Q_ASSERT(spin);
    qreal ceny = spin->value();

    QPointF center(cenx,ceny);
    QPointF scenter = layer->getLayerTransform().map(center);
    layer->setCenterScreenUnits(scenter);
    view->update();
}

void page_layers::clear_deltas(int col)
{
    auto layer = getLayer(col);
    if (!layer) return;

    Xform xf;
    layer->setCanvasXform(xf);
    view->update();
    populateLayers();
    onRefresh();
}

Layer * page_layers::getLayer(int col)
{
    return wlayers[col];
}

void page_layers::slot_selectLayer()
{
    QList<QTableWidgetItem *> selected = layerTable->selectedItems();
    if (selected.size())
    {
        int column = selected.first()->column();
        auto layer = getLayer(column);
        config->selectedLayer = layer;
    }
}
