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

#include "makers/motif_maker/figure_editors.h"
#include "makers/motif_maker/motif_maker.h"
#include "base/tiledpatternmaker.h"
#include "tapp/star.h"
#include "tapp/extended_star.h"
#include "tapp/star_connect_figure.h"
#include "tapp/rosette_connect_figure.h"
#include "tapp/extended_rosette.h"
#include "makers/motif_maker/feature_button.h"
#include "panels/page_motif_maker.h"

using std::dynamic_pointer_cast;

// An abstract class for containing the controls related to the editing
// of one kind of figure.  A complex hierarchy of FigureEditors gets built
// up to become the changeable controls for editing figures in FigureMaker.

FigureEditor::FigureEditor(page_motif_maker * fm, QString figname)
{
    menu = fm;
    name     = figname;

    vbox = new AQVBoxLayout();
    setLayout(vbox);

    boundarySides = new SliderSet("Boundary sides", 4, 1, 64);
    boundaryScale = new DoubleSliderSet("Boundary Scale", 1.0, 0.1, 4.0, 100 );
    figureScale   = new DoubleSliderSet("Figure Scale", 1.0, 0.1, 4.0, 100 );
    figureRotate  = new DoubleSliderSet("Figure Rotation",0.0, -360.0, 360.0, 1);

    boundaryScale->setPrecision(8);
    figureScale->setPrecision(8);

    addLayout(boundarySides);
    addLayout(boundaryScale);
    addLayout(figureScale);
    addLayout(figureRotate);

    connect(this,          &FigureEditor::sig_figure_changed, menu, &page_motif_maker::slot_figureChanged); //, Qt::QueuedConnection);

    connect(boundaryScale, &DoubleSliderSet::valueChanged, this, &FigureEditor::updateGeometry);
    connect(boundarySides, &SliderSet::valueChanged,       this, &FigureEditor::updateGeometry);
    connect(figureScale,   &DoubleSliderSet::valueChanged, this, &FigureEditor::updateGeometry);
    connect(figureRotate,  &DoubleSliderSet::valueChanged, this, &FigureEditor::updateGeometry);
}

void FigureEditor::resetWithFigure(FigurePtr fig, bool doEmit)
{
    Q_UNUSED(doEmit);
    figure = fig;
}

void FigureEditor::updateLimits()
{
    if (!figure)
        return;

    int    bs = figure->getExtBoundarySides();
    qreal  sc = figure->getExtBoundaryScale();
    qreal  fs = figure->getFigureScale();
    qreal  rr = figure->getFigureRotate();

    blockSignals(true);
    boundarySides->setValues(bs, 1, 64);
    boundaryScale->setValues(sc, 0.1, 4.0);
    figureScale->setValues(fs, 0.1, 4.0);
    figureRotate->setValues(rr,-360.0,360.0);
    blockSignals(false);
}

void FigureEditor::updateGeometry(bool doEmit)
{
    if (!figure)
        return;

    int sides     = boundarySides->value();
    qreal bscale  = boundaryScale->value();
    qreal fscale  = figureScale->value();
    qreal  rot    = figureRotate->value();

    blockSignals(true);
    figure->setExtBoundarySides(sides);
    figure->setExtBoundaryScale(bscale);
    figure->setFigureScale(fscale);
    figure->setFigureRotate(rot);
    blockSignals(false);

    if (doEmit)
        emit sig_figure_changed(figure);
}

StarEditor::StarEditor(page_motif_maker *fm, QString figname) : FigureEditor(fm,figname)
{
    n_slider = new SliderSet("Radial Points N", 8, 3, 64);
    d_slider = new DoubleSliderSet("Star Editor Hops D", 3.0, 1.0, 10.0, 100 );
    s_slider = new SliderSet("Star Editor Intersects S", 2, 1, 5);

    addLayout(n_slider);
    addLayout(d_slider);
    addLayout(s_slider);

    connect(n_slider, &SliderSet::valueChanged,       this, &StarEditor::updateGeometry);
    connect(d_slider, &DoubleSliderSet::valueChanged, this, &StarEditor::updateGeometry);
    connect(s_slider, &SliderSet::valueChanged,       this, &StarEditor::updateGeometry);
}

FigurePtr StarEditor::getFigure()
{
    return star;
}

