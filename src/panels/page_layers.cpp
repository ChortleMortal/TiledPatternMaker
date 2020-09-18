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
#include "base/utilities.h"
#include "viewers/workspace_viewer.h"

using std::string;

Q_DECLARE_METATYPE(WeakLayerPtr)

page_layers:: page_layers(ControlPanel * cpanel)  : panel_page(cpanel,"Layer Info")
{
    layerTable = new AQTableWidget(this);

    vbox->addWidget(layerTable);
    vbox->addStretch();

    layerTable->setRowCount(NUM_LAYER_ROWS);

    QStringList qslV;
    qslV << "Layer" << "Visible" << "Z-level" << ""
         << "View Scale"  << "View Rot"  << "View Left (X)"  << "View Top (Y)" << ""
         << "Canvas Scale" << "Canvas Rot" << "Canvas Left (X)" << "Canvas Top (Y)" << "Canvas CentreX" << "Canvas CentreY"
         << "Layer Centre" << "Layer Scale" << "Layer Rot" << "Layer X" << "Layer Y" << "Sub-layers";

    layerTable->setVerticalHeaderLabels(qslV);
    layerTable->horizontalHeader()->setVisible(false);
    layerTable->setMaximumWidth(880);
    layerTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    layerTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(&visMapper,      SIGNAL(mapped(int)), this, SLOT(slot_visibilityChanged(int)));
    connect(&zMapper,        SIGNAL(mapped(int)), this, SLOT(slot_zChanged(int)));
    connect(&alignMapper,    SIGNAL(mapped(int)), this, SLOT(slot_alignPressed(int)));
    connect(&leftMapper,     SIGNAL(mapped(int)), this, SLOT(slot_set_deltas(int)));
    connect(&topMapper,      SIGNAL(mapped(int)), this, SLOT(slot_set_deltas(int)));
    connect(&widthMapper,    SIGNAL(mapped(int)), this, SLOT(slot_set_deltas(int)));
    connect(&rotMapper,      SIGNAL(mapped(int)), this, SLOT(slot_set_deltas(int)));
    connect(&cenXMapper,     SIGNAL(mapped(int)), this, SLOT(slot_set_deltas(int)));
    connect(&cenYMapper,     SIGNAL(mapped(int)), this, SLOT(slot_set_deltas(int)));
    connect(&clearMapper,    SIGNAL(mapped(int)), this, SLOT(slot_clear_deltas(int)));

    connect(workspace, &View::sig_deltaScale,    this, &page_layers::refreshCanvas);
    connect(workspace, &View::sig_deltaRotate,   this, &page_layers::refreshCanvas);
    connect(workspace, &View::sig_deltaMoveY,    this, &page_layers::refreshCanvas);
    connect(workspace, &View::sig_deltaMoveX,    this, &page_layers::refreshCanvas);
    connect(workspace, &View::sig_wheel_scale,   this, &page_layers::refreshCanvas);
    connect(workspace, &View::sig_wheel_rotate,  this, &page_layers::refreshCanvas);
    connect(workspace, &View::sig_mouseTranslate,this, &page_layers::refreshCanvas);
}

void page_layers::onEnter()
{
    populateLayers();
    refreshCanvas();
}

