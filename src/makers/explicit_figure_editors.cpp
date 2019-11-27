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

#include "figure_maker.h"
#include "base/tiledpatternmaker.h"
#include "tapp/Star.h"
#include "tapp/ExplicitFigure.h"
#include "FeatureButton.h"

ExplicitEditor::ExplicitEditor(FigureMaker * ed, QString aname) : FigureEditor(ed, aname)
{
}

FigurePtr ExplicitEditor::getFigure()
{
    return explicitFig;
}

void ExplicitEditor::resetWithFigure(FigurePtr fig)
{
    if (!fig)
    {
        explicitFig.reset();
        return;
    }

    qDebug() << "ExplicitEditor::resetWithFigure" << fig.get() << "  " << fig->getFigTypeString();

    explicitFig = std::dynamic_pointer_cast<ExplicitFigure>(fig);
    if (!explicitFig)
    {
        MapPtr map = fig->getFigureMap();
        explicitFig = make_shared<ExplicitFigure>(*fig.get(),map,FIG_TYPE_EXPLICIT);
    }
    else
    {
        explicitFig->setFigType(FIG_TYPE_EXPLICIT);
    }

    Q_ASSERT(explicitFig);
    FigureEditor::resetWithFigure(explicitFig);

    updateLimits();
    updateGeometry();
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


ExplicitGirihEditor::ExplicitGirihEditor(FigureMaker * ed, QString aname) : FigureEditor (ed, aname)
{
    side = new SliderSet("ExplicitGirihEditor Star Sides", 10, 3, 24);
    skip = new DoubleSliderSet("ExplicitGirihEditor Skip D", 3.0, 1.0, 12.0, 100);

    addLayout(side);
    addLayout(skip);

    connect(skip, &DoubleSliderSet::valueChanged, this, &ExplicitGirihEditor::updateGeometry);
    connect(side, &SliderSet::valueChanged,       this, &ExplicitGirihEditor::updateGeometry);
}

FigurePtr  ExplicitGirihEditor::getFigure()
{
    return girihFig;
}

void  ExplicitGirihEditor::resetWithFigure(FigurePtr fig)
{
    if (!fig)
    {
        girihFig.reset();
        return;
    }

    girihFig = std::dynamic_pointer_cast<ExplicitFigure>(fig);
    if (!girihFig)
    {
        girihFig = make_shared<ExplicitFigure>(*fig.get(),FIG_TYPE_GIRIH);
    }
    else
    {
        girihFig->setFigType(FIG_TYPE_GIRIH);
    }

    Q_ASSERT(girihFig);
    FigureEditor::resetWithFigure(girihFig);

    updateLimits();
    updateGeometry();
}

void ExplicitGirihEditor::updateLimits()
{
    if (!girihFig)
        return;

    int sideval   = girihFig->sides;
    qreal skipval = girihFig->skip;

    blockSignals(true);
    side->setValue(sideval);
    skip->setValue(skipval);
    blockSignals(false);

    FigureEditor::updateLimits();
}

void  ExplicitGirihEditor::updateGeometry()
{
    int side_val   = side->value();
    qreal skip_val = skip->value();

    girihFig->sides = side_val;
    girihFig->skip  = skip_val;

    FigureEditor::updateGeometry();

    MapPtr map = figmaker->createExplicitGirihMap(side_val, skip_val);
    girihFig->setExplicitMap(map);

    emit sig_figure_changed();
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

ExplicitHourglassEditor::ExplicitHourglassEditor(FigureMaker * ed, QString aname) : FigureEditor(ed, aname)
{
    // Hourglass panel.
    d = new DoubleSliderSet("ExplicitHourglassEditor D", 2.0, 1.0, 12.0, 100);
    s = new SliderSet("ExplicitHourglassEditor SH", 1, 1, 12);

    addLayout(d);
    addLayout(s);

    connect(d, &DoubleSliderSet::valueChanged, this, &ExplicitHourglassEditor::updateGeometry);
    connect(s, &SliderSet::valueChanged,       this, &ExplicitHourglassEditor::updateGeometry);
}


FigurePtr ExplicitHourglassEditor::getFigure()
{
    return hourglassFig;
}

void ExplicitHourglassEditor::resetWithFigure(FigurePtr fig)
{
    if (!fig)
    {
        hourglassFig.reset();
        return;
    }

    hourglassFig = std::dynamic_pointer_cast<ExplicitFigure>(fig);
    if (!hourglassFig)
    {
        hourglassFig = make_shared<ExplicitFigure>(*figure.get(),FIG_TYPE_HOURGLASS);
    }
    else
    {
        hourglassFig->setFigType(FIG_TYPE_HOURGLASS);
    }

    FigureEditor::resetWithFigure(hourglassFig);

    updateLimits();
    updateGeometry();
}

void ExplicitHourglassEditor::updateLimits()
{
    FeaturePtr feature = figmaker->getActiveFeature();
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

    FigureEditor::updateLimits();
}

void ExplicitHourglassEditor::updateGeometry()
{
    qreal dval = d->value();
    int sval   = s->value();

    hourglassFig->d = dval;
    hourglassFig->s = sval;

    FigureEditor::updateGeometry();

    MapPtr map = figmaker->createExplicitHourglassMap(dval, sval);
    hourglassFig->setExplicitMap(map);

    emit sig_figure_changed();
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

ExplicitInferEditor::ExplicitInferEditor(FigureMaker * ed, QString aname) : FigureEditor(ed, aname)
{
}

FigurePtr ExplicitInferEditor::getFigure()
{
    return explicitFig;
}

void ExplicitInferEditor::resetWithFigure(FigurePtr fig)
{
    if (!fig)
    {
        explicitFig.reset();
        return;
    }

    qDebug() << "ExplicitInferEditor::resetWithFigure" << fig.get() << "  " << fig->getFigTypeString();

    explicitFig = std::dynamic_pointer_cast<ExplicitFigure>(fig);
    if (!explicitFig)
    {
        explicitFig = make_shared<ExplicitFigure>(*fig.get(),FIG_TYPE_INFER);
    }

    else
    {
        explicitFig->setFigType(FIG_TYPE_INFER);
    }

    FigureEditor::resetWithFigure(explicitFig);

    updateLimits();
    updateGeometry();
}

void ExplicitInferEditor::updateGeometry()
{
    FigureEditor::updateLimits();

    MapPtr map  = figmaker->createExplicitInferredMap();
    explicitFig->setExplicitMap(map);

    emit sig_figure_changed();
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

ExplicitIntersectEditor::ExplicitIntersectEditor(FigureMaker * ed, QString aname) : FigureEditor(ed, aname)
{
    side = new SliderSet("ExplicitIntersectEditor Star Sides", 10, 3, 24);
    skip = new DoubleSliderSet("ExplicitIntersectEditor D", 3, 0, 12, 100);
    s    = new SliderSet("ExplicitIntersectEditor S", 1, 1, 12);
    progressive_box = new QCheckBox("Progressive");

    addLayout(side);
    addLayout(skip);
    addLayout(s);
    addWidget(progressive_box);

    connect(skip, &DoubleSliderSet::valueChanged, this, &ExplicitIntersectEditor::updateGeometry);
    connect(side, &SliderSet::valueChanged,       this, &ExplicitIntersectEditor::updateGeometry);
    connect(s,    &SliderSet::valueChanged,       this, &ExplicitIntersectEditor::updateGeometry);
    connect(progressive_box, &QCheckBox::stateChanged, this, &ExplicitIntersectEditor::updateGeometry);
}

FigurePtr ExplicitIntersectEditor::getFigure()
{
    return intersect;
}

void ExplicitIntersectEditor::resetWithFigure(FigurePtr fig)
{
    if (!fig)
    {
        intersect.reset();
        return;
    }

    intersect = std::dynamic_pointer_cast<ExplicitFigure>(fig);
    if (!intersect)
    {
        intersect = make_shared<ExplicitFigure>(*fig.get(),FIG_TYPE_INTERSECT);
    }
    else
    {
        intersect->setFigType(FIG_TYPE_INTERSECT);
    }

    FigureEditor::resetWithFigure(intersect);

    updateLimits();
    updateGeometry();
}

void ExplicitIntersectEditor::updateLimits()
{
    blockSignals(true);
    side->setValue(intersect->sides);
    skip->setValue(intersect->skip);
    s->setValue(intersect->s);
    progressive_box->setChecked(intersect->progressive);
    blockSignals(false);

    FigureEditor::updateLimits();
}

void ExplicitIntersectEditor::updateGeometry()
{
    int side_val        = side->value();
    qreal skip_val      = skip->value();
    int sval            = s->value();
    bool progressive    = progressive_box->isChecked();

    intersect->sides = side_val;
    intersect->skip  = skip_val;
    intersect->s     = sval;
    intersect->progressive = progressive;

    MapPtr map          = figmaker->createExplicitIntersectMap(side_val, skip_val, sval, progressive);
    intersect->setExplicitMap(map);

    emit sig_figure_changed();
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

ExplicitRosetteEditor::ExplicitRosetteEditor(FigureMaker * ed, QString aname) : FigureEditor(ed, aname)
{
    q_slider = new DoubleSliderSet("ExplicitRosetteEditor Q (Tip Angle)", 0.0, -3.0, 3.0, 100 );
    s_slider = new SliderSet("ExplicitRosetteEditor S (Sides Intersections)", 1, 1, 5);
    r_slider = new DoubleSliderSet("ExplicitRosetteEditor R (Flex Point)", 0.5, 0.0, 1.0, 100 );

    addLayout(q_slider);
    addLayout(s_slider);
    addLayout(r_slider);

    connect(q_slider, &DoubleSliderSet::valueChanged, this, &ExplicitRosetteEditor::updateGeometry);
    connect(s_slider, &SliderSet::valueChanged,       this, &ExplicitRosetteEditor::updateGeometry);
    connect(r_slider, &DoubleSliderSet::valueChanged, this, &ExplicitRosetteEditor::updateGeometry);
}

FigurePtr ExplicitRosetteEditor::getFigure()
{
    return expRoseFig;
}

void ExplicitRosetteEditor::resetWithFigure(FigurePtr fig)
{
    if (!fig)
    {
        expRoseFig.reset();
        return;
    }

    expRoseFig = std::dynamic_pointer_cast<ExplicitFigure>(fig);
    if (!expRoseFig)
    {
        expRoseFig = make_shared<ExplicitFigure>(*fig.get(),FIG_TYPE_EXPLICIT_ROSETTE);
    }
    else
    {
        expRoseFig->setFigType(FIG_TYPE_EXPLICIT_ROSETTE);
    }

    FigureEditor::resetWithFigure(expRoseFig);

    updateLimits();
    updateGeometry();
}

void ExplicitRosetteEditor::updateLimits()
{
    blockSignals(true);

    if (expRoseFig)
    {
        FeaturePtr feature = figmaker->getActiveFeature();
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

    FigureEditor::updateLimits();
}

void ExplicitRosetteEditor::updateGeometry()
{
    qreal qval = q_slider->value();
    int   sval = s_slider->value();
    qreal rval = r_slider->value();

    expRoseFig->q = qval;
    expRoseFig->s = sval;
    expRoseFig->r_flexPt = rval;

    FigureEditor::updateGeometry();

    MapPtr map = figmaker->createExplicitRosetteMap(qval, sval, rval);
    expRoseFig->setExplicitMap(map);

    emit sig_figure_changed();
}

////////////////////////////////////////////////////////////////////////////
//
// ExplicitStarEditor.java
//
// The controls for editing a Star.  Glue code, just like RosetteEditor.
// caser pthis implementation inherits the rosette editor

ExplicitStarEditor::ExplicitStarEditor(FigureMaker * ed, QString aname) : FigureEditor(ed, aname)
{
    d_slider = new DoubleSliderSet("ExplicitStarEditor D", 0.0, 0.0, 1.0, 100);
    s_slider = new SliderSet("ExplicitStarEditor S", 0.0, 0.0, 1.0);

    addLayout(d_slider);
    addLayout(s_slider);

    connect(d_slider, &DoubleSliderSet::valueChanged, this, &ExplicitStarEditor::updateGeometry);
    connect(s_slider, &SliderSet::valueChanged,       this, &ExplicitStarEditor::updateGeometry);
}

FigurePtr ExplicitStarEditor::getFigure()
{
    return expStarFig;
}

void ExplicitStarEditor::resetWithFigure(FigurePtr fig)
{
    if (!fig)
    {
        expStarFig.reset();
        return;
    }

    expStarFig = std::dynamic_pointer_cast<ExplicitFigure>(fig);
    if (!expStarFig)
    {
        expStarFig  = make_shared<ExplicitFigure>(*fig.get(), FIG_TYPE_EXPLICIT_STAR);
    }
    else
    {
        expStarFig->setFigType(FIG_TYPE_EXPLICIT_STAR);
    }

    FigureEditor::resetWithFigure(expStarFig);

    updateLimits();
    updateGeometry();
}

void ExplicitStarEditor::updateLimits()
{
    FeaturePtr feature = figmaker->getActiveFeature();
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

    FigureEditor::updateLimits();
}

void ExplicitStarEditor::updateGeometry()
{
    qreal dval = d_slider->value();
    int sval   = s_slider->value();

    expStarFig->d = dval;
    expStarFig->s = sval;

    FigureEditor::updateGeometry();

    MapPtr map = figmaker->createExplicitStarMap(dval, sval);
    expStarFig->setExplicitMap(map);

    emit sig_figure_changed();
}

//  Make a figure from a feature

ExplicitFeatureEditor::ExplicitFeatureEditor(FigureMaker * ed, QString aname) : FigureEditor(ed, aname)
{
}

FigurePtr ExplicitFeatureEditor::getFigure()
{
    return featFig;
}

void ExplicitFeatureEditor::resetWithFigure(FigurePtr fig)
{
    if (!fig)
    {
        featFig.reset();
        return;
    }

    featFig = std::dynamic_pointer_cast<ExplicitFigure>(fig);
    if (!featFig)
    {
        MapPtr map = fig->getFigureMap();
        featFig = make_shared<ExplicitFigure>(map, FIG_TYPE_FEATURE);
    }
    else
    {
        featFig->setFigType(FIG_TYPE_FEATURE);
    }

    updateLimits();
    updateGeometry();
}

void ExplicitFeatureEditor::updateGeometry()
{
    FigureEditor::updateGeometry();

    MapPtr map = figmaker->createExplicitFeatureMap();
    featFig->setExplicitMap(map);
}