void StarEditor::resetWithFigure(FigurePtr fig, bool doEmit)
{
    if (!fig)
    {
        star.reset();
        return;
    }

    star = dynamic_pointer_cast<Star>(fig);
    if (!star)
    {
        int n = 6;  // default
        qreal rotate = 0.0;
        RadialPtr rp = dynamic_pointer_cast<RadialFigure>(fig);
        if (rp)
        {
            n = rp->getN();
            rotate = rp->getFigureRotate();
        }
        star = make_shared<Star>(*fig.get(), n, n <=6 ? n / 3.0 : 3.0, 2, rotate);
    }

    Q_ASSERT(star);
    FigureEditor::resetWithFigure(star,false);

    updateLimits();
    updateGeometry(doEmit);
}

void StarEditor::updateLimits()
{
    if (star)
    {
        FigureEditor::updateLimits();

        int    nn = star->getN();
        qreal  dd = star->getD();
        int    ss = star->getS();

        //double dmax = 0.5 * (double)nn;
        blockSignals(true);
        n_slider->setValues(nn,3,64);
        d_slider->setValues( dd, 1.0, static_cast<qreal>(nn) * 2.0);
        s_slider->setValues( ss, 1.0, nn * 2);
        blockSignals(false);
    }
}

void StarEditor::updateGeometry(bool doEmit)
{
    if (star)
    {
        FigureEditor::updateGeometry(false);

        int nval      = n_slider->value();
        qreal dval    = d_slider->value();
        int sval      = s_slider->value();

        blockSignals(true);
        star->setN(nval);
        star->setD(dval);
        star->setS(sval);
        star->resetMaps();
        blockSignals(false);

        if (doEmit)
            emit sig_figure_changed(star);
    }
}

// The controls for editing a Star.  Glue code, just like RosetteEditor.
// DAC - Actually the comment above is not true

RosetteEditor::RosetteEditor(page_motif_maker * fm, QString figname) : FigureEditor(fm,figname)
{
    n_slider = new SliderSet("Radial Points N", 8, 3, 64);
    q_slider = new DoubleSliderSet("RosetteEditor Q (Tip Angle)", 0.0, -3.0, 3.0, 100 );
    k_slider = new DoubleSliderSet("RosetteEditor K (Neck Angle)", 0.0, -3.0, 3.0, 1000 );
    k_slider->setPrecision(4);
    s_slider = new SliderSet("RosetteEditor S (Sides Intersections)", 1, 1, 5);

    addLayout(n_slider);
    addLayout(q_slider);
    addLayout(k_slider);
    addLayout(s_slider);

    connect(n_slider, &SliderSet::valueChanged,       this, &RosetteEditor::updateGeometry, Qt::QueuedConnection);
    connect(q_slider, &DoubleSliderSet::valueChanged, this, &RosetteEditor::updateGeometry, Qt::QueuedConnection);
    connect(k_slider, &DoubleSliderSet::valueChanged, this, &RosetteEditor::updateGeometry, Qt::QueuedConnection);
    connect(s_slider, &SliderSet::valueChanged,       this, &RosetteEditor::updateGeometry, Qt::QueuedConnection);
}

FigurePtr RosetteEditor::getFigure()
{
    return rosette;
}

void RosetteEditor::resetWithFigure(FigurePtr fig, bool doEmit)
{

    if (!fig)
    {
        rosette.reset();
        return;
    }

    rosette = dynamic_pointer_cast<Rosette>(fig);
    if (!rosette)
    {
        int n = 6;  // default
        qreal rotate = 0.0;
        RadialPtr rp = dynamic_pointer_cast<RadialFigure>(fig);
        if (rp)
        {
            n = rp->getN();
            rotate = rp->getFigureRotate();
        }
        rosette = make_shared<Rosette>(*fig.get(), n, 0.0, 3, 0.0, rotate);
    }

    Q_ASSERT(rosette);
    FigureEditor::resetWithFigure(rosette,false);
    updateLimits();
    updateGeometry(doEmit);
}

void RosetteEditor::updateLimits()
{
    if( rosette)
    {
        FigureEditor::updateLimits();

        int    nn = rosette->getN();
        double qq = rosette->getQ();
        double kk = rosette->getK();
        int    ss = rosette->getS();

        blockSignals(true);
        n_slider->setValues(nn,3,64);
        q_slider->setValues(qq, -3.0, 3.0);       // DAC was -1.0, 1.0
        s_slider->setValues(ss, 1.0, 5);
        k_slider->setValues(kk,-3.0, 3.0);
        blockSignals(false);
    }
}

