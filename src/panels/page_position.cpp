/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusianf and Islamic art
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

#include "page_position.h"
#include "base/patterns.h"
#include "base/canvas.h"
#include "viewers/workspaceviewer.h"
#include "makers/mapeditor.h"

using std::string;

#define SPACING 50

page_position:: page_position(ControlPanel * cpanel)  : panel_page(cpanel,"Position")
{
    createDesignWidget();
    vbox->addWidget(designWidget);

    createLayerTable();
    vbox->addWidget(layerTable);

    setMouseTracking(true);

    onEnter();
}

void page_position::createDesignWidget()
{
    designWidget       = new QWidget;
    QVBoxLayout * vbox = new QVBoxLayout;
    designWidget->setLayout(vbox);

    xSliderSet      = new SliderSet("X",0,-100,100);
    ySliderSet      = new SliderSet("Y",0,-100,100);
    scaleSliderSet  = new SliderSet("Scale",1.0,0,200);
    vbox->addLayout(xSliderSet);
    vbox->addLayout(ySliderSet);
    vbox->addLayout(scaleSliderSet);
    vbox->addSpacing(19);

    QLabel * label;

    QHBoxLayout * hbox = new QHBoxLayout;

    label = new QLabel("xStart");
    hbox->addWidget(label);
    xStart = new QSpinBox();
    hbox->addWidget(xStart);
    hbox->addSpacing(SPACING);

    label = new QLabel("xSep");
    hbox->addWidget(label);
    xSep = new QDoubleSpinBox();
    hbox->addWidget(xSep);
    hbox->addSpacing(SPACING);

    label = new QLabel("xOff");
    hbox->addWidget(label);
    xOff = new QDoubleSpinBox();
    hbox->addWidget(xOff);
    hbox->addSpacing(SPACING+20);

    vbox->addLayout(hbox);

    hbox = new QHBoxLayout;

    label = new QLabel("yStart");
    hbox->addWidget(label);
    yStart = new QSpinBox();
    hbox->addWidget(yStart);
    hbox->addSpacing(SPACING);

    label = new QLabel("ySep");
    hbox->addWidget(label);
    ySep = new QDoubleSpinBox();
    hbox->addWidget(ySep);
    hbox->addSpacing(SPACING);

    label = new QLabel("yOff");
    hbox->addWidget(label);
    yOff = new QDoubleSpinBox();
    hbox->addWidget(yOff);
    hbox->addSpacing(SPACING+20);

    vbox->addLayout(hbox);
    vbox->addStretch();

    const int rmin = -1000;
    const int rmax =  1000;
    xOff->setRange(rmin,rmax);
    yOff->setRange(rmin,rmax);
    xSep->setRange(rmin,rmax);
    ySep->setRange(rmin,rmax);
    xStart->setRange(rmin,rmax);
    yStart->setRange(rmin,rmax);

    connect(scaleSliderSet,     SIGNAL(valueChanged(int)),         this,    SLOT(setScale(int)));
    connect(xSep,               SIGNAL(valueChanged(qreal)),       this,    SLOT(set_sep(qreal)));
    connect(ySep,               SIGNAL(valueChanged(qreal)),       this,    SLOT(set_sep(qreal)));
    connect(xOff,               SIGNAL(valueChanged(qreal)),       this,    SLOT(set_off(qreal)));
    connect(yOff,               SIGNAL(valueChanged(qreal)),       this,    SLOT(set_off(qreal)));
    connect(xStart,             SIGNAL(valueChanged(int)),         this,    SLOT(set_start(int)));
    connect(yStart,             SIGNAL(valueChanged(int)),         this,    SLOT(set_start(int)));

    connect(this,               &page_position::sig_separationAbs,  canvas,  &Canvas::slot_repositionAbs);
    connect(this,               &page_position::sig_offsetAbs,      canvas,  &Canvas::slot_offsetAbs2);
    connect(this,               &page_position::sig_originAbs,      canvas,  &Canvas::slot_originAbs);
}

