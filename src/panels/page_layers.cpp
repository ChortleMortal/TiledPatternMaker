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

#include "panels/page_layers.h"
#include "designs/patterns.h"
#include "base/canvas.h"
#include "base/utilities.h"
#include "viewers/workspace_viewer.h"

using std::string;

Q_DECLARE_METATYPE(LayerPtr)

page_layers:: page_layers(ControlPanel * cpanel)  : panel_page(cpanel,"Layer Info")
{
    layerTable = new AQTableWidget(this);

    vbox->addWidget(layerTable);
    vbox->addStretch();


    layerTable->setRowCount(NUM_LAYER_ROWS);

    QStringList qslV;
    qslV << "Layer" << "Visible" << "Z-level" << "Align-to"
         << "View Scale"  << "View Rot"  << "View Left (X)"  << "View Top (Y)"
         << "Canvas Scale" << "Canvas Rot" << "Canvas Left (X)" << "Canvas Top (Y)" << "Clear"
         << "Center" << "Layer Scale" << "Layer Rot" << "Layer X" << "Layer Y" << "Sub-layers";

    layerTable->setVerticalHeaderLabels(qslV);
    layerTable->horizontalHeader()->setVisible(false);
    layerTable->setMaximumWidth(880);
    layerTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    layerTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(&visibilityMapper, SIGNAL(mapped(int)), this, SLOT(slot_visibilityChanged(int)),Qt::UniqueConnection);
    connect(&zMapper,        SIGNAL(mapped(int)), this, SLOT(slot_zChanged(int)),Qt::UniqueConnection);
    connect(&alignMapper,    SIGNAL(mapped(int)), this, SLOT(slot_alignPressed(int)),Qt::UniqueConnection);
    connect(&leftMapper,     SIGNAL(mapped(int)), this, SLOT(slot_set_deltas(int)));
    connect(&topMapper,      SIGNAL(mapped(int)), this, SLOT(slot_set_deltas(int)));
    connect(&widthMapper,    SIGNAL(mapped(int)), this, SLOT(slot_set_deltas(int)));
    connect(&rotMapper,      SIGNAL(mapped(int)), this, SLOT(slot_set_deltas(int)));
    connect(&clearMapper,    SIGNAL(mapped(int)), this, SLOT(slot_clear_deltas(int)));

    connect(canvas, &Canvas::sig_deltaScale,    this, &page_layers::refreshPage);
    connect(canvas, &Canvas::sig_deltaRotate,   this, &page_layers::refreshPage);
    connect(canvas, &Canvas::sig_deltaMoveY,    this, &page_layers::refreshPage);
    connect(canvas, &Canvas::sig_deltaMoveX,    this, &page_layers::refreshPage);
}

void page_layers::onEnter()
{
    populateLayers();
}