void RosetteEditor::updateGeometry(bool doEmit)
{
    if(rosette)
    {
        FigureEditor::updateGeometry(false);

        qreal  qval = q_slider->value();
        qreal  kval = k_slider->value();
        int    sval = s_slider->value();
        int    nval = n_slider->value();

        blockSignals(true);
        rosette->setN(nval);
        rosette->setQ(qval);
        rosette->setS(sval);
        rosette->setK(kval);
        rosette->resetMaps();
        blockSignals(false);

        if (doEmit)
            emit sig_figure_changed(rosette);
    }
}

// ConnectStarEditor
ConnectStarEditor::ConnectStarEditor(page_motif_maker *fm, QString figname) : StarEditor(fm,figname)
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

void ConnectStarEditor::resetWithFigure(FigurePtr fig, bool doEmit)
{
    if (!fig)
    {
        starConnect.reset();
        return;
    }

    starConnect = dynamic_pointer_cast<StarConnectFigure>(fig);
    if (!starConnect)
    {
        StarPtr sp = dynamic_pointer_cast<Star>(fig);
        if (sp)
        {
            starConnect = make_shared<StarConnectFigure>(*fig.get(),sp->getN(),sp->getD(),sp->getS(),sp->getFigureRotate());
        }
        else
        {
            int n = 6;  // default
            qreal rotate = 0.0;
            RadialPtr rp = dynamic_pointer_cast<RadialFigure>(fig);
            if (rp)
            {
                n = rp->getN();
                rotate = rp->getFigureRotate();
            }
            starConnect = make_shared<StarConnectFigure>(*fig.get(),n, n <= 6 ? n / 3.0 : 3.0, 2, rotate);
        }
    }

    Q_ASSERT(starConnect);
    StarEditor::resetWithFigure(starConnect,false);

    starConnect->setFigureScale(starConnect->computeConnectScale());

    updateLimits();
    updateGeometry(doEmit);
}

void ConnectStarEditor::calcScale()
{
    qreal scale = starConnect->computeConnectScale();
    figureScale->setValue(scale);
    updateGeometry(true);
}

// ConnectRosetteEditor

ConnectRosetteEditor::ConnectRosetteEditor(page_motif_maker * fm, QString figname) : RosetteEditor(fm,figname)
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

void ConnectRosetteEditor::resetWithFigure(FigurePtr fig, bool doEmit)
{
    if (!fig)
    {
        rosetteConnect.reset();
        return;
    }

    rosetteConnect = dynamic_pointer_cast<RosetteConnectFigure>(fig);
    if (!rosetteConnect)
    {

        RosettePtr rsp = dynamic_pointer_cast<Rosette>(fig);
        if (rsp)
        {
            rosetteConnect = make_shared<RosetteConnectFigure>(*fig.get(),rsp->getN(),rsp->getQ(),
                                                    rsp->getS(),rsp->getK(), rsp->getFigureRotate());
        }
        else
        {
            int n = 6;  // default
            qreal rotate = 0.0;
            RadialPtr rp = dynamic_pointer_cast<RadialFigure>(fig);
            if (rp)
            {
                n = rp->getN();
                rotate = rp->getFigureRotate();
            }
            rosetteConnect = make_shared<RosetteConnectFigure>(* fig.get(), n, 0.0, 3, 0.0, rotate);
        }
    }

    Q_ASSERT(rosetteConnect);
    RosetteEditor::resetWithFigure(rosetteConnect,false);

    rosetteConnect->setFigureScale(rosetteConnect->computeConnectScale());

    updateLimits();
    updateGeometry(doEmit);
}

void ConnectRosetteEditor::calcScale()
{
    if (rosetteConnect)
    {
        qreal scale = rosetteConnect->computeConnectScale();
        figureScale->setValue(scale);
        updateGeometry(true);
    }
}

// ExtendedStarEditor
ExtendedStarEditor::ExtendedStarEditor(page_motif_maker *fm, QString figname) : StarEditor(fm,figname)
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

