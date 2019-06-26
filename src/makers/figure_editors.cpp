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

#include "figure_editors.h"
#include "figure_maker.h"
#include "base/tiledpatternmaker.h"
#include "tapp/Star.h"
#include "tapp/ExtendedStar.h"
#include "tapp/StarConnectFigure.h"
#include "tapp/RosetteConnectFigure.h"
#include "tapp/ExtendedRosette.h"
#include "FeatureButton.h"

// An abstract class for containing the controls related to the editing
// of one kind of figure.  A complex hierarchy of FigureEditors gets built
// up to become the changeable controls for editing figures in FigureMaker.

FigureEditor::FigureEditor(FigureMaker * editor, QString name)
{
    this->editor = editor;
    this->name   = name;

    vbox = new AQVBoxLayout();
    setLayout(vbox);

    boundarySides = new SliderSet("Boundary sides", 4, 1, 64);
    boundaryScale = new DoubleSliderSet("Boundary Scale", 1.0, 0.1, 4.0, 100 );
    figureScale   = new DoubleSliderSet("Figure Scale", 1.0, 0.1, 4.0, 100 );

    boundaryScale->setPrecision(8);
    figureScale->setPrecision(8);

    addLayout(boundarySides);
    addLayout(boundaryScale);
    addLayout(figureScale);

    connect(this,          &FigureEditor::sig_figure_changed, editor, &FigureMaker::slot_figureChanged, Qt::QueuedConnection);
    connect(boundaryScale, &DoubleSliderSet::valueChanged, this, &FigureEditor::updateGeometry, Qt::QueuedConnection);
    connect(figureScale,   &DoubleSliderSet::valueChanged, this, &FigureEditor::updateGeometry, Qt::QueuedConnection);
    connect(boundarySides, &SliderSet::valueChanged,       this, &FigureEditor::updateGeometry, Qt::QueuedConnection);
}

RadialEditor::RadialEditor(FigureMaker * editor, QString name) : FigureEditor(editor,name)
{
    radial_r = new DoubleSliderSet("Radial Rotation",0.0, -360.0, 360.0, 1);
    n = new SliderSet("Radial Points N", 8, 3, 64);

    addLayout(radial_r);
    addLayout(n);

    connect(radial_r, &DoubleSliderSet::valueChanged, this, &RadialEditor::updateGeometry, Qt::QueuedConnection);
    connect(n, &SliderSet::valueChanged,       this, &RadialEditor::updateGeometry, Qt::QueuedConnection);
}

StarEditor::StarEditor(FigureMaker * editor, QString name) : RadialEditor(editor,name)
{
    d = new DoubleSliderSet("Star Editor Hops D", 3.0, 1.0, 10.0, 100 );
    s = new SliderSet("Star Editor Intersects S", 2, 1, 5);

    addLayout(d);
    addLayout(s);

    connect(d, &DoubleSliderSet::valueChanged, this, &StarEditor::updateGeometry, Qt::QueuedConnection);
    connect(s, &SliderSet::valueChanged,       this, &StarEditor::updateGeometry, Qt::QueuedConnection);
}

FigurePtr StarEditor::getFigure()
{
    return star;
}

void StarEditor::resetWithFigure( FigurePtr figure )
{
    if (!figure)
    {
        star.reset();
        return;
    }

    if (figure->getFigType() == FIG_TYPE_STAR)
    {
        star = std::dynamic_pointer_cast<Star>(figure);
    }
    else
    {
        int n=6;  // default
        RadialPtr rp = std::dynamic_pointer_cast<RadialFigure>(figure);
        if (rp)
        {
            n = rp->getN();
        }
        star = make_shared<Star>(*figure.get(), n, n <=6 ? n / 3.0 : 3.0, 2 );
    }

    updateLimits();
    updateGeometry();
}

void StarEditor::updateLimits()
{
    if (star)
    {
        int    bs = star->getExtBoundarySides();
        qreal  sc = star->getExtBoundaryScale();
        qreal  fs = star->getFigureScale();
        int    nn = star->getN();
        qreal  dd = star->getD();
        int    ss = star->getS();
        double rr = star->getR();

        //double dmax = 0.5 * (double)nn;
        blockSignals(true);
        boundarySides->setValues(bs, 1, 64);
        boundaryScale->setValues(sc, 0.1, 4.0);
        figureScale->setValues(fs, 0.1, 4.0);
        n->setValues(nn,3,64);
        d->setValues( dd, 1.0, qreal(nn*2));
        s->setValues( ss, 1.0, nn * 2);
        radial_r->setValues(rr,-360.0,360.0);
        blockSignals(false);
    }
}