void  page_layers::refreshPage()
{
    if (!refresh)
    {
        return;
    }

    QVector<LayerPtr> layers = view->getActiveLayers();
    if (layers.size() != layerTable->columnCount())
    {
        populateLayers();
    }

    int col = 0;
    for (auto layer : layers)
    {
        // design number
        QTableWidgetItem * twi = layerTable->item(LAYER_NAME,col);
        twi->setText(QString("%1 %2").arg(layer->getName()).arg(Utils::addr(layer.get())));
        twi->setData(Qt::UserRole,QVariant::fromValue(layer));

        // layer number and visibility
        QWidget * w = layerTable->cellWidget(LAYER_VISIBILITY,col);
        QCheckBox * cb = dynamic_cast<QCheckBox*>(w);
        Q_ASSERT(cb);
        cb->blockSignals(true);
        cb->setChecked(layer->isVisible());
        cb->blockSignals(false);

        // z-level
        w = layerTable->cellWidget(LAYER_Z,col);
        QDoubleSpinBox * zBox = dynamic_cast<QDoubleSpinBox*>(w);
        Q_ASSERT(zBox);
        zBox->blockSignals(true);
        zBox->setValue(layer->zValue());
        zBox->blockSignals(false);

        // view transform
        QTransform t = layer->getViewTransform();

        QTableWidgetItem * item = layerTable->item(VIEW_SCALE,col);
        item->setText(QString::number(Transform::scalex(t),'f',16));

        item = layerTable->item(VIEW_ROT,col);
        item->setText(QString::number(Transform::rotation(t),'f',16));

        item = layerTable->item(VIEW_X,col);
        item->setText(QString::number(Transform::transx(t),'f',16));

        item = layerTable->item(VIEW_Y,col);
        item->setText(QString::number(Transform::transy(t),'f',16));

        // canvas transform
        Xform xf = layer->getCanvasXform();

        QDoubleSpinBox * spin;
        w    = layerTable->cellWidget(CANVAS_ROT,col);
        spin = dynamic_cast<QDoubleSpinBox*>(w);
        Q_ASSERT(spin);
        w->blockSignals(true);
        spin->setValue(xf.getRotateDegrees());
        w->blockSignals(false);

        w    = layerTable->cellWidget(CANVAS_SCALE,col);
        spin = dynamic_cast<QDoubleSpinBox*>(w);
        Q_ASSERT(spin);
        w->blockSignals(true);
        spin->setValue(xf.getScale());
        w->blockSignals(false);

        w    = layerTable->cellWidget(CANVAS_X,col);
        spin = dynamic_cast<QDoubleSpinBox*>(w);
        Q_ASSERT(spin);
        w->blockSignals(true);
        spin->setValue(xf.getTranslateX());
        w->blockSignals(false);

        w    = layerTable->cellWidget(CANVAS_Y,col);
        spin = dynamic_cast<QDoubleSpinBox*>(w);
        Q_ASSERT(spin);
        w->blockSignals(true);
        spin->setValue(xf.getTranslateY());
        w->blockSignals(false);

        twi = layerTable->item(LAYER_CENTER,col);
        QPointF center = layer->getCenter();
        twi->setText(QString("%1 : %2").arg(center.x()).arg(center.y()));

        // layer transform
        t = layer->getLayerTransform();

        item = layerTable->item(LAYER_SCALE,col);
        item->setText(QString::number(Transform::scalex(t),'f',16));

        item = layerTable->item(LAYER_ROT,col);
        item->setText(QString::number(Transform::rotation(t),'f',16));

        item = layerTable->item(LAYER_X,col);
        item->setText(QString::number(Transform::transx(t),'f',16));

        item = layerTable->item(SUB_LAYERS,col);
        item->setText(QString::number(layer->numSubLayers()));

        // sub-layers
        item = layerTable->item(LAYER_Y,col);
        item->setText(QString::number(Transform::transy(t),'f',16));

        //
        layerTable->setColumnWidth(col,151);
        col++;
    }

    layerTable->adjustTableSize(880);
    updateGeometry();
}

void page_layers::populateLayers()
{
    layerTable->clearContents();

    QVector<LayerPtr> layers = view->getActiveLayers();
    layerTable->setColumnCount(layers.size());

    int col = 0;
    for (auto layer : layers)
    {
        populateLayer(layer,col++);
    }

    layerTable->adjustTableSize(880);
    updateGeometry();
}