void page_layers::populateLayers()
{
    layerTable->clearContents();

    QVector<LayerPtr> layers = workspace->getActiveLayers();
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
    connect(layer.get(), &Layer::sig_center,  this, &page_layers::refreshCanvas, Qt::UniqueConnection);

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
    connect(cb, SIGNAL(toggled(bool)), &visMapper, SLOT(map()),Qt::UniqueConnection);
    visMapper.setMapping(cb,col);

    // z-level
    qreal z = layer->zValue();
    QDoubleSpinBox * zBox = new QDoubleSpinBox;
    zBox->setRange(-10,10);
    zBox->setValue(z);
    zBox->setAlignment(Qt::AlignCenter);
    layerTable->setCellWidget(LAYER_Z,col,zBox);

    connect(zBox, SIGNAL(valueChanged(double)), &zMapper, SLOT(map()),Qt::UniqueConnection);
    zMapper.setMapping(zBox,col);

    // align
    QPushButton * abtn = new QPushButton("Align-to-this");
    layerTable->setCellWidget(LAYER_ALIGN,col,abtn);
    connect(abtn, SIGNAL(clicked(bool)), &alignMapper, SLOT(map()),Qt::UniqueConnection);
    alignMapper.setMapping(abtn,col);

    // view transform
    QTransform t = layer->getLayerTransform();

    QTableWidgetItem * item = new QTableWidgetItem( QString::number(Transform::scalex(t),'f',16));
    layerTable->setItem(FRAME_SCALE,col,item);
    item->setBackground(Qt::yellow);
    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

    item = new QTableWidgetItem( QString::number(Transform::rotation(t),'f',16));
    layerTable->setItem(FRAME_ROT,col,item);
    item->setBackground(Qt::yellow);
    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

    item = new QTableWidgetItem( QString::number(Transform::transx(t),'f',16));
    layerTable->setItem(FRAME_X,col,item);
    item->setBackground(Qt::yellow);
    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

    item = new QTableWidgetItem( QString::number(Transform::transy(t),'f',16));
    layerTable->setItem(FRAME_Y,col,item);
    item->setBackground(Qt::yellow);
    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

    // scene xfrorm
    Xform xf = layer->getCanvasXform();

    QDoubleSpinBox * dleft  = new QDoubleSpinBox();
    QDoubleSpinBox * dtop   = new QDoubleSpinBox();
    QDoubleSpinBox * dwidth = new QDoubleSpinBox();
    QDoubleSpinBox * drot   = new QDoubleSpinBox();
    QDoubleSpinBox * dcenX  = new QDoubleSpinBox();
    QDoubleSpinBox * dcenY  = new QDoubleSpinBox();

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
    QObject::connect(dwidth, SIGNAL(valueChanged(qreal)), &widthMapper, SLOT(map()));
    widthMapper.setMapping(dwidth,col);

    layerTable->setCellWidget(CANVAS_ROT,col,drot);
    QObject::connect(drot, SIGNAL(valueChanged(qreal)), &rotMapper, SLOT(map()));
    rotMapper.setMapping(drot,col);

    layerTable->setCellWidget(CANVAS_X,col,dleft);
    QObject::connect(dleft, SIGNAL(valueChanged(qreal)), &leftMapper, SLOT(map()));
    leftMapper.setMapping(dleft,col);

    layerTable->setCellWidget(CANVAS_Y,col,dtop);
    QObject::connect(dtop, SIGNAL(valueChanged(qreal)), &topMapper, SLOT(map()));
    topMapper.setMapping(dtop,col);

    layerTable->setCellWidget(CANVAS_CENTER_X,col,dcenX);
    QObject::connect(dcenX, SIGNAL(valueChanged(qreal)), &cenXMapper, SLOT(map()));
    cenXMapper.setMapping(dcenX,col);

    layerTable->setCellWidget(CANVAS_CENTER_Y,col,dcenY);
    QObject::connect(dcenY, SIGNAL(valueChanged(qreal)), &cenYMapper, SLOT(map()));
    cenYMapper.setMapping(dcenY,col);

    // layer transform
    twi = new QTableWidgetItem();
    t = layer->getLayerTransform();

    layerTable->setItem(LAYER_CENTER,col,twi);
    twi->setBackground(Qt::yellow);
    twi->setTextAlignment(Qt::AlignCenter);

    item = new QTableWidgetItem( QString::number(Transform::scalex(t),'f',16));
    layerTable->setItem(LAYER_SCALE,col,item);
    item->setBackground(Qt::yellow);
    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

    item = new QTableWidgetItem( QString::number(Transform::rotation(t),'f',16));
    layerTable->setItem(LAYER_ROT,col,item);
    item->setBackground(Qt::yellow);
    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

    item = new QTableWidgetItem( QString::number(Transform::transx(t),'f',16));
    layerTable->setItem(LAYER_X,col,item);
    item->setBackground(Qt::yellow);
    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

    item = new QTableWidgetItem( QString::number(Transform::transy(t),'f',16));
    layerTable->setItem(LAYER_Y,col,item);
    item->setBackground(Qt::yellow);
    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

    QPushButton * clearD = new QPushButton("Clear Canvas");
    layerTable->setCellWidget(CANVAS_CLEAR,col,clearD);
    QObject::connect(clearD, SIGNAL(clicked(bool)), &clearMapper, SLOT(map()));
    clearMapper.setMapping(clearD,col);

    item = new QTableWidgetItem(QString::number(layer->numSubLayers()));
    item->setBackground(Qt::yellow);
    item->setTextAlignment(Qt::AlignCenter);
    layerTable->setItem(SUB_LAYERS,col,item);
}

