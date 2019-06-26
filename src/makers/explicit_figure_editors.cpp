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

ExplicitEditor::ExplicitEditor(FigureMaker * ed, QString name) : FigureEditor(ed, name)
{
}

FigurePtr ExplicitEditor::getFigure()
{
    return explicitFig;
}

void ExplicitEditor::resetWithFigure(FigurePtr figure)
{
    if (!figure)
    {
        explicitFig.reset();
        return;
    }

    qDebug() << "ExplicitEditor::resetWithFigure" << figure.get() << "  " << figure->getFigTypeString();

    explicitFig = std::dynamic_pointer_cast<ExplicitFigure>(figure);
    if (!explicitFig)
    {
        MapPtr map = figure->getFigureMap();
        explicitFig = make_shared<ExplicitFigure>(*figure.get(),map,FIG_TYPE_EXPLICIT);
    }
    else
    {
        explicitFig->setFigType(FIG_TYPE_EXPLICIT);
    }

    updateLimits();
    updateGeometry();
}

void ExplicitEditor::updateLimits()
{}

void ExplicitEditor::updateGeometry()
{
    int  bSides             = boundarySides->value();
    qreal bScale            = boundaryScale->value();
    qreal figScale          = figureScale->value();

    explicitFig->setExtBoundarySides(bSides);
    explicitFig->setExtBoundaryScale(bScale);
    explicitFig->setFigureScale(figScale);

    explicitFig->buildBoundary();

    emit sig_figure_changed();
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


ExplicitGirihEditor::ExplicitGirihEditor(FigureMaker * ed, QString name) : FigureEditor (ed, name)
{
    side = new SliderSet("ExplicitGirihEditor Star Sides", 10, 3, 24);
    skip = new DoubleSliderSet("ExplicitGirihEditor Skip D", 3.0, 1.0, 12.0, 100);

    QPushButton * intersectBtn = new QPushButton("Make Girih");
    intersectBtn->setMaximumWidth(101);

    addLayout(side);
    addLayout(skip);
    vbox->addWidget(intersectBtn,0,Qt::AlignHCenter);

    connect(skip, &DoubleSliderSet::valueChanged, this, &ExplicitGirihEditor::updateGeometry);
    connect(side, &SliderSet::valueChanged,       this, &ExplicitGirihEditor::updateGeometry);
    connect(intersectBtn, &QPushButton::clicked,  this, &ExplicitGirihEditor::updateGeometry);
}

FigurePtr  ExplicitGirihEditor::getFigure()
{
    return girihFig;
}

void  ExplicitGirihEditor::resetWithFigure(FigurePtr figure)
{
    if (!figure)
    {
        girihFig.reset();
        return;
    }

    girihFig = std::dynamic_pointer_cast<ExplicitFigure>(figure);
    if (!girihFig)
    {
        girihFig = make_shared<ExplicitFigure>(*figure.get(),FIG_TYPE_GIRIH);
    }

    updateLimits();
    updateGeometry();
}

void ExplicitGirihEditor::updateLimits()
{}

void  ExplicitGirihEditor::updateGeometry()
{
    int side_val   = side->value();
    qreal skip_val = skip->value();

    MapPtr map = editor->createExplicitGirihMap(side_val, skip_val);
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

ExplicitHourglassEditor::ExplicitHourglassEditor(FigureMaker * ed, QString name) : FigureEditor(ed, name)
{
    // Hourglass panel.
    d = new DoubleSliderSet("ExplicitHourglassEditor D", 2.0, 1.0, 12.0, 100);
    s = new SliderSet("ExplicitHourglassEditor SH", 1, 1, 12);


    QPushButton * hourglassBtn = new QPushButton("Make Hourglass");
    hourglassBtn->setMaximumWidth(101);

    addLayout(d);
    addLayout(s);
    vbox->addWidget(hourglassBtn,0,Qt::AlignHCenter);

    connect(d, &DoubleSliderSet::valueChanged, this, &ExplicitHourglassEditor::updateGeometry);
    connect(s, &SliderSet::valueChanged,       this, &ExplicitHourglassEditor::updateGeometry);
    connect(hourglassBtn,&QPushButton::clicked,this, &ExplicitHourglassEditor::updateGeometry);
}


FigurePtr ExplicitHourglassEditor::getFigure()
{
    return hourglassFig;
}

void ExplicitHourglassEditor::resetWithFigure(FigurePtr figure)
{
    if (!figure)
    {
        hourglassFig.reset();
        return;
    }

    hourglassFig = std::dynamic_pointer_cast<ExplicitFigure>(figure);
    if (!hourglassFig)
    {
        hourglassFig = make_shared<ExplicitFigure>(*figure.get(),FIG_TYPE_HOURGLASS);
    }

    updateLimits();
    updateGeometry();
}

void ExplicitHourglassEditor::updateLimits()
{
    FeaturePtr feature = editor->getActiveFeature();
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

        d->setValues(dd, dmin, dmax);
        s->setValues(static_cast<int>(ss), static_cast<int>(smin), n);
    }
}

void ExplicitHourglassEditor::updateGeometry()
{
    qreal dval = d->value();
    int sval   = s->value();

    MapPtr map = editor->createExplicitHourglassMap(dval, sval);
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

ExplicitInferEditor::ExplicitInferEditor(FigureMaker * ed, QString name) : FigureEditor(ed, name)
{
    QPushButton * inferBtn = new QPushButton("Make Infer");
    inferBtn->setMaximumWidth(101);

    vbox->addWidget(inferBtn,0,Qt::AlignHCenter);

    connect(inferBtn, &QPushButton::clicked, this, &ExplicitInferEditor::updateGeometry);
}

FigurePtr ExplicitInferEditor::getFigure()
{
    return explicitFig;
}

void ExplicitInferEditor::resetWithFigure(FigurePtr figure)
{
    if (!figure)
    {
        explicitFig.reset();
        return;
    }

    qDebug() << "ExplicitInferEditor::resetWithFigure" << figure.get() << "  " << figure->getFigTypeString();

    explicitFig = std::dynamic_pointer_cast<ExplicitFigure>(figure);
    if (!explicitFig)
    {
        explicitFig = make_shared<ExplicitFigure>(*figure.get(),FIG_TYPE_INFER);
    }

    updateLimits();
    updateGeometry();
}

void ExplicitInferEditor::updateLimits()
{
}

void ExplicitInferEditor::updateGeometry()
{
    MapPtr map  = editor->createExplicitInferredMap();
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

ExplicitIntersectEditor::ExplicitIntersectEditor(FigureMaker * ed, QString name) : FigureEditor(ed, name)
{
    side = new SliderSet("ExplicitIntersectEditor Star Sides", 10, 3, 24);
    skip = new DoubleSliderSet("ExplicitIntersectEditor D", 3, 0, 12, 100);
    s    = new SliderSet("ExplicitIntersectEditor S", 1, 1, 12);
    progressive_box = new QCheckBox("Progressive");
    QPushButton * intersectBtn = new QPushButton("Make Intersect");
    intersectBtn->setMaximumWidth(101);

    addLayout(side);
    addLayout(skip);
    addLayout(s);
    addWidget(progressive_box);
    vbox->addWidget(intersectBtn,0,Qt::AlignHCenter);

    connect(skip, &DoubleSliderSet::valueChanged, this, &ExplicitIntersectEditor::updateGeometry);
    connect(side, &SliderSet::valueChanged,       this, &ExplicitIntersectEditor::updateGeometry);
    connect(s,    &SliderSet::valueChanged,       this, &ExplicitIntersectEditor::updateGeometry);
    connect(progressive_box, &QCheckBox::stateChanged, this, &ExplicitIntersectEditor::updateGeometry);
    connect(intersectBtn,    &QPushButton::clicked,    this, &ExplicitIntersectEditor::updateGeometry);
}

FigurePtr ExplicitIntersectEditor::getFigure()
{
    return intersectFig;
}

void ExplicitIntersectEditor::resetWithFigure(FigurePtr figure)
{
    if (!figure)
    {
        intersectFig.reset();
        return;
    }

    intersectFig = std::dynamic_pointer_cast<ExplicitFigure>(figure);
    if (!intersectFig)
    {
        bool progressive = progressive_box->isChecked();
        eFigType fill    = (progressive) ? FIG_TYPE_INTERSECT_PROGRESSIVE :  FIG_TYPE_INTERSECT;
        intersectFig     = make_shared<ExplicitFigure>(*figure.get(),fill);
    }

    updateLimits();
    updateGeometry();
}

void ExplicitIntersectEditor::updateLimits()
{}

void ExplicitIntersectEditor::updateGeometry()
{
    int side_val        = side->value();
    qreal skip_val      = skip->value();
    int sval            =  s->value();
    bool progressive    = progressive_box->isChecked();
    eFigType fill       = (progressive) ? FIG_TYPE_INTERSECT_PROGRESSIVE :  FIG_TYPE_INTERSECT;
    MapPtr map          = editor->createExplicitIntersectMap(side_val, skip_val, sval, progressive);

    intersectFig->setFigType(fill);
    intersectFig->setExplicitMap(map);
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

ExplicitRosetteEditor::ExplicitRosetteEditor(FigureMaker * ed, QString name) : RosetteEditor(ed, name)
{
    r = new DoubleSliderSet("RosetteEditor r", 0.5, 0.0, 1.0, 100 );

    QPushButton * rosetteBtn = new QPushButton("Make Rosette");
    rosetteBtn->setMaximumWidth(101);

    addLayout(r);
    vbox->addWidget(rosetteBtn,0,Qt::AlignHCenter);

    connect(r, &DoubleSliderSet::valueChanged, this, &ExplicitRosetteEditor::updateGeometry);
    connect(rosetteBtn, &QPushButton::clicked, this, &ExplicitRosetteEditor::updateGeometry);
}

FigurePtr ExplicitRosetteEditor::getFigure()
{
    return expRoseFig;
}

void ExplicitRosetteEditor::resetWithFigure(FigurePtr figure)
{
    if (!figure)
    {
        expRoseFig.reset();
        return;
    }

    expRoseFig = std::dynamic_pointer_cast<ExplicitFigure>(figure);
    if (!expRoseFig)
    {
        expRoseFig = make_shared<ExplicitFigure>(*figure.get(),FIG_TYPE_EXPLICIT_ROSETTE);
    }

    updateLimits();
    updateGeometry();
}

void ExplicitRosetteEditor::updateLimits()
{
    FeaturePtr feature = editor->getActiveFeature();
    if (feature)
    {
        int n = feature->numPoints();

        qreal smin = 1.0;
        qreal smax = ceil( 0.5 * static_cast<qreal>(n));
        qreal scur = s->value();
        qreal ss   = floor(std::min(smax, std::max(smin, scur)));

        s->setValues(static_cast<int>(ss), static_cast<int>(smin), static_cast<int>(smax));
    }
}

void ExplicitRosetteEditor::updateGeometry()
{
    qreal qval = q->value();
    int sval   = s->value();
    qreal rval = r->value();

    MapPtr map = editor->createExplicitRosetteMap(qval, sval, rval);
    expRoseFig->setExplicitMap(map);
    emit sig_figure_changed();
}

////////////////////////////////////////////////////////////////////////////
//
// ExplicitStarEditor.java
//
// The controls for editing a Star.  Glue code, just like RosetteEditor.
// caser pthis implementation inherits the rosette editor

ExplicitStarEditor::ExplicitStarEditor(FigureMaker * editor, QString name) : StarEditor(editor, name)
{
    QPushButton * starBtn = new QPushButton("Make Star");
    starBtn->setMaximumWidth(101);

    vbox->addWidget(starBtn,0,Qt::AlignHCenter);

    connect(starBtn, &QPushButton::clicked, this, &ExplicitStarEditor::updateGeometry);
}

FigurePtr ExplicitStarEditor::getFigure()
{
    return starFig;
}

void ExplicitStarEditor::resetWithFigure(FigurePtr figure)
{
    if (!figure)
    {
        starFig.reset();
        return;
    }

    starFig = std::dynamic_pointer_cast<ExplicitFigure>(figure);
    if (!starFig)
    {
        starFig    = make_shared<ExplicitFigure>(*figure.get(), FIG_TYPE_EXPLICIT_STAR);
    }

    updateLimits();
    updateGeometry();
}

void ExplicitStarEditor::updateLimits()
{
    FeaturePtr feature = editor->getActiveFeature();
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
        qreal ss   = floor(std::min(smax, std::max(smin, scur)));

        d->setValues(dd, dmin, dmax);
        s->setValues(static_cast<int>(ss), static_cast<int>(smin), static_cast<int>(smax));
    }
}

void ExplicitStarEditor::updateGeometry()
{
    qreal dval = d->value();
    int sval   = s->value();

    MapPtr map = editor->createExplicitStarMap(dval, sval);
    starFig->setExplicitMap(map);
    emit sig_figure_changed();
}

//  Make a figure from a feature

ExplicitFeatureEditor::ExplicitFeatureEditor(FigureMaker * ed, QString name) : FigureEditor(ed, name)
{
    QPushButton * fBtn = new QPushButton("Make Explicit Feature");
    fBtn->setMaximumWidth(101);

    vbox->addWidget(fBtn,0,Qt::AlignHCenter);

    connect(fBtn, &QPushButton::clicked, this, &ExplicitFeatureEditor::updateGeometry);
}

FigurePtr ExplicitFeatureEditor::getFigure()
{
    return featFig;
}

void ExplicitFeatureEditor::resetWithFigure(FigurePtr figure)
{
    if (!figure)
    {
        featFig.reset();
        return;
    }


    featFig = std::dynamic_pointer_cast<ExplicitFigure>(figure);
    if (!featFig)
    {
        featFig = make_shared<ExplicitFigure>(*figure.get(), FIG_TYPE_FEATURE);
    }

    updateLimits();
    updateGeometry();
}

void ExplicitFeatureEditor::updateLimits()
{}

void ExplicitFeatureEditor::updateGeometry()
{
    MapPtr map = editor->createExplicitFeatureMap();
    featFig->setExplicitMap(map);
}