void StarEditor::updateGeometry()
{
    if (star)
    {
        int sides     = boundarySides->value();
        qreal bscale  = boundaryScale->value();
        qreal fscale  = figureScale->value();
        int nval      = n->value();
        qreal dval    = d->value();
        int sval      = s->value();
        qreal  rot    = radial_r->value();

        blockSignals(true);
        star->setExtBoundarySides(sides);
        star->setExtBoundaryScale(bscale);
        star->setFigureScale(fscale);
        star->setN(nval);
        star->setD(dval);
        star->setS(sval);
        star->setR(rot);
        star->resetMaps();
        blockSignals(false);

        emit sig_figure_changed();
    }
}

// ConnectStarEditor
ConnectStarEditor::ConnectStarEditor(FigureMaker *  editor, QString name) : StarEditor(editor,name)
{
    defaultBtn = new QPushButton("Calc Scale");
    defaultBtn->setFixedWidth(131);

    addWidget(defaultBtn);

    QObject::connect(defaultBtn, &QPushButton::clicked,  this, &ConnectStarEditor::calcScale);
}

FigurePtr ConnectStarEditor::getFigure()
{
    return starConnect;
}

void ConnectStarEditor::resetWithFigure(FigurePtr figure)
{
    if (!figure)
    {
        starConnect.reset();
        return;
    }

    starConnect = std::dynamic_pointer_cast<StarConnectFigure>(figure);
    if (!starConnect)
    {
        int n=6;  // default
        RadialPtr rp = std::dynamic_pointer_cast<RadialFigure>(figure);
        if (rp)
        {
            n = rp->getN();
        }
        starConnect = make_shared<StarConnectFigure>(*figure.get(),n, n <= 6 ? n / 3.0 : 3.0, 2);
    }

    starConnect->setFigureScale(starConnect->computeConnectScale());

    updateLimits();
    updateGeometry();
}

void ConnectStarEditor::updateLimits()
{
    if (starConnect)
    {
        int bsides   = starConnect->getExtBoundarySides();
        qreal bscale = starConnect->getExtBoundaryScale();
        qreal fscale = starConnect->getFigureScale();

        int nn   = starConnect->getN();
        qreal dd = starConnect->getD();
        int ss   = starConnect->getS();
        qreal rr = starConnect->getR();

        blockSignals(true);

        // these dont call set geometry
        boundarySides->setValues(bsides, 1, 64);
        boundaryScale->setValues(bscale, 0.1, 4.0);
        figureScale->setValues(fscale, 0.1, 4.0);

        // these do
        n->setValues(nn, 3,64);
        d->setValues(dd, 1.0, qreal(nn*2));
        s->setValues(ss, 1.0, nn * 2);
        radial_r->setValues(rr,-360.0,360.0);

        blockSignals(false);
    }
}

void ConnectStarEditor::updateGeometry()
{
    if (starConnect)
    {

        int nval        = n->value();
        qreal dval      = d->value();
        int sval        = s->value();
        qreal rval      = radial_r->value();

        int  bSides     = boundarySides->value();
        qreal bScale    = boundaryScale->value();
        qreal figScale  = figureScale->value();

        blockSignals(true);

        // these dont call set geometry
        starConnect->setExtBoundarySides(bSides);
        starConnect->setExtBoundaryScale(bScale);
        starConnect->setFigureScale(figScale);

        // these do
        starConnect->setN(nval);
        starConnect->setD(dval);
        starConnect->setS(sval);
        starConnect->setR(rval);
        starConnect->resetMaps();
        blockSignals(false);

        emit sig_figure_changed();
    }
}

void ConnectStarEditor::calcScale()
{
    qreal scale = starConnect->computeConnectScale();
    figureScale->setValue(scale);
    updateGeometry();
}

// ExtendedStarEditor
ExtendedStarEditor::ExtendedStarEditor(FigureMaker *  editor, QString name) : StarEditor(editor,name)
{
    extendBox1 = new QCheckBox("Extend Peripheral Vertices");
    extendBox2 = new QCheckBox("Extend Free Vertices");

    addWidget(extendBox1);
    addWidget(extendBox2);

    connect(extendBox1,    &QCheckBox::clicked,            this, &ExtendedStarEditor::updateGeometry, Qt::QueuedConnection);
    connect(extendBox2,    &QCheckBox::clicked,            this, &ExtendedStarEditor::updateGeometry, Qt::QueuedConnection);
}

FigurePtr ExtendedStarEditor::getFigure()
{
    return extended;
}

