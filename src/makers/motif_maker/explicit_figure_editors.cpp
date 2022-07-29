#include <QDebug>
#include <QCheckBox>

#include "makers/motif_maker/explicit_figure_editors.h"
#include "figures/explicit_figure.h"
#include "figures/radial_figure.h"
#include "makers/motif_maker/motif_maker.h"
#include "mosaic/design_element.h"
#include "panels/page_motif_maker.h"
#include "tile/feature.h"
#include "widgets/layout_sliderset.h"

typedef std::shared_ptr<RadialFigure>    RadialPtr;

ExplicitEditor::ExplicitEditor(page_motif_maker *ed, QString aname) : FigureEditor(ed, aname)
{
}

void ExplicitEditor::setFigure(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getFigure())
    {
        explicitFig.reset();
        return;
    }

    explicitFig = resetFigure(del,FIG_TYPE_EXPLICIT);

    figureToEditor();
    editorToFigure(doEmit);
}

ExplicitPtr ExplicitEditor::resetFigure(DesignElementPtr del, eFigType figType)
{
    FigurePtr fig = del->getFigure();
    qDebug() << "ExplicitEditor::setFigure" << fig.get() << "  " << fig->getFigTypeString();

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
        del->setFigure(ep);
    }

    Q_ASSERT(ep);
    FigureEditor::setFigure(ep,false);

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

    connect(skip, &DoubleSliderSet::valueChanged, this, [this]() { editorToFigure(true);});
    connect(side, &SliderSet::valueChanged,       this, [this]() { editorToFigure(true);});
}

void  ExplicitGirihEditor::setFigure(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getFigure())
    {
        girihFig.reset();
        return;
    }

    girihFig = resetFigure(del,FIG_TYPE_EXPLICIT_GIRIH);

    figureToEditor();
    editorToFigure(doEmit);
}

void ExplicitGirihEditor::figureToEditor()
{
    auto gfig = girihFig.lock();
    if (!gfig)
        return;

    int sideval   = gfig->getN();
    qreal skipval = gfig->skip;

    blockSignals(true);
    side->setValue(sideval);
    skip->setValue(skipval);
    blockSignals(false);

    FigureEditor::figureToEditor();
}

void  ExplicitGirihEditor::editorToFigure(bool doEmit)
{
    auto gfig = girihFig.lock();
    if (gfig)
    {
        int side_val   = side->value();
        qreal skip_val = skip->value();

        gfig->setN(side_val);
        gfig->skip  = skip_val;

        FigureEditor::editorToFigure(false);

        MotifMaker * motifMaker = MotifMaker::getInstance();
        MapPtr map = motifMaker->createExplicitGirihMap(side_val, skip_val);
        gfig->setExplicitMap(map);

        if (doEmit)
            emit sig_figure_modified(gfig);
    }
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

    connect(d, &DoubleSliderSet::valueChanged, this, [this]() { editorToFigure(true);});
    connect(s, &SliderSet::valueChanged,       this, [this]() { editorToFigure(true);});
}

void ExplicitHourglassEditor::setFigure(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getFigure())
    {
        hourglassFig.reset();
        return;
    }

    hourglassFig = resetFigure(del,FIG_TYPE_EXPLICIT_HOURGLASS);

    figureToEditor();
    editorToFigure(doEmit);
}

void ExplicitHourglassEditor::figureToEditor()
{
    auto hfig = hourglassFig.lock();
    if (hfig)
    {
        FeaturePtr feature = menu->getActiveFeature();
        if (feature)
        {
            int n = feature->numPoints();

            qreal dmin = 1.0;
            qreal dmax = 0.5 * static_cast<qreal>(n);
            qreal dcur = hfig->d;
            qreal dd   = std::min(dmax - 0.5, std::max(dmin, dcur));

            qreal smin = 1.0;
            qreal smax = ceil( 0.5 * static_cast<qreal>(n));
            qreal scur = hfig->s;
            qreal ss   = floor(std::min( smax, std::max(smin, scur)));

            blockSignals(true);
            d->setValues(dd, dmin, dmax);
            s->setValues(static_cast<int>(ss), static_cast<int>(smin), n);
            blockSignals(false);
        }
        else
        {
            blockSignals(true);
            d->setValue(hfig->d);
            s->setValue(hfig->s);
            blockSignals(false);
        }
    }
    FigureEditor::figureToEditor();
}

