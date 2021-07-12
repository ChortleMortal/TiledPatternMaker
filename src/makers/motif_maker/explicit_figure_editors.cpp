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

#include "explicit_figure_editors.h"
#include "tapp/star.h"
#include "tapp/explicit_figure.h"
#include "tile/feature.h"
#include "makers/motif_maker/motif_maker.h"
#include "panels/layout_sliderset.h"
#include "panels/page_motif_maker.h"

typedef std::shared_ptr<RadialFigure>    RadialPtr;

ExplicitEditor::ExplicitEditor(page_motif_maker *ed, QString aname) : FigureEditor(ed, aname)
{
}

FigurePtr ExplicitEditor::getFigure()
{
    return explicitFig;
}

void ExplicitEditor::resetWithFigure(FigurePtr fig, bool doEmit)
{
    if (!fig)
    {
        explicitFig.reset();
        return;
    }

    qDebug() << "ExplicitEditor::resetWithFigure" << fig.get() << "  " << fig->getFigTypeString();

    explicitFig = resetFigure(fig,FIG_TYPE_EXPLICIT);

    updateEditor();
    updateFigure(doEmit);
}

ExplicitPtr ExplicitEditor::resetFigure(FigurePtr fig, eFigType figType)
{
    ExplicitPtr ep = std::dynamic_pointer_cast<ExplicitFigure>(fig);
    if (ep)
    {
        ep->setFigType(figType);
    }
    else
    {
        int sides = 10;
        RadialPtr rp = std::dynamic_pointer_cast<RadialFigure>(fig);
        if (rp)
        {
            sides = rp->getN();
        }
        MapPtr map = fig->getFigureMap();
        ep = std::make_shared<ExplicitFigure>(*fig.get(),map,figType, sides);
    }

    Q_ASSERT(ep);
    FigureEditor::resetWithFigure(ep,false);

    return ep;
}

////////////////////////////////////////////////////////////////////////////
//
// ExplicitGirihEditor.java
//
// The editing controls for explicit figures.  A simple class, because
// (right now) explicit figures don't have any editing controls.  All
// you can do is ask the figure to be inferred from the rest of the
// Prototype.  So all we need is one button.
//
// If I have time (I've got about 36 hours until the deadline), this
// is the place to expend lots of effort.  Add the ability to edit
// the explicit map directly by hand, beginning with a vertex in the
// centre of every edge of the feature.


ExplicitGirihEditor::ExplicitGirihEditor(page_motif_maker * ed, QString aname) : ExplicitEditor (ed, aname)
{
    side = new SliderSet("ExplicitGirihEditor Star Sides", 10, 3, 24);
    skip = new DoubleSliderSet("ExplicitGirihEditor Skip D", 3.0, 1.0, 12.0, 100);

    skip->setPrecision(8);

    addLayout(side);
    addLayout(skip);

    connect(skip, &DoubleSliderSet::valueChanged, this, [this]() { updateFigure(true);});
    connect(side, &SliderSet::valueChanged,       this, [this]() { updateFigure(true);});
}

FigurePtr  ExplicitGirihEditor::getFigure()
{
    return girihFig;
}

void  ExplicitGirihEditor::resetWithFigure(FigurePtr fig, bool doEmit)
{
    if (!fig)
    {
        girihFig.reset();
        return;
    }

    girihFig = resetFigure(fig,FIG_TYPE_EXPLICIT_GIRIH);

    updateEditor();
    updateFigure(doEmit);
}

void ExplicitGirihEditor::updateEditor()
{
    if (!girihFig)
        return;

    int sideval   = girihFig->getN();
    qreal skipval = girihFig->skip;

    blockSignals(true);
    side->setValue(sideval);
    skip->setValue(skipval);
    blockSignals(false);

    FigureEditor::updateEditor();
}