void ExtendedStarEditor::resetWithFigure(FigurePtr figure)
{
    if (!figure)
    {
        extended.reset();
        return;
    }

    extended = std::dynamic_pointer_cast<ExtendedStar>(figure);
    if (!extended)
    {
        int n=6;  // default
        RadialPtr rp = std::dynamic_pointer_cast<RadialFigure>(figure);
        if (rp)
        {
            n = rp->getN();
        }
        extended = make_shared<ExtendedStar>(*figure.get(), n, n <= 6 ? n / 3.0 : 3.0, 2);
    }

    updateLimits();
    updateGeometry();
}


void ExtendedStarEditor::updateLimits()
{
    if (extended)
    {
        int bsides   = extended->getExtBoundarySides();
        qreal bscale = extended->getExtBoundaryScale();
        qreal fscale = extended->getFigureScale();
        bool ext_t   = extended->getExtendPeripheralVertices();
        bool ext_nt  = extended->getExtendFreeVertices();

        int nn   = extended->getN();
        qreal dd = extended->getD();
        int ss   = extended->getS();
        qreal rr = extended->getR();

        blockSignals(true);

        // these dont call set geometry
        boundarySides->setValues(bsides, 1, 64);
        boundaryScale->setValues(bscale, 0.1, 4.0);
        figureScale->setValues(fscale, 0.1, 4.0);
        extendBox1->setChecked(ext_t);
        extendBox2->setChecked(ext_nt);

        // these do
        n->setValues(nn, 3,64);
        d->setValues(dd, 1.0, qreal(nn*2));
        s->setValues(ss, 1.0, nn * 2);
        radial_r->setValues(rr,-360.0,360.0);

        blockSignals(false);
    }
}

void ExtendedStarEditor::updateGeometry()
{
    if (extended)
    {
        int nval        = n->value();
        qreal dval      = d->value();
        int sval        = s->value();
        qreal rval      = radial_r->value();

        int  bSides     = boundarySides->value();
        qreal bScale    = boundaryScale->value();
        qreal figScale  = figureScale->value();
        bool extendPeripheralVertices = extendBox1->isChecked();
        bool extendFreeVertices       = extendBox2->isChecked();

        blockSignals(true);

        // these dont call set geometry
        extended->setExtBoundarySides(bSides);
        extended->setExtBoundaryScale(bScale);
        extended->setFigureScale(figScale);
        extended->setExtendPeripheralVertices(extendPeripheralVertices);
        extended->setExtendFreeVertices(extendFreeVertices);

        // these do
        extended->setN(nval);
        extended->setD(dval);
        extended->setS(sval);
        extended->setR(rval);
        extended->resetMaps();
        blockSignals(false);

        emit sig_figure_changed();
    }
}


// The controls for editing a Star.  Glue code, just like RosetteEditor.
// DAC - Actually the comment above is not true

RosetteEditor::RosetteEditor(FigureMaker * editor, QString name) : RadialEditor(editor,name)
{
    q = new DoubleSliderSet("RosetteEditor Q (Tip Angle)", 0.0, -3.0, 3.0, 100 );
    k = new DoubleSliderSet("RosetteEditor K (Neck Angle)", 0.0, -3.0, 3.0, 1000 );
    k->setPrecision(4);
    s = new SliderSet("RosetteEditor S (Sides Intersects", 1, 1, 5);

    addLayout(q);
    addLayout(k);
    addLayout(s);

    connect(q, &DoubleSliderSet::valueChanged, this, &RosetteEditor::updateGeometry, Qt::QueuedConnection);
    connect(k, &DoubleSliderSet::valueChanged, this, &RosetteEditor::updateGeometry, Qt::QueuedConnection);
    connect(s, &SliderSet::valueChanged, this, &RosetteEditor::updateGeometry, Qt::QueuedConnection);
}

FigurePtr RosetteEditor::getFigure()
{
    return rosette;
}

void RosetteEditor::resetWithFigure( FigurePtr figure )
{
    if (!figure)
    {
        rosette.reset();
        return;
    }

    if (figure->getFigType() == FIG_TYPE_ROSETTE)
    {
        rosette = std::dynamic_pointer_cast<Rosette>(figure);
    }
    else
    {
        int n=6;  // default
        RadialPtr rp = std::dynamic_pointer_cast<RadialFigure>(figure);
        if (rp)
        {
            n = rp->getN();
        }
        rosette = make_shared<Rosette>(*figure.get(), n, 0.0, 3);
    }

    updateLimits();
    updateGeometry();
}