void page_layers::populateLayer(LayerPtr layer, int col)
{
    // design number
    QTableWidgetItem * twi = new QTableWidgetItem(layer->getName());
    twi->setData(Qt::UserRole,QVariant::fromValue(layer));
    layerTable->setItem(LAYER_NAME,col,twi);

    // layer number and visibility
    QCheckBox * cb = new QCheckBox();
    layerTable->setCellWidget(LAYER_VISIBILITY,col,cb);
    cb->setChecked(layer->isVisible());
    connect(cb, SIGNAL(toggled(bool)), &visibilityMapper, SLOT(map()),Qt::UniqueConnection);
    visibilityMapper.setMapping(cb,col);

    // z-level
    qreal z = layer->zValue();
    QDoubleSpinBox * zBox = new QDoubleSpinBox;
    zBox->setRange(-10,10);
    zBox->setValue(z);
    layerTable->setCellWidget(LAYER_Z,col,zBox);

    connect(zBox, SIGNAL(valueChanged(double)), &zMapper, SLOT(map()),Qt::UniqueConnection);
    zMapper.setMapping(zBox,col);

    // align
    QPushButton * abtn = new QPushButton("Align-to-this");
    layerTable->setCellWidget(LAYER_ALIGN,col,abtn);
    connect(abtn, SIGNAL(clicked(bool)), &alignMapper, SLOT(map()),Qt::UniqueConnection);
    alignMapper.setMapping(abtn,col);

    // view transform
    QTransform t = layer->getViewTransform();

    QTableWidgetItem * item = new QTableWidgetItem( QString::number(Transform::scalex(t),'f',16));
    layerTable->setItem(VIEW_SCALE,col,item);
    item->setBackground(Qt::yellow);

    item = new QTableWidgetItem( QString::number(Transform::rotation(t),'f',16));
    layerTable->setItem(VIEW_ROT,col,item);
    item->setBackground(Qt::yellow);

    item = new QTableWidgetItem( QString::number(Transform::transx(t),'f',16));
    layerTable->setItem(VIEW_X,col,item);
    item->setBackground(Qt::yellow);

    item = new QTableWidgetItem( QString::number(Transform::transy(t),'f',16));
    layerTable->setItem(VIEW_Y,col,item);
    item->setBackground(Qt::yellow);

    // scene xfrorm
    Xform xf = layer->getCanvasXform();

    QDoubleSpinBox * dleft  = new QDoubleSpinBox();
    QDoubleSpinBox * dtop   = new QDoubleSpinBox();
    QDoubleSpinBox * dwidth = new QDoubleSpinBox();
    QDoubleSpinBox * drot   = new QDoubleSpinBox();

    dleft->setRange(-4096.0,4096.0);
    dtop->setRange(-3840.0,3840.0);
    dwidth->setRange(-4096.0,4096.0);
    drot->setRange(-360.0,360.0);

    dleft->setDecimals(16);
    dtop->setDecimals(16);
    dwidth->setDecimals(16);
    drot->setDecimals(16);

    dleft->setValue(xf.getTranslateX());
    dtop->setValue(xf.getTranslateY());
    dwidth->setValue(xf.getScale());
    drot->setValue(qRadiansToDegrees(xf.getRotateRadians()));

    dwidth->setSingleStep(0.01);

    layerTable->setCellWidget(CANVAS_X,col,dleft);
    QObject::connect(dleft, SIGNAL(valueChanged(qreal)), &leftMapper, SLOT(map()));
    leftMapper.setMapping(dleft,col);

    layerTable->setCellWidget(CANVAS_Y,col,dtop);
    QObject::connect(dtop, SIGNAL(valueChanged(qreal)), &topMapper, SLOT(map()));
    topMapper.setMapping(dtop,col);

    layerTable->setCellWidget(CANVAS_SCALE,col,dwidth);
    QObject::connect(dwidth, SIGNAL(valueChanged(qreal)), &widthMapper, SLOT(map()));
    widthMapper.setMapping(dwidth,col);

    layerTable->setCellWidget(CANVAS_ROT,col,drot);
    QObject::connect(drot, SIGNAL(valueChanged(qreal)), &rotMapper, SLOT(map()));
    rotMapper.setMapping(drot,col);

    twi = new QTableWidgetItem();
    layerTable->setItem(LAYER_CENTER,col,twi);

    // layer transform
    t = layer->getLayerTransform();

    item = new QTableWidgetItem( QString::number(Transform::scalex(t),'f',16));
    layerTable->setItem(LAYER_SCALE,col,item);
    item->setBackground(Qt::yellow);

    item = new QTableWidgetItem( QString::number(Transform::rotation(t),'f',16));
    layerTable->setItem(LAYER_ROT,col,item);
    item->setBackground(Qt::yellow);

    item = new QTableWidgetItem( QString::number(Transform::transx(t),'f',16));
    layerTable->setItem(LAYER_X,col,item);
    item->setBackground(Qt::yellow);

    item = new QTableWidgetItem( QString::number(Transform::transy(t),'f',16));
    layerTable->setItem(LAYER_Y,col,item);
    item->setBackground(Qt::yellow);

    QPushButton * clearD = new QPushButton("Clear");
    layerTable->setCellWidget(CANVAS_CLEAR,col,clearD);
    QObject::connect(clearD, SIGNAL(clicked(bool)), &clearMapper, SLOT(map()));
    clearMapper.setMapping(clearD,col);

    item = new QTableWidgetItem(QString::number(layer->numSubLayers()));
    layerTable->setItem(SUB_LAYERS,col,item);
}