void  page_layers::refreshPage()
{
    QVector<LayerPtr> layers = workspace->getActiveLayers();
    if (layers.size() != layerTable->columnCount())
    {
        populateLayers();
        refreshCanvas();
    }

    int col = 0;
    for (auto layer : layers)
    {
        // design number
        QTableWidgetItem * twi = layerTable->item(LAYER_NAME,col);
        twi->setText(QString("%1 %2").arg(layer->getName()).arg(Utils::addr(layer.get())));

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
        QDoubleSpinBox * zBox = dynamic_cast<QDoubleSpinBox*>(w);
        Q_ASSERT(zBox);
        zBox->blockSignals(true);
        zBox->setValue(layer->zValue());
        zBox->blockSignals(false);

        // view transform
        QTransform t = layer->getFrameTransform();

        QTableWidgetItem * item = layerTable->item(FRAME_SCALE,col);
        item->setText(QString::number(Transform::scalex(t),'f',16));

        item = layerTable->item(FRAME_ROT,col);
        item->setText(QString::number(Transform::rotation(t),'f',16));

        item = layerTable->item(FRAME_X,col);
        item->setText(QString::number(Transform::transx(t),'f',16));

        item = layerTable->item(FRAME_Y,col);
        item->setText(QString::number(Transform::transy(t),'f',16));

        // layer transform
        t = layer->getLayerTransform();

        twi = layerTable->item(LAYER_CENTER,col);
        QPointF center = layer->getCenterScreen();
        twi->setText(QString("%1 : %2").arg(QString::number(center.x(),'f',4)).arg(QString::number(center.y(),'f',4)));

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
        col++;
    }

    layerTable->adjustTableSize(880);
    updateGeometry();
}

void page_layers::refreshCanvas()
{
    int col = 0;
    QVector<LayerPtr> layers = workspace->getActiveLayers();
    if (layers.size() != layerTable->columnCount())
    {
        refreshPage();
        return;
    }

    for (auto layer : layers)
    {
        // canvas transform
        Xform xf = layer->getCanvasXform();

        QDoubleSpinBox * spin;
        QWidget * w  = layerTable->cellWidget(CANVAS_SCALE,col);
        spin = dynamic_cast<QDoubleSpinBox*>(w);
        Q_ASSERT(spin);
        w->blockSignals(true);
        spin->setValue(xf.getScale());
        w->blockSignals(false);

        w = layerTable->cellWidget(CANVAS_ROT,col);
        spin = dynamic_cast<QDoubleSpinBox*>(w);
        Q_ASSERT(spin);
        w->blockSignals(true);
        spin->setValue(xf.getRotateDegrees());
        w->blockSignals(false);

        w = layerTable->cellWidget(CANVAS_X,col);
        spin = dynamic_cast<QDoubleSpinBox*>(w);
        Q_ASSERT(spin);
        w->blockSignals(true);
        spin->setValue(xf.getTranslateX());
        w->blockSignals(false);

        w  = layerTable->cellWidget(CANVAS_Y,col);
        spin = dynamic_cast<QDoubleSpinBox*>(w);
        Q_ASSERT(spin);
        w->blockSignals(true);
        spin->setValue(xf.getTranslateY());
        w->blockSignals(false);

        QPointF center = xf.getCenter();

        w = layerTable->cellWidget(CANVAS_CENTER_X,col);
        spin = dynamic_cast<QDoubleSpinBox*>(w);
        Q_ASSERT(spin);
        w->blockSignals(true);
        spin->setValue(center.x());
        w->blockSignals(false);

        w  = layerTable->cellWidget(CANVAS_CENTER_Y,col);
        spin = dynamic_cast<QDoubleSpinBox*>(w);
        Q_ASSERT(spin);
        w->blockSignals(true);
        spin->setValue(center.y());
        w->blockSignals(false);
        col++;
    }
}