void page_position::createLayerTable()
{
    layerTable = new QTableWidget();
    layerTable->setSortingEnabled(false);
    layerTable->setColumnCount(6);
    layerTable->setSelectionMode(QAbstractItemView::SingleSelection);
    layerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    layerTable->setColumnWidth(PP_CLEAR,50);
    layerTable->verticalHeader()->setVisible(false);


    QStringList qslh;
    qslh << "Top-left (X)" << "Top (Y)" << "Scale" << "Rot" << "Layer Transform" << "Clear";
    layerTable->setHorizontalHeaderLabels(qslh);

    connect(&leftMapper,  SIGNAL(mapped(int)), this, SLOT(slot_set_deltas(int)));
    connect(&topMapper,   SIGNAL(mapped(int)), this, SLOT(slot_set_deltas(int)));
    connect(&widthMapper, SIGNAL(mapped(int)), this, SLOT(slot_set_deltas(int)));
    connect(&rotMapper,   SIGNAL(mapped(int)), this, SLOT(slot_set_deltas(int)));
    connect(&clearMapper, SIGNAL(mapped(int)), this, SLOT(slot_clear_deltas(int)));

    connect(canvas, &Canvas::sig_deltaScale,    this, &page_position::onEnter);
    connect(canvas, &Canvas::sig_deltaRotate,   this, &page_position::onEnter);
    connect(canvas, &Canvas::sig_deltaMoveV,    this, &page_position::onEnter);
    connect(canvas, &Canvas::sig_deltaMoveH,    this, &page_position::onEnter);
}

void  page_position::onEnter()
{
    if (workspace->getDesigns().size())
    {
        designWidget->show();
        updateDesignWidget();
    }
    else
    {
        designWidget->hide();
    }

   updateLayerTable();
}

void  page_position::refreshPage()
{
    static bool first = true;

    QVector<Layer*> views = viewer->getActiveLayers();

    if (first && views.size())
    {
        // horrendous kludge
        onEnter();
        onEnter();
        first = false;
        return;
    }

    int row = 0;
    for (auto layer : views)
    {
        QTransform t = layer->getLayerTransform();
        QTableWidgetItem * item = layerTable->item(row,PP_LAYER_T);
        if (item)
        {
            item->setText(Transform::toInfoString(t));

        }
        row++;
    }
    layerTable->resizeColumnToContents(PP_LAYER_T);
    adjustTableSize(layerTable);
    updateGeometry();
}

void page_position::updateDesignWidget()
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    DesignPtr d = designs[0];

    scaleSliderSet->setValue(static_cast<int>(d->getDesignInfo().getDiameter()/2.0));

    xOff->setValue(d->getXoffset2());
    yOff->setValue(d->getYoffset2());
    xSep->setValue(d->getXseparation());
    ySep->setValue(d->getYseparation());
    QPoint pt = d->getDesignInfo().getStartTile().toPoint();
    xStart->setValue(pt.x());
    yStart->setValue(pt.y());
}

void page_position::updateLayerTable()
{
    layerTable->clearContents();
    QVector<Layer*> views = viewer->getActiveLayers();
    int num = views.size();
    layerTable->setRowCount(num);
    if (num == 0)return;

    int row = 0;
    for (auto layer : views)
    {
        addLayerToTable(layer,row);
        row++;
    }

    layerTable->resizeColumnToContents(PP_LEFT);
    layerTable->resizeColumnToContents(PP_TOP);
    layerTable->resizeColumnToContents(PP_SCALE);
    layerTable->resizeColumnToContents(PP_ROT);
    layerTable->resizeColumnToContents(PP_LAYER_T);
    adjustTableSize(layerTable);
    updateGeometry();
}