void RosetteEditor::updateLimits()
{
    if( rosette)
    {
        int    bs = rosette->getExtBoundarySides();
        qreal  sc = rosette->getExtBoundaryScale();
        qreal  fs = rosette->getFigureScale();
        int    nn = rosette->getN();
        double qq = rosette->getQ();
        double kk = rosette->getK();
        double rr = rosette->getR();
        int    ss = rosette->getS();

        blockSignals(true);
        boundarySides->setValues(bs, 1, 64);
        boundaryScale->setValues(sc, 0.1, 4.0);
        figureScale->setValues(fs, 0.1, 4.0);
        n->setValues(nn,3,64);
        q->setValues(qq, -3.0, 3.0);       // DAC was -1.0, 1.0
        s->setValues(ss, 1.0, 5);
        k->setValues(kk,-3.0, 3.0);
        radial_r->setValues(rr,-360.0,360.0);
        blockSignals(false);
    }
}

void RosetteEditor::updateGeometry()
{
    if(rosette)
    {
        int sides     = boundarySides->value();
        qreal bscale  = boundaryScale->value();
        qreal fscale  = figureScale->value();
        qreal  qval = q->value();
        qreal  kval = k->value();
        int    sval = s->value();
        int    nval = n->value();
        qreal  rot  = radial_r->value();

        blockSignals(true);
        rosette->setExtBoundarySides(sides);
        rosette->setExtBoundaryScale(bscale);
        rosette->setFigureScale(fscale);
        rosette->setN(nval);
        rosette->setQ(qval);
        rosette->setS(sval);
        rosette->setK(kval);
        rosette->setR(rot);
        rosette->resetMaps();
        blockSignals(false);

        emit sig_figure_changed();
    }
}

// ConnectRosetteEditor

ConnectRosetteEditor::ConnectRosetteEditor(FigureMaker *  editor, QString name) : RosetteEditor(editor,name)
{
    defaultBtn = new QPushButton("Calc Scale");
    defaultBtn->setFixedWidth(131);

    addWidget(defaultBtn);

    QObject::connect(defaultBtn, &QPushButton::clicked,  this, &ConnectRosetteEditor::calcScale);
}

FigurePtr ConnectRosetteEditor::getFigure()
{
    return rosetteConnect;
}

void ConnectRosetteEditor::resetWithFigure(FigurePtr figure)
{
    if (!figure)
    {
        rosetteConnect.reset();
        return;
    }

    rosetteConnect = std::dynamic_pointer_cast<RosetteConnectFigure>(figure);
    if (!rosetteConnect)
    {
        int n=6;  // default
        RadialPtr rp = std::dynamic_pointer_cast<RadialFigure>(figure);
        if (rp)
        {
            n = rp->getN();
        }
        rosetteConnect = make_shared<RosetteConnectFigure>(* figure.get(), n, 0.0, 3);
    }

    rosetteConnect->setFigureScale(rosetteConnect->computeConnectScale());

    updateLimits();
    updateGeometry();
}

void ConnectRosetteEditor::updateLimits()
{
    if (rosetteConnect)
    {
        int    bs = rosetteConnect->getExtBoundarySides();
        qreal  sc = rosetteConnect->getExtBoundaryScale();
        qreal  fs = rosetteConnect->getFigureScale();
        int    nn = rosetteConnect->getN();
        double qq = rosetteConnect->getQ();
        double kk = rosetteConnect->getK();
        double rr = rosetteConnect->getR();
        int    ss = rosetteConnect->getS();

        blockSignals(true);
        boundarySides->setValues(bs, 1, 64);
        boundaryScale->setValues(sc, 0.1, 4.0);
        figureScale->setValues(fs, 0.1, 4.0);
        n->setValues(nn,3,64);
        q->setValues(qq, -3.0, 3.0);       // DAC was -1.0, 1.0
        s->setValues(ss, 1.0, 5);
        k->setValues(kk,-3.0, 3.0);
        radial_r->setValues(rr,-360.0,360.0);
        blockSignals(false);
    }
}


void ConnectRosetteEditor::updateGeometry()
{
   if (rosetteConnect)
   {
       int sides     = boundarySides->value();
       qreal bscale  = boundaryScale->value();
       qreal fscale  = figureScale->value();
       qreal  qval = q->value();
       qreal  kval = k->value();
       int    sval = s->value();
       int    nval = n->value();
       qreal  rot  = radial_r->value();

       blockSignals(true);
       rosetteConnect->setExtBoundarySides(sides);
       rosetteConnect->setExtBoundaryScale(bscale);
       rosetteConnect->setFigureScale(fscale);
       rosetteConnect->setN(nval);
       rosetteConnect->setQ(qval);
       rosetteConnect->setS(sval);
       rosetteConnect->setK(kval);
       rosetteConnect->setR(rot);
       rosetteConnect->resetMaps();
       blockSignals(false);

       emit sig_figure_changed();
    }
    emit sig_figure_changed();
}

