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

#include "page_position.h"
#include "base/patterns.h"
#include "base/canvas.h"
#include "viewers/workspaceviewer.h"
#include "makers/mapeditor.h"

using std::string;

#define SPACING 50

page_position:: page_position(ControlPanel *panel)  : panel_page(panel,"Position")
{
    createDesignWidget();
    vbox->addWidget(designWidget);

    createStylesWidget();
    vbox->addWidget(stylesWidget);

    myBlockUpdates = false;
    setMouseTracking(true);
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

void page_position::createStylesWidget()
{
    stylesWidget       = new QWidget;
    QVBoxLayout * vbox = new QVBoxLayout;
    stylesWidget->setLayout(vbox);

    indexBox  = new QSpinBox;
    connect(indexBox,SIGNAL(valueChanged(int)),this, SLOT(slot_select(int)));

    layerDesc = new QLabel;

    bleft  = new DoubleSpinSet("Boundary left",0,-1000.0,1000.0);
    btop   = new DoubleSpinSet("Boundary top",0,-1000.0,1000.0);
    bwidth = new DoubleSpinSet("Boundary width",0,-1000.0,1000.0);
    btheta = new DoubleSpinSet("Boundary theta",0,-1000.0,1000.0);

    dleft  = new DoubleSpinSet("Delta left",0,-1000.0,1000.0);
    dtop   = new DoubleSpinSet("Delta top",0,-1000.0,1000.0);
    dwidth = new DoubleSpinSet("Delta width",0,-1000.0,1000.0);
    dtheta = new DoubleSpinSet("Delta theta",0,-1000.0,1000.0);

    aleft  = new DoubleSpinSet("Adjusted left",0,-1000.0,1000.0);
    atop   = new DoubleSpinSet("Adjusted top",0,-1000.0,1000.0);
    awidth = new DoubleSpinSet("Adjusted width",0,-1000.0,1000.0);
    atheta = new DoubleSpinSet("Adjusted theta",0,-1000.0,1000.0);
    aleft->setReadOnly(true);
    atop->setReadOnly(true);
    awidth->setReadOnly(true);
    atheta->setReadOnly(true);

    QPushButton * clearD = new QPushButton("Clear deltas");

    QGridLayout * grid = new QGridLayout;
    int row = 0;

    grid->addWidget(indexBox,row,0);
    grid->addWidget(layerDesc,row,1);
    grid->addWidget(clearD,row,2);

    row++;
    grid->addLayout(bleft,row,0);
    grid->addLayout(dleft,row,1);
    grid->addLayout(aleft,row,2);

    row++;
    grid->addLayout(btop,row,0);
    grid->addLayout(dtop,row,1);
    grid->addLayout(atop,row,2);

    row++;
    grid->addLayout(bwidth,row,0);
    grid->addLayout(dwidth,row,1);
    grid->addLayout(awidth,row,2);

    row++;
    grid->addLayout(btheta,row,0);
    grid->addLayout(dtheta,row,1);
    grid->addLayout(atheta,row,2);

    vbox->addLayout(grid);

    transLabel = new QLabel("transform");
    vbox->addWidget(transLabel);

    connect(clearD,         &QPushButton::clicked,             this,    &page_position::slot_clear_deltas);

    connect(bleft,          SIGNAL(valueChanged(qreal)),       this,    SLOT(slot_set_bounds(qreal)));
    connect(btop,           SIGNAL(valueChanged(qreal)),       this,    SLOT(slot_set_bounds(qreal)));
    connect(bwidth,         SIGNAL(valueChanged(qreal)),       this,    SLOT(slot_set_bounds(qreal)));
    connect(btheta,         SIGNAL(valueChanged(qreal)),       this,    SLOT(slot_set_bounds(qreal)));

    connect(dleft,          SIGNAL(valueChanged(qreal)),       this,    SLOT(slot_set_deltas(qreal)));
    connect(dtop,           SIGNAL(valueChanged(qreal)),       this,    SLOT(slot_set_deltas(qreal)));
    connect(dwidth,         SIGNAL(valueChanged(qreal)),       this,    SLOT(slot_set_deltas(qreal)));
    connect(dtheta,         SIGNAL(valueChanged(qreal)),       this,    SLOT(slot_set_deltas(qreal)));
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

    updateStylesWidget();
}

void  page_position::refreshPage()
{
    if (myBlockUpdates)
        return;
    onEnter();
}

void page_position::updateDesignWidget()
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    DesignPtr d = designs[0];

    scaleSliderSet->setValue(d->getDesignInfo().getDiameter()/2.0);

    xOff->setValue(d->getXoffset2());
    yOff->setValue(d->getYoffset2());
    xSep->setValue(d->getXseparation());
    ySep->setValue(d->getYseparation());
    QPointF pt = d->getDesignInfo().getStartTile();
    xStart->setValue((int)pt.x());
    yStart->setValue((int)pt.y());
}

void page_position::updateStylesWidget()
{
    Layer * layer = nullptr;
    QVector<Layer*> views = viewer->getActiveLayers();
    int num = views.size();
    if (views.size() == 0)
    {
        return;
    }

    indexBox->setRange(0,num-1);
    int sel = indexBox->value();

    layer = views[sel];
    layerDesc->setText(layer->getName());

    qreal degrees;

    Bounds b = layer->getBounds();
    degrees = qRadiansToDegrees(b.theta);
    bleft->setValue(b.left);
    btop->setValue(b.top);
    bwidth->setValue(b.width);
    btheta->setValue(degrees);

    b = layer->getDeltas();
    degrees = qRadiansToDegrees(b.theta);
    dleft->setValue(b.left);
    dtop->setValue(b.top);
    dwidth->setValue(b.width);
    dtheta->setValue(degrees);

    b = layer->getAdjustedBounds();
    degrees = qRadiansToDegrees(b.theta);
    aleft->setValue(b.left);
    atop->setValue(b.top);
    awidth->setValue(b.width);
    atheta->setValue(degrees);

    TransformPtr t = layer->getLayerTransform();
    transLabel->setText(t->toString());
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

void page_position::enterEvent(QEvent * event)
{
    Q_UNUSED(event);
    qDebug() << "enter";
    myBlockUpdates = true;
    QWidget::enterEvent(event);
}

void page_position::leaveEvent(QEvent * event)
{
    Q_UNUSED(event);
    qDebug() << "exit";
    myBlockUpdates = false;
    QWidget::leaveEvent(event);
}

void page_position::slot_set_bounds(qreal)
{
    QVector<Layer*> views = viewer->getActiveLayers();
    if (views.size() == 0)
        return;
    Layer * layer = views.first();

    qreal theta = qDegreesToRadians(btheta->value());
    Bounds b(bleft->value(), btop->value(), bwidth->value(), theta);
    layer->setBounds(b);
    layer->forceUpdateLayer();
}

void page_position::slot_set_deltas(qreal)
{
    QVector<Layer*> views = viewer->getActiveLayers();
    if (views.size() == 0)
        return;
    Layer * layer = views.first();

    qreal theta = qDegreesToRadians(dtheta->value());
    Bounds b(dleft->value(), dtop->value(), dwidth->value(), theta);
    layer->setDeltas(b);
    layer->forceUpdateLayer();
}

void page_position::slot_clear_deltas()
{
    QVector<Layer*> views = viewer->getActiveLayers();
    if (views.size() == 0)
        return;
    Layer * layer = views.first();
    Bounds b;
    layer->setDeltas(b);
    layer->forceUpdateLayer();
}

void page_position::slot_select(int)
{
    onEnter();
}