void  ExplicitGirihEditor::updateFigure(bool doEmit)
{
    int side_val   = side->value();
    qreal skip_val = skip->value();

    girihFig->setN(side_val);
    girihFig->skip  = skip_val;

    FigureEditor::updateFigure(false);

    MotifMaker * motifMaker = MotifMaker::getInstance();
    MapPtr map = motifMaker->createExplicitGirihMap(side_val, skip_val);
    girihFig->setExplicitMap(map);

    if (doEmit)
        emit sig_figure_changed(girihFig);
}

////////////////////////////////////////////////////////////////////////////
//
// ExplicitHourglassEditor.java
//
// The editing controls for explicit figures.  A simple class, because
// (right now) explicit figures don't have any editing controls.  All
// you can do is ask the figure to be inferred from the rest of the
// Prototype.  So all we need is one button.
//
// If I have time (I've got about 36 hours until the deadline), this
// is the place to expend lots of effort.  Add the ability to edit
// the explicit map directly by hand, beginning with a vertex in the
// centre of every edge of the feature.

ExplicitHourglassEditor::ExplicitHourglassEditor(page_motif_maker *ed, QString aname) : ExplicitEditor(ed, aname)
{
    // Hourglass panel.
    d = new DoubleSliderSet("ExplicitHourglassEditor D", 2.0, 1.0, 12.0, 100);
    s = new SliderSet("ExplicitHourglassEditor SH", 1, 1, 12);

    addLayout(d);
    addLayout(s);

    connect(d, &DoubleSliderSet::valueChanged, this, [this]() { updateFigure(true);});
    connect(s, &SliderSet::valueChanged,       this, [this]() { updateFigure(true);});
}


FigurePtr ExplicitHourglassEditor::getFigure()
{
    return hourglassFig;
}

void ExplicitHourglassEditor::resetWithFigure(FigurePtr fig, bool doEmit)
{
    if (!fig)
    {
        hourglassFig.reset();
        return;
    }

    hourglassFig = resetFigure(fig,FIG_TYPE_EXPLICIT_HOURGLASS);

    updateEditor();
    updateFigure(doEmit);
}

void ExplicitHourglassEditor::updateEditor()
{
    FeaturePtr feature = menu->getActiveFeature();
    if (feature)
    {
        int n = feature->numPoints();

        qreal dmin = 1.0;
        qreal dmax = 0.5 * static_cast<qreal>(n);
        qreal dcur = d->value();
        qreal dd   = std::min(dmax - 0.5, std::max(dmin, dcur));

        qreal smin = 1.0;
        qreal smax = ceil( 0.5 * static_cast<qreal>(n));
        qreal scur = s->value();
        qreal ss   = floor(std::min( smax, std::max(smin, scur)));

        blockSignals(true);
        d->setValues(dd, dmin, dmax);
        s->setValues(static_cast<int>(ss), static_cast<int>(smin), n);
        blockSignals(false);
    }
    else
    {
        blockSignals(true);
        d->setValue(hourglassFig->d);
        s->setValue(hourglassFig->s);
        blockSignals(false);
    }

    FigureEditor::updateEditor();
}

void ExplicitHourglassEditor::updateFigure(bool doEmit)
{
    qreal dval = d->value();
    int sval   = s->value();

    hourglassFig->d = dval;
    hourglassFig->s = sval;

    FigureEditor::updateFigure(false);

    MotifMaker * motifMaker = MotifMaker::getInstance();
    MapPtr map = motifMaker->createExplicitHourglassMap(dval, sval);
    hourglassFig->setExplicitMap(map);

    if  (doEmit)
        emit sig_figure_changed(hourglassFig);
}

////////////////////////////////////////////////////////////////////////////
//
// ExplicitInferEditor.java
//
// The editing controls for explicit figures.  A simple class, because
// (right now) explicit figures don't have any editing controls.  All
// you can do is ask the figure to be inferred from the rest of the
// Prototype.  So all we need is one button.
//
// If I have time (I've got about 36 hours until the deadline), this
// is the place to expend lots of effort.  Add the ability to edit
// the explicit map directly by hand, beginning with a vertex in the
// centre of every edge of the feature.