void ConnectRosetteEditor::calcScale()
{
    if (rosetteConnect)
    {
        qreal scale = rosetteConnect->computeConnectScale();
        figureScale->setValue(scale);
        updateGeometry();
    }
}


// ExtendedRosetteEditor

ExtendedRosetteEditor::ExtendedRosetteEditor(FigureMaker *  editor, QString name) : RosetteEditor(editor,name)
{
    extendPeriphBox    = new QCheckBox("Extend PeripheralVertices");
    extendFreeBox      = new QCheckBox("Extend Free Vertices");
    connectBoundaryBox = new QCheckBox("Connect Boundary Vertices");

    addWidget(extendPeriphBox);
    addWidget(extendFreeBox);
    addWidget(connectBoundaryBox);

    connect(extendPeriphBox,    &QCheckBox::clicked,       this, &ExtendedRosetteEditor::updateGeometry, Qt::QueuedConnection);
    connect(extendFreeBox,      &QCheckBox::clicked,       this, &ExtendedRosetteEditor::updateGeometry, Qt::QueuedConnection);
    connect(connectBoundaryBox, &QCheckBox::clicked,       this, &ExtendedRosetteEditor::updateGeometry, Qt::QueuedConnection);
}

FigurePtr ExtendedRosetteEditor::getFigure()
{
    return extended;
}

void ExtendedRosetteEditor::resetWithFigure(FigurePtr figure)
{
    if (!figure)
    {
        extended.reset();
        return;
    }

    extended = std::dynamic_pointer_cast<ExtendedRosette>(figure);
    if (!extended)
    {
        int n=6;  // default
        RadialPtr rp = std::dynamic_pointer_cast<RadialFigure>(figure);
        if (rp)
        {
            n = rp->getN();
        }
        extended = make_shared<ExtendedRosette>(*figure.get(), n, 0.0, 3);
    }

    updateLimits();
    updateGeometry();
}


void ExtendedRosetteEditor::updateLimits()
{
    if (extended)
    {
        int bsides   = extended->getExtBoundarySides();
        qreal bscale = extended->getExtBoundaryScale();
        qreal fscale = extended->getFigureScale();
        bool ext_t   = extended->getExtendPeripheralVertices();
        bool ext_nt  = extended->getExtendFreeVertices();
        bool con_bd  = extended->getConnectBoundaryVertices();

        int nn   = extended->getN();
        qreal qq = extended->getQ();
        int ss   = extended->getS();
        qreal kk = extended->getK();
        qreal rr = extended->getR();

        // these dont call set geometry
        boundarySides->setValues(bsides, 1, 64);
        boundaryScale->setValues(bscale, 0.1, 4.0);
        figureScale->setValues(fscale, 0.1, 4.0);
        extendPeriphBox->setChecked(ext_t);
        extendFreeBox->setChecked(ext_nt);
        connectBoundaryBox->setChecked(con_bd);

        // these do
        n->setValues(nn,3,64);
        q->setValues(qq, -3.0, 3.0);
        s->setValues(ss, 1, 5);
        k->setValues(kk,-3.0,3.0);
        radial_r->setValues(rr,-360.0,360.0);
    }
}

void ExtendedRosetteEditor::updateGeometry()
{
    if (extended)
    {
        int nval                = n->value();
        qreal qval              = q->value();
        qreal kval              = k->value();
        qreal rot               = radial_r->value();
        int sval                = s->value();

        int  bSides             = boundarySides->value();
        qreal bScale            = boundaryScale->value();
        qreal figScale          = figureScale->value();
        bool extendPeripherals  = extendPeriphBox->isChecked();
        bool extendFreeVertices = extendFreeBox->isChecked();
        bool connectBoundary    = connectBoundaryBox->isChecked();

        // these don't call set geometry
        extended->setExtBoundarySides(bSides);
        extended->setExtBoundaryScale(bScale);
        extended->setFigureScale(figScale);
        extended->setExtendPeripheralVertices(extendPeripherals);
        extended->setExtendFreeVertices(extendFreeVertices);
        extended->setConnectBoundaryVertices(connectBoundary);

        // these do
        extended->setN(nval);
        extended->setQ(qval);
        extended->setS(sval);
        extended->setK(kval);
        extended->setR(rot);
        extended->resetMaps();
        emit sig_figure_changed();
    }
}