void page_layers::slot_visibilityChanged(int col)
{
    QCheckBox * cb = dynamic_cast<QCheckBox*>(layerTable->cellWidget(LAYER_VISIBILITY,col));
    bool visible   = cb->isChecked();

    QTableWidgetItem * twi = layerTable->item(LAYER_NAME,col);
    QVariant tmp = twi->data(Qt::UserRole);
    LayerPtr layer = tmp.value<LayerPtr>();

    qDebug() << "visibility changed: row=" << col << "layer=" << layer->getName();

    layer->setVisible(visible);
    layer->forceRedraw();
}

void page_layers::slot_zChanged(int col)
{
    QDoubleSpinBox * dsp = dynamic_cast<QDoubleSpinBox*>(layerTable->cellWidget(LAYER_Z,col));
    qreal z = dsp->value();

    QTableWidgetItem * twi = layerTable->item(LAYER_NAME,col);
    QVariant tmp = twi->data(Qt::UserRole);
    LayerPtr layer = tmp.value<LayerPtr>();

    qDebug() << "z-level changed: row=" << col << "layer=" << layer->getName();

    layer->setZValue(z);
    layer->forceRedraw();
}

void page_layers::slot_alignPressed(int col)
{
    // get from settings
    QTableWidgetItem * twi = layerTable->item(LAYER_NAME,col);
    QVariant tmp = twi->data(Qt::UserRole);
    LayerPtr layer = tmp.value<LayerPtr>();
    Xform xf =  layer->getCanvasXform();

    qDebug() << "align to: col=" << col << "layer=" << layer->getName();

    // apply settings to
    QVector<LayerPtr> layers = view->getActiveLayers();
    for (auto otherlayer : layers)
    {
        if (otherlayer == layer)
        {
            continue;
        }
        otherlayer->setCanvasXform(xf);
        otherlayer->forceUpdateLayer();
    }
    onEnter();
}


void page_layers::slot_set_deltas(int col)
{
    qDebug() << "page_position::slot_set_deltas col =" << col;
    QVector<LayerPtr> layers = view->getActiveLayers();

    if (col > (layers.size() -1))
    {
        qDebug() << "invalid row =" << col  << "size = " << layers.size();
        return;
    }

    LayerPtr layer = layers[col];

    QWidget * w;
    QDoubleSpinBox * spin;

    w    = layerTable->cellWidget(CANVAS_X,col);
    spin = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(spin);
    qreal dleft = spin->value();

    w    = layerTable->cellWidget(CANVAS_Y,col);
    spin = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(spin);
    qreal dtop = spin->value();

    w    = layerTable->cellWidget(CANVAS_SCALE,col);
    spin = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(spin);
    qreal dwidth = spin->value();

    w    = layerTable->cellWidget(CANVAS_ROT,col);
    spin = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(spin);
    qreal drot = spin->value();

    Xform xf = Xform(dwidth,qDegreesToRadians(drot), dleft, dtop);
    layer->setCanvasXform(xf);
    layer->forceUpdateLayer();
}

void page_layers::slot_clear_deltas(int col)
{
    QVector<LayerPtr> layers = view->getActiveLayers();
    LayerPtr layer = layers[col];
    Xform xf;
    layer->setCanvasXform(xf);
    layer->forceUpdateLayer();
    onEnter();
}