void ExtendedStarEditor::resetWithFigure(FigurePtr fig, bool doEmit)
{
    if (!fig)
    {
        extended.reset();
        return;
    }

    extended = dynamic_pointer_cast<ExtendedStar>(fig);
    if (!extended)
    {
        StarPtr sp = dynamic_pointer_cast<Star>(fig);
        if (sp)
        {
            extended = make_shared<ExtendedStar>(*fig.get(),sp->getN(),sp->getD(),sp->getS(),sp->getFigureRotate());
        }
        else
        {
            int  n = 6;  // default
            qreal rotate = 0.0;
            RadialPtr rp = dynamic_pointer_cast<RadialFigure>(fig);
            if (rp)
            {
                n = rp->getN();
                rotate = rp->getFigureRotate();
            }
            extended = make_shared<ExtendedStar>(*fig.get(), n, n <= 6 ? n / 3.0 : 3.0, 2, rotate);
        }
    }

    Q_ASSERT(extended);
    StarEditor::resetWithFigure(extended,false);

    updateLimits();
    updateGeometry(doEmit);
}

void ExtendedStarEditor::updateLimits()
{
    if (extended)
    {
        bool ext_t   = extended->getExtendPeripheralVertices();
        bool ext_nt  = extended->getExtendFreeVertices();

        blockSignals(true);
        extendBox1->setChecked(ext_t);
        extendBox2->setChecked(ext_nt);
        blockSignals(false);

        StarEditor::updateLimits();
    }
}

void ExtendedStarEditor::updateGeometry(bool doEmit)
{
    if (extended)
    {
        bool extendPeripheralVertices = extendBox1->isChecked();
        bool extendFreeVertices       = extendBox2->isChecked();

        blockSignals(true);
        extended->setExtendPeripheralVertices(extendPeripheralVertices);
        extended->setExtendFreeVertices(extendFreeVertices);
        blockSignals(false);

        StarEditor::updateGeometry(false);

        if (doEmit)
            emit sig_figure_changed(extended);
    }
}

// ExtendedRosetteEditor

ExtendedRosetteEditor::ExtendedRosetteEditor(page_motif_maker * fm, QString figname) : RosetteEditor(fm,figname)
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

void ExtendedRosetteEditor::resetWithFigure(FigurePtr fig, bool doEmit)
{
    if (!fig)
    {
        extended.reset();
        return;
    }

    extended = dynamic_pointer_cast<ExtendedRosette>(fig);
    if (!extended)
    {
        RosettePtr rsp = dynamic_pointer_cast<Rosette>(fig);
        if (rsp)
        {
            extended = make_shared<ExtendedRosette>(*fig.get(),rsp->getN(),rsp->getQ(),
                                                    rsp->getS(),rsp->getK(), rsp->getFigureRotate());
        }
        else
        {
            int n = 6;  // default
            qreal rotate = 0.0;
            RadialPtr rp = std::dynamic_pointer_cast<RadialFigure>(fig);
            if (rp)
            {
                n = rp->getN();
                rotate = rp->getFigureRotate();
            }
            extended = make_shared<ExtendedRosette>(*fig.get(), n, 0.0, 3, 0, rotate);
        }
    }

    Q_ASSERT(extended);
    RosetteEditor::resetWithFigure(extended,false);

    updateLimits();
    updateGeometry(doEmit);
}


void ExtendedRosetteEditor::updateLimits()
{
    if (extended)
    {
        bool ext_t   = extended->getExtendPeripheralVertices();
        bool ext_nt  = extended->getExtendFreeVertices();
        bool con_bd  = extended->getConnectBoundaryVertices();

        blockSignals(true);
        extendPeriphBox->setChecked(ext_t);
        extendFreeBox->setChecked(ext_nt);
        connectBoundaryBox->setChecked(con_bd);
        blockSignals(false);

        RosetteEditor::updateLimits();
    }
}

void ExtendedRosetteEditor::updateGeometry(bool doEmit)
{
    if (extended)
    {
        bool extendPeripherals  = extendPeriphBox->isChecked();
        bool extendFreeVertices = extendFreeBox->isChecked();
        bool connectBoundary    = connectBoundaryBox->isChecked();

        blockSignals(true);
        extended->setExtendPeripheralVertices(extendPeripherals);
        extended->setExtendFreeVertices(extendFreeVertices);
        extended->setConnectBoundaryVertices(connectBoundary);
        blockSignals(false);

        RosetteEditor::updateGeometry(false);

        if (doEmit)
            emit sig_figure_changed(extended);
    }
}