ExplicitInferEditor::ExplicitInferEditor(page_motif_maker * ed, QString aname) : ExplicitEditor(ed, aname)
{
}

FigurePtr ExplicitInferEditor::getFigure()
{
    return explicitInferFig;
}

void ExplicitInferEditor::resetWithFigure(FigurePtr fig, bool doEmit)
{
    if (!fig)
    {
        explicitInferFig.reset();
        return;
    }

    qDebug() << "ExplicitInferEditor::resetWithFigure" << fig.get() << "  " << fig->getFigTypeString();

    explicitInferFig = resetFigure(fig,FIG_TYPE_EXPLICIT_INFER);

    updateEditor();
    updateFigure(doEmit);
}

void ExplicitInferEditor::updateFigure(bool doEmit)
{
    FigureEditor::updateFigure(false);

    MotifMaker * motifMaker = MotifMaker::getInstance();
    MapPtr map  = motifMaker->createExplicitInferredMap();
    explicitInferFig->setExplicitMap(map);

    if (doEmit)
        emit sig_figure_changed(explicitInferFig);
}

////////////////////////////////////////////////////////////////////////////
//
// ExplicitIntersectEditor.java
//
// The editing controls for explicit figures.  A simple class, because
// (right now) explicit figures don't have any editing controls.  All
// you can do is ask the figure to be inferred from the rest of the
// Prototype.  So all we need is one button.
//
// If I have time (I've got about 36 hours until the deadline), this
// is the place to expend lots of effort.  Add the ability to edit
// the explicit map directly by hand, beginning with a vertex in the
// centre of every edge of the feature.

ExplicitIntersectEditor::ExplicitIntersectEditor(page_motif_maker *ed, QString aname) : ExplicitEditor(ed, aname)
{
    side = new SliderSet("ExplicitIntersectEditor Star Sides", 10, 3, 24);
    skip = new DoubleSliderSet("ExplicitIntersectEditor D", 3, 0, 12, 100);
    s    = new SliderSet("ExplicitIntersectEditor S", 1, 1, 12);
    progressive_box = new QCheckBox("Progressive");

    addLayout(side);
    addLayout(skip);
    addLayout(s);
    addWidget(progressive_box);

    connect(skip, &DoubleSliderSet::valueChanged, this, [this]() { updateFigure(true);});
    connect(side, &SliderSet::valueChanged,       this, [this]() { updateFigure(true);});
    connect(s,    &SliderSet::valueChanged,       this, [this]() { updateFigure(true);});
    connect(progressive_box, &QCheckBox::stateChanged, this, [this]() { updateFigure(true);});
}

FigurePtr ExplicitIntersectEditor::getFigure()
{
    return intersect;
}

void ExplicitIntersectEditor::resetWithFigure(FigurePtr fi, bool doEmit)
{
    if (!fi)
    {
        intersect.reset();
        return;
    }

    intersect = resetFigure(fi, FIG_TYPE_EXPLICIT_INTERSECT);

    updateEditor();
    updateFigure(doEmit);
}

void ExplicitIntersectEditor::updateEditor()
{
    blockSignals(true);
    side->setValue(intersect->getN());
    skip->setValue(intersect->skip);
    s->setValue(intersect->s);
    progressive_box->setChecked(intersect->progressive);
    blockSignals(false);

    FigureEditor::updateEditor();
}

void ExplicitIntersectEditor::updateFigure(bool doEmit)
{
    int side_val        = side->value();
    qreal skip_val      = skip->value();
    int sval            = s->value();
    bool progressive    = progressive_box->isChecked();

    intersect->setN(side_val);
    intersect->skip  = skip_val;
    intersect->s     = sval;
    intersect->progressive = progressive;

    FigureEditor::updateFigure(false);

    MotifMaker * motifMaker = MotifMaker::getInstance();
    MapPtr map = motifMaker->createExplicitIntersectMap(side_val, skip_val, sval, progressive);
    intersect->setExplicitMap(map);

    if (doEmit)
        emit sig_figure_changed(intersect);
}