void ExplicitHourglassEditor::editorToFigure(bool doEmit)
{
    auto hfig = hourglassFig.lock();
    if (hfig)
    {
        qreal dval = d->value();
        int sval   = s->value();

        hfig->d = dval;
        hfig->s = sval;

        FigureEditor::editorToFigure(false);

        MotifMaker * motifMaker = MotifMaker::getInstance();
        MapPtr map = motifMaker->createExplicitHourglassMap(dval, sval);
        hfig->setExplicitMap(map);

        if  (doEmit)
            emit sig_figure_modified(hfig);
    }
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

void ExplicitInferEditor::setFigure(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getFigure())
    {
        explicitInferFig.reset();
        return;
    }

    explicitInferFig = resetFigure(del,FIG_TYPE_EXPLICIT_INFER);

    figureToEditor();
    editorToFigure(doEmit);
}

void ExplicitInferEditor::editorToFigure(bool doEmit)
{
    auto eifig = explicitInferFig.lock();
    if (eifig)
    {
        FigureEditor::editorToFigure(false);

        MotifMaker * motifMaker = MotifMaker::getInstance();
        MapPtr map  = motifMaker->createExplicitInferredMap();
        eifig->setExplicitMap(map);

        if (doEmit)
            emit sig_figure_modified(eifig);
    }
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

    connect(skip, &DoubleSliderSet::valueChanged, this, [this]() { editorToFigure(true);});
    connect(side, &SliderSet::valueChanged,       this, [this]() { editorToFigure(true);});
    connect(s,    &SliderSet::valueChanged,       this, [this]() { editorToFigure(true);});
    connect(progressive_box, &QCheckBox::stateChanged, this, [this]() { editorToFigure(true);});
}

void ExplicitIntersectEditor::setFigure(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getFigure())
    {
        intersect.reset();
        return;
    }

    intersect = resetFigure(del, FIG_TYPE_EXPLICIT_INTERSECT);

    figureToEditor();
    editorToFigure(doEmit);
}

void ExplicitIntersectEditor::figureToEditor()
{
    auto isect = intersect.lock();
    if (isect)
    {
        blockSignals(true);
        side->setValue(isect->getN());
        skip->setValue(isect->skip);
        s->setValue(isect->s);
        progressive_box->setChecked(isect->progressive);
        blockSignals(false);

        FigureEditor::figureToEditor();
    }
}

void ExplicitIntersectEditor::editorToFigure(bool doEmit)
{
    auto isect = intersect.lock();
    if (isect)
    {
        int side_val        = side->value();
        qreal skip_val      = skip->value();
        int sval            = s->value();
        bool progressive    = progressive_box->isChecked();

        isect->setN(side_val);
        isect->skip  = skip_val;
        isect->s     = sval;
        isect->progressive = progressive;

        FigureEditor::editorToFigure(false);

        MotifMaker * motifMaker = MotifMaker::getInstance();
        MapPtr map = motifMaker->createExplicitIntersectMap(side_val, skip_val, sval, progressive);
        isect->setExplicitMap(map);

        if (doEmit)
            emit sig_figure_modified(isect);
    }
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

    connect(q_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToFigure(true);});
    connect(s_slider, &SliderSet::valueChanged,       this, [this]() { editorToFigure(true);});
    connect(r_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToFigure(true);});
}

void ExplicitRosetteEditor::setFigure(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getFigure())
    {
        expRoseFig.reset();
        return;
    }

    expRoseFig =resetFigure(del,FIG_TYPE_EXPLICIT_ROSETTE);

    figureToEditor();
    editorToFigure(doEmit);
}