void page_layers::slot_visibilityChanged(int col)
{
    QWidget * qw = layerTable->cellWidget(LAYER_VISIBILITY,col);
    Q_ASSERT(qw);
    QCheckBox * cb = qw->findChild<QCheckBox*>();
    Q_ASSERT(cb);
    bool visible   = cb->isChecked();

    LayerPtr layer = getLayer(col);
    if (layer)
    {
        qDebug() << "visibility changed: row=" << col << "layer=" << layer->getName();
        layer->setVisible(visible);
        layer->forceRedraw();
    }
}

void page_layers::slot_zChanged(int col)
{
    QDoubleSpinBox * dsp = dynamic_cast<QDoubleSpinBox*>(layerTable->cellWidget(LAYER_Z,col));
    Q_ASSERT(dsp);
    qreal z = dsp->value();

    LayerPtr layer = getLayer(col);
    if (layer)
    {
        qDebug() << "z-level changed: row=" << col << "layer=" << layer->getName();
        layer->setZValue(z);
        layer->forceRedraw();
    }
}

void page_layers::slot_alignPressed(int col)
{
    LayerPtr layer = getLayer(col);
    if (!layer) return;

    Xform xf =  layer->getCanvasXform();

    qDebug() << "align to: col=" << col << "layer=" << layer->getName();

    // apply settings to
    QVector<LayerPtr> layers = workspace->getActiveLayers();
    for (auto olayer : layers)
    {
        if (olayer == layer)
        {
            continue;
        }
        olayer->setCanvasXform(xf);
        olayer->forceLayerRecalc();
    }
    onEnter();
}


void page_layers::slot_set_deltas(int col)
{
    qDebug() << "page_layers::slot_set_deltas col =" << col;

    LayerPtr layer = getLayer(col);
    if (!layer) return;

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

    w    = layerTable->cellWidget(CANVAS_CENTER_X,col);
    spin = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(spin);
    qreal cenx = spin->value();

    w    = layerTable->cellWidget(CANVAS_CENTER_Y,col);
    spin = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(spin);
    qreal ceny = spin->value();

    QPointF center(cenx,ceny);

    Xform xf = Xform(dwidth,qDegreesToRadians(drot), dleft, dtop);
    xf.setCenter(center);
    layer->setCanvasXform(xf);
}

void page_layers::slot_clear_deltas(int col)
{
    LayerPtr layer = getLayer(col);
    if (!layer) return;

    Xform xf;
    layer->setCanvasXform(xf);
    onEnter();
}

LayerPtr page_layers::getLayer(int col)
{
    LayerPtr layer;
    QVector<LayerPtr> views = workspace->getActiveLayers();

    if (col > (views.size() -1))
    {
        qDebug() << "invalid col =" << col  << "size = " << views.size();
        return layer;
    }

    layer = views[col];
    return layer;
}