////////////////////////////////////////////////////////////////////////////
//
// ExplicitRosetteEditor.java
//
// The editing controls for explicit figures.  A simple class, because
// (right now) explicit figures don't have any editing controls.  All
// you can do is ask the figure to be inferred from the rest of the
// Prototype.  So all we need is one button.
//
// If I have time (I've got about 36 hours until the deadline), this
// is the place to expend lots of effort.  Add the ability to edit
// the explicit map directly by hand, beginning with a vertex in the
// centre of every edge of the feature.

// casper - interesting but somewhat inscrutable comment
// this implementation inherits the rosette editor

ExplicitRosetteEditor::ExplicitRosetteEditor(page_motif_maker * ed, QString aname) : ExplicitEditor(ed, aname)
{
    q_slider = new DoubleSliderSet("ExplicitRosetteEditor Q (Tip Angle)", 0.0, -3.0, 3.0, 100 );
    s_slider = new SliderSet("ExplicitRosetteEditor S (Sides Intersections)", 1, 1, 5);
    r_slider = new DoubleSliderSet("ExplicitRosetteEditor R (Flex Point)", 0.5, 0.0, 1.0, 100 );

    addLayout(q_slider);
    addLayout(s_slider);
    addLayout(r_slider);

    connect(q_slider, &DoubleSliderSet::valueChanged, this, [this]() { updateFigure(true);});
    connect(s_slider, &SliderSet::valueChanged,       this, [this]() { updateFigure(true);});
    connect(r_slider, &DoubleSliderSet::valueChanged, this, [this]() { updateFigure(true);});
}

FigurePtr ExplicitRosetteEditor::getFigure()
{
    return expRoseFig;
}

void ExplicitRosetteEditor::resetWithFigure(FigurePtr fig, bool doEmit)
{
    if (!fig)
    {
        expRoseFig.reset();
        return;
    }

    expRoseFig =resetFigure(fig,FIG_TYPE_EXPLICIT_ROSETTE);

    updateEditor();
    updateFigure(doEmit);
}

void ExplicitRosetteEditor::updateEditor()
{
    blockSignals(true);

    if (expRoseFig)
    {
        FeaturePtr feature = menu->getActiveFeature();
        if (feature)
        {
            int n = feature->numPoints();
            qreal smin = 1.0;
            qreal smax = ceil( 0.5 * static_cast<qreal>(n));
            qreal scur = s_slider->value();
            qreal ss   = floor(std::min(smax, std::max(smin, scur)));
            s_slider->setValues(static_cast<int>(ss), static_cast<int>(smin), static_cast<int>(smax));
        }
        else
        {
            int ss = expRoseFig->s;
            s_slider->setValues(ss, 1.0, 5);
        }

        double qq = expRoseFig->q;
        qreal  rp = expRoseFig->r_flexPt;

        blockSignals(true);
        q_slider->setValues(qq, -3.0, 3.0);       // DAC was -1.0, 1.0
        r_slider->setValues(rp,0.0,1.0);
        blockSignals(false);
    }
    else
    {
        r_slider->setValue(expRoseFig->r_flexPt);
        q_slider->setValue(expRoseFig->q);
        s_slider->setValue(expRoseFig->s);
    }

    blockSignals(false);

    FigureEditor::updateEditor();
}

void ExplicitRosetteEditor::updateFigure(bool doEmit)
{
    qreal qval = q_slider->value();
    int   sval = s_slider->value();
    qreal rval = r_slider->value();

    expRoseFig->q = qval;
    expRoseFig->s = sval;
    expRoseFig->r_flexPt = rval;

    FigureEditor::updateFigure(false);

    MotifMaker * motifMaker = MotifMaker::getInstance();
    MapPtr map = motifMaker->createExplicitRosetteMap(qval, sval, rval);
    expRoseFig->setExplicitMap(map);

    if (doEmit)
        emit sig_figure_changed(expRoseFig);
}