void ExplicitRosetteEditor::figureToEditor()
{
    auto erfig = expRoseFig.lock();
    if (erfig)
    {
        blockSignals(true);

        FeaturePtr feature = menu->getActiveFeature();
        if (feature)
        {
            int n = feature->numPoints();
            qreal smin = 1.0;
            qreal smax = ceil( 0.5 * static_cast<qreal>(n));
            qreal scur = erfig->s;
            qreal ss   = floor(std::min(smax, std::max(smin, scur)));
            s_slider->setValues(static_cast<int>(ss), static_cast<int>(smin), static_cast<int>(smax));
        }
        else
        {
            int ss = erfig->s;
            s_slider->setValues(ss, 1.0, 5);
        }

        q_slider->setValues(erfig->q, -3.0, 3.0);       // DAC was -1.0, 1.0
        r_slider->setValues(erfig->r_flexPt,0.0,1.0);

        blockSignals(false);

        FigureEditor::figureToEditor();
    }
}

void ExplicitRosetteEditor::editorToFigure(bool doEmit)
{
    auto erfig = expRoseFig.lock();
    if (erfig)
    {
        qreal qval = q_slider->value();
        int   sval = s_slider->value();
        qreal rval = r_slider->value();

        erfig->q = qval;
        erfig->s = sval;
        erfig->r_flexPt = rval;

        FigureEditor::editorToFigure(false);

        MotifMaker * motifMaker = MotifMaker::getInstance();
        MapPtr map = motifMaker->createExplicitRosetteMap(qval, sval, rval);
        erfig->setExplicitMap(map);

        if (doEmit)
            emit sig_figure_modified(erfig);
    }
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

    connect(d_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToFigure(true);});
    connect(s_slider, &SliderSet::valueChanged,       this, [this]() { editorToFigure(true);});
}

void ExplicitStarEditor::setFigure(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getFigure())
    {
        expStarFig.reset();
        return;
    }

    expStarFig = resetFigure(del,FIG_TYPE_EXPLICIT_STAR);

    figureToEditor();
    editorToFigure(doEmit);
}

void ExplicitStarEditor::figureToEditor()
{
    auto esfig = expStarFig.lock();
    if (esfig)
    {
        FeaturePtr feature = menu->getActiveFeature();
        if (feature)
        {
            int n = feature->numPoints();

            qreal dmin = 1.0;
            qreal dmax = 0.5 * static_cast<qreal>(n);
            qreal dcur = esfig->d;
            qreal dd   = std::min(dmax - 0.5, std::max(dmin, dcur));

            qreal smin = 1.0;
            qreal smax = ceil( 0.5 * static_cast<qreal>(n));
            qreal scur = esfig->s;
            qreal ss   = floor(std::min(smax, std::max(smin, scur)));

            blockSignals(true);
            d_slider->setValues(dd, dmin, dmax);
            s_slider->setValues(static_cast<int>(ss), static_cast<int>(smin), static_cast<int>(smax));
            blockSignals(false);
        }
        else
        {
            blockSignals(true);
            d_slider->setValue(esfig->d);
            s_slider->setValue(esfig->s);
            blockSignals(false);
        }

        FigureEditor::figureToEditor();
    }
}

void ExplicitStarEditor::editorToFigure(bool doEmit)
{
    auto esfig = expStarFig.lock();
    if (esfig)
    {
        qreal dval = d_slider->value();
        int sval   = s_slider->value();

        esfig->d = dval;
        esfig->s = sval;

        FigureEditor::editorToFigure(false);

        MotifMaker * motifMaker = MotifMaker::getInstance();
        MapPtr map = motifMaker->createExplicitStarMap(dval, sval);
        esfig->setExplicitMap(map);

        if (doEmit)
            emit sig_figure_modified(esfig);
    }
}

//  Make a figure from a feature
ExplicitFeatureEditor::ExplicitFeatureEditor(page_motif_maker * ed, QString aname) : ExplicitEditor(ed, aname)
{
}

void ExplicitFeatureEditor::setFigure(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getFigure())
    {
        featFig.reset();
        return;
    }

    featFig = resetFigure(del,FIG_TYPE_EXPLICIT_FEATURE);

    figureToEditor();
    editorToFigure(doEmit);
}

void ExplicitFeatureEditor::editorToFigure(bool doEmit)
{
    auto ffig = featFig.lock();
    if (ffig)
    {
        FigureEditor::editorToFigure(false);

        MotifMaker * motifMaker = MotifMaker::getInstance();
        MapPtr map = motifMaker->createExplicitFeatureMap();
        ffig->setExplicitMap(map);

        if  (doEmit)
        {
            emit sig_figure_modified(ffig);
        }
    }
}