void page_position::addLayerToTable(Layer * layer, int row)
{
    //layerDesc->setText(layer->getName());

    Xform xf = layer->getDeltas();

    QDoubleSpinBox * dleft  = new QDoubleSpinBox();
    QDoubleSpinBox * dtop   = new QDoubleSpinBox();
    QDoubleSpinBox * dwidth = new QDoubleSpinBox();
    QDoubleSpinBox * drot   = new QDoubleSpinBox();

    dleft->setRange(-1000.0,1000.0);
    dtop->setRange(-1000.0,1000.0);
    dwidth->setRange(-1000.0,1000.0);
    drot->setRange(-360.0,360.0);

    dleft->setDecimals(16);
    dtop->setDecimals(16);
    dwidth->setDecimals(16);
    drot->setDecimals(16);

    dleft->setValue(xf.translateX);
    dtop->setValue(xf.translateY);
    dwidth->setValue(xf.scale);
    drot->setValue(qRadiansToDegrees(xf.rotationRadians));

    dwidth->setSingleStep(0.01);

    layerTable->setCellWidget(row,PP_LEFT,dleft);
    QObject::connect(dleft, SIGNAL(valueChanged(qreal)), &leftMapper, SLOT(map()));
    leftMapper.setMapping(dleft,row);

    layerTable->setCellWidget(row,PP_TOP,dtop);
    QObject::connect(dtop, SIGNAL(valueChanged(qreal)), &topMapper, SLOT(map()));
    topMapper.setMapping(dtop,row);

    layerTable->setCellWidget(row,PP_SCALE,dwidth);
    QObject::connect(dwidth, SIGNAL(valueChanged(qreal)), &widthMapper, SLOT(map()));
    widthMapper.setMapping(dwidth,row);

    layerTable->setCellWidget(row,PP_ROT,drot);
    QObject::connect(drot, SIGNAL(valueChanged(qreal)), &rotMapper, SLOT(map()));
    rotMapper.setMapping(drot,row);

    QTransform t = layer->getLayerTransform();
    QTableWidgetItem * item = new QTableWidgetItem(Transform::toInfoString(t));
    layerTable->setItem(row,PP_LAYER_T,item);

    QPushButton * clearD = new QPushButton("Clear");
    layerTable->setCellWidget(row,PP_CLEAR,clearD);
    QObject::connect(clearD, SIGNAL(clicked(bool)), &clearMapper, SLOT(map()));
    clearMapper.setMapping(clearD,row);
    clearD->setFixedWidth(48);
}

void page_position::setScale(int radius)
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    if (designs.size() == 0) return;
    DesignPtr d = designs[0];
    d->getDesignInfo().setDiameter(radius * 2.0);
    emit sig_render();
}

void page_position::set_sep(qreal)
{
    qreal xsep = xSep->value();
    qreal ysep = ySep->value();
    qDebug() << "xsep="  << xsep << "ysep=" << ysep;
    emit sig_separationAbs(xsep,ysep);
}

void page_position::set_off(qreal)
{
    qreal xoff = xOff->value();
    qreal yoff = yOff->value();
    qDebug() << "xoff="  << xoff << "yoff=" << yoff;
    emit sig_offsetAbs(xoff,yoff);
}

void page_position::set_start(int)
{
    int xstart = xStart->value();
    int ystart = yStart->value();
    qDebug() << "xstart="  << xstart << "ystart=" << ystart;
    emit sig_originAbs(xstart,ystart);
}

void page_position::slot_set_deltas(int row)
{
    qDebug() << "page_position::slot_set_deltas row =" << row;
    QVector<Layer*> views = viewer->getActiveLayers();

    if (row > (views.size() -1))
    {
        qDebug() << "invalid row =" << row  << "size = " << views.size();
        return;
    }

    Layer * layer = views[row];

    QWidget * w           = layerTable->cellWidget(row,PP_LEFT);
    QDoubleSpinBox * spin = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(spin);
    qreal dleft = spin->value();

    w    = layerTable->cellWidget(row,PP_TOP);
    spin = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(spin);
    qreal dtop = spin->value();

    w    = layerTable->cellWidget(row,PP_SCALE);
    spin = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(spin);
    qreal dwidth = spin->value();

    w    = layerTable->cellWidget(row,PP_ROT);
    spin = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(spin);
    qreal drot = spin->value();

    Xform xf = Xform(dwidth,qDegreesToRadians(drot), dleft, dtop);
    layer->setDeltas(xf);
    layer->forceUpdateLayer();
}

void page_position::slot_clear_deltas(int row)
{
    QVector<Layer*> views = viewer->getActiveLayers();
    Layer * layer = views[row];
    Xform xf;
    layer->setDeltas(xf);
    layer->forceUpdateLayer();
    onEnter();
}