////////////////////////////////////////////////////////////////////////////
//
// ExplicitStarEditor.java
//
// The controls for editing a Star.  Glue code, just like RosetteEditor.
// caser pthis implementation inherits the rosette editor

ExplicitStarEditor::ExplicitStarEditor(page_motif_maker *ed, QString aname) : ExplicitEditor(ed, aname)
{
    d_slider = new DoubleSliderSet("ExplicitStarEditor D", 0.0, 0.0, 1.0, 100);
    s_slider = new SliderSet("ExplicitStarEditor S", 0.0, 0.0, 1.0);

    addLayout(d_slider);
    addLayout(s_slider);

    connect(d_slider, &DoubleSliderSet::valueChanged, this, [this]() { updateFigure(true);});
    connect(s_slider, &SliderSet::valueChanged,       this, [this]() { updateFigure(true);});
}

FigurePtr ExplicitStarEditor::getFigure()
{
    return expStarFig;
}

void ExplicitStarEditor::resetWithFigure(FigurePtr fig, bool doEmit)
{
    if (!fig)
    {
        expStarFig.reset();
        return;
    }

    expStarFig = resetFigure(fig,FIG_TYPE_EXPLICIT_STAR);

    updateEditor();
    updateFigure(doEmit);
}

void ExplicitStarEditor::updateEditor()
{
    FeaturePtr feature = menu->getActiveFeature();
    if (feature)
    {
        int n = feature->numPoints();

        qreal dmin = 1.0;
        qreal dmax = 0.5 * static_cast<qreal>(n);
        qreal dcur = d_slider->value();
        qreal dd   = std::min(dmax - 0.5, std::max(dmin, dcur));

        qreal smin = 1.0;
        qreal smax = ceil( 0.5 * static_cast<qreal>(n));
        qreal scur = s_slider->value();
        qreal ss   = floor(std::min(smax, std::max(smin, scur)));

        blockSignals(true);
        d_slider->setValues(dd, dmin, dmax);
        s_slider->setValues(static_cast<int>(ss), static_cast<int>(smin), static_cast<int>(smax));
        blockSignals(false);
    }
    else
    {
        blockSignals(true);
        d_slider->setValue(expStarFig->d);
        s_slider->setValue(expStarFig->s);
        blockSignals(false);
    }

    FigureEditor::updateEditor();
}

void ExplicitStarEditor::updateFigure(bool doEmit)
{
    qreal dval = d_slider->value();
    int sval   = s_slider->value();

    expStarFig->d = dval;
    expStarFig->s = sval;

    FigureEditor::updateFigure(false);

    MotifMaker * motifMaker = MotifMaker::getInstance();
    MapPtr map = motifMaker->createExplicitStarMap(dval, sval);
    expStarFig->setExplicitMap(map);

    if (doEmit)
        emit sig_figure_changed(expStarFig);
}

//  Make a figure from a feature

ExplicitFeatureEditor::ExplicitFeatureEditor(page_motif_maker * ed, QString aname) : ExplicitEditor(ed, aname)
{
}

FigurePtr ExplicitFeatureEditor::getFigure()
{
    return featFig;
}

void ExplicitFeatureEditor::resetWithFigure(FigurePtr fig, bool doEmit)
{
    if (!fig)
    {
        featFig.reset();
        return;
    }

    featFig = resetFigure(fig,FIG_TYPE_EXPLICIT_FEATURE);

    updateEditor();
    updateFigure(doEmit);
}

void ExplicitFeatureEditor::updateFigure(bool doEmit)
{
    FigureEditor::updateFigure(false);

    MotifMaker * motifMaker = MotifMaker::getInstance();
    MapPtr map = motifMaker->createExplicitFeatureMap();
    featFig->setExplicitMap(map);

    if  (doEmit)
    {
        emit sig_figure_changed(featFig);
    }
}
