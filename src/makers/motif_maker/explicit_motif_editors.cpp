#include <QDebug>
#include <QCheckBox>

#include "makers/motif_maker/explicit_motif_editors.h"
#include "motifs/explicit_motif.h"
#include "motifs/radial_motif.h"
#include "makers/motif_maker/motif_maker.h"
#include "mosaic/design_element.h"
#include "panels/page_motif_maker.h"
#include "tile/tile.h"
#include "widgets/layout_sliderset.h"

typedef std::shared_ptr<RadialMotif>    RadialPtr;

ExplicitEditor::ExplicitEditor(page_motif_maker *ed, QString aname) : NamedMotifEditor(ed, aname)
{
    motifMaker = MotifMaker::getInstance();
}

void ExplicitEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getMotif())
    {
        explicitMotif.reset();
        return;
    }

    explicitMotif = resetMotif(del,MOTIF_TYPE_EXPLICIT);

    motifToEditor();
    editorToMotif(doEmit);
}

ExplicitPtr ExplicitEditor::resetMotif(DesignElementPtr del, eMotifType figType)
{
    MotifPtr fig = del->getMotif();
    qDebug() << "ExplicitEditor::resetMotif" << fig.get() << "  " << fig->getMotifTypeString();

    ExplicitPtr ep = std::dynamic_pointer_cast<ExplicitMotif>(fig);
    if (ep)
    {
        ep->setMotifType(figType);
    }
    else
    {
        int sides = 10;
        RadialPtr rp = std::dynamic_pointer_cast<RadialMotif>(fig);
        if (rp)
        {
            sides = rp->getN();
        }
        MapPtr map = fig->getMap();
        ep = std::make_shared<ExplicitMotif>(*fig.get(),map,figType, sides);
        del->setMotif(ep);
    }

    Q_ASSERT(ep);
    NamedMotifEditor::setMotif(ep,false);

    return ep;
}

////////////////////////////////////////////////////////////////////////////
//
// ExplicitGirihEditor.java
//
// The editing controls for explicit motifs.  A simple class, because
// (right now) explicit motifs don't have any editing controls.  All
// you can do is ask the motif to be inferred from the rest of the
// Prototype.  So all we need is one button.
//
// If I have time (I've got about 36 hours until the deadline), this
// is the place to expend lots of effort.  Add the ability to edit
// the explicit map directly by hand, beginning with a vertex in the
// centre of every edge of the tile.


ExplicitGirihEditor::ExplicitGirihEditor(page_motif_maker * ed, QString aname) : ExplicitEditor (ed, aname)
{
    side = new SliderSet("Explicit Girih Star Sides", 10, 3, 24);
    skip = new DoubleSliderSet("Explicit Girih Skip D", 3.0, 1.0, 12.0, 100);

    skip->setPrecision(8);

    addLayout(side);
    addLayout(skip);

    connect(skip, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(side, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
}

void  ExplicitGirihEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getMotif())
    {
        girih.reset();
        return;
    }

    girih = resetMotif(del,MOTIF_TYPE_EXPLICIT_GIRIH);

    motifToEditor();
    editorToMotif(doEmit);
}

void ExplicitGirihEditor::motifToEditor()
{
    auto gfig = girih.lock();
    if (!gfig)
        return;

    int sideval   = gfig->getN();
    qreal skipval = gfig->skip;

    blockSignals(true);
    side->setValue(sideval);
    skip->setValue(skipval);
    blockSignals(false);

    NamedMotifEditor::motifToEditor();
}

void  ExplicitGirihEditor::editorToMotif(bool doEmit)
{
    auto gfig = girih.lock();
    if (gfig)
    {
        gfig->dump();
        int star_sides  = side->value();
        qreal star_skip = skip->value();

        gfig->setN(star_sides);
        gfig->skip  = star_skip;

        NamedMotifEditor::editorToMotif(false);

        auto proto   = motifMaker->getSelectedPrototype();
        auto tile = motifMaker->getActiveTile();
        gfig->setupInfer(proto);
        gfig->newExplicitMap();
        gfig->inferGirih(tile,star_sides,star_skip);
        gfig->setMotifBoundary(tile->getPolygon());
        gfig->dump();

        if (doEmit)
            emit sig_motif_modified(gfig);
    }
}

////////////////////////////////////////////////////////////////////////////
//
// ExplicitHourglassEditor.java
//
// The editing controls for explicit motifs.  A simple class, because
// (right now) explicit motifs don't have any editing controls.  All
// you can do is ask the motif to be inferred from the rest of the
// Prototype.  So all we need is one button.
//
// If I have time (I've got about 36 hours until the deadline), this
// is the place to expend lots of effort.  Add the ability to edit
// the explicit map directly by hand, beginning with a vertex in the
// centre of every edge of the tile.

ExplicitHourglassEditor::ExplicitHourglassEditor(page_motif_maker *ed, QString aname) : ExplicitEditor(ed, aname)
{
    // Hourglass panel.
    d = new DoubleSliderSet("Explicit Hourglass D", 2.0, 1.0, 12.0, 100);
    s = new SliderSet("Explicit Hourglass SH", 1, 1, 12);

    addLayout(d);
    addLayout(s);

    connect(d, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
}

void ExplicitHourglassEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getMotif())
    {
        hourglass.reset();
        return;
    }

    hourglass = resetMotif(del,MOTIF_TYPE_EXPLICIT_HOURGLASS);

    motifToEditor();
    editorToMotif(doEmit);
}

void ExplicitHourglassEditor::motifToEditor()
{
    auto hfig = hourglass.lock();
    if (hfig)
    {
        TilePtr tile = menu->getActiveTile();
        if (tile)
        {
            int n = tile->numPoints();

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
    NamedMotifEditor::motifToEditor();
}

void ExplicitHourglassEditor::editorToMotif(bool doEmit)
{
    auto hfig = hourglass.lock();
    if (hfig)
    {
        hfig->dump();
        qreal dval = d->value();
        int sval   = s->value();

        hfig->d = dval;
        hfig->s = sval;

        NamedMotifEditor::editorToMotif(false);

        auto proto   = motifMaker->getSelectedPrototype();
        auto tile = motifMaker->getActiveTile();
        hfig->setupInfer(proto);
        hfig->newExplicitMap();
        hfig->inferHourglass(tile, dval, sval);
        hfig->setMotifBoundary(tile->getPolygon());
        hfig->dump();

        if  (doEmit)
            emit sig_motif_modified(hfig);
    }
}

////////////////////////////////////////////////////////////////////////////
//
// ExplicitInferEditor.java
//
// The editing controls for explicit motifs.  A simple class, because
// (right now) explicit motifs don't have any editing controls.  All
// you can do is ask the motif to be inferred from the rest of the
// Prototype.  So all we need is one button.
//
// If I have time (I've got about 36 hours until the deadline), this
// is the place to expend lots of effort.  Add the ability to edit
// the explicit map directly by hand, beginning with a vertex in the
// centre of every edge of the tile.

ExplicitInferEditor::ExplicitInferEditor(page_motif_maker * ed, QString aname) : ExplicitEditor(ed, aname)
{
}

void ExplicitInferEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getMotif())
    {
        explicitInfer.reset();
        return;
    }

    explicitInfer = resetMotif(del,MOTIF_TYPE_EXPLICIT_INFER);

    motifToEditor();
    editorToMotif(doEmit);
}

void ExplicitInferEditor::editorToMotif(bool doEmit)
{
    auto eifig = explicitInfer.lock();
    if (eifig)
    {
        eifig->dump();
        NamedMotifEditor::editorToMotif(false);

        auto proto = motifMaker->getSelectedPrototype();
        auto tile  = motifMaker->getActiveTile();
        eifig->setupInfer(proto);
        eifig->newExplicitMap();
        eifig->infer(tile);
        eifig->setMotifBoundary(motifMaker->getActiveTile()->getPolygon());
        eifig->dump();

        if (doEmit)
            emit sig_motif_modified(eifig);
    }
}

////////////////////////////////////////////////////////////////////////////
//
// ExplicitIntersectEditor.java
//
// The editing controls for explicit motifs.  A simple class, because
// (right now) explicit motifs don't have any editing controls.  All
// you can do is ask the motif to be inferred from the rest of the
// Prototype.  So all we need is one button.
//
// If I have time (I've got about 36 hours until the deadline), this
// is the place to expend lots of effort.  Add the ability to edit
// the explicit map directly by hand, beginning with a vertex in the
// centre of every edge of the tile.

ExplicitIntersectEditor::ExplicitIntersectEditor(page_motif_maker *ed, QString aname) : ExplicitEditor(ed, aname)
{
    side = new SliderSet("Explicit Intersect Star Sides", 10, 3, 24);
    skip = new DoubleSliderSet("Explicit Intersect Sides Skip", 3, 0, 12, 100);
    s    = new SliderSet("Explicit Intersect S", 1, 1, 12);
    progressive_box = new QCheckBox("Progressive");

    addLayout(side);
    addLayout(skip);
    addLayout(s);
    addWidget(progressive_box);

    connect(skip, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(side, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(s,    &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(progressive_box, &QCheckBox::stateChanged, this, [this]() { editorToMotif(true);});
}

void ExplicitIntersectEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getMotif())
    {
        intersect.reset();
        return;
    }

    intersect = resetMotif(del, MOTIF_TYPE_EXPLICIT_INTERSECT);

    motifToEditor();
    editorToMotif(doEmit);
}

void ExplicitIntersectEditor::motifToEditor()
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

        NamedMotifEditor::motifToEditor();
    }
}

void ExplicitIntersectEditor::editorToMotif(bool doEmit)
{
    auto isect = intersect.lock();
    if (isect)
    {
        isect->dump();
        int starSides       = side->value();
        qreal starSkip      = skip->value();
        int sval            = s->value();
        bool progressive    = progressive_box->isChecked();

        isect->setN(starSides);
        isect->skip  = starSkip;
        isect->s     = sval;
        isect->progressive = progressive;

        NamedMotifEditor::editorToMotif(false);

        auto proto  = motifMaker->getSelectedPrototype();
        auto tile   = motifMaker->getActiveTile();
        isect->setupInfer(proto);
        isect->newExplicitMap();
        if (progressive)
        {
            isect->inferIntersectProgressive(tile  , starSides, starSkip, sval);
        }
        else
        {
            isect->inferIntersect(tile  , starSides, starSkip, sval);
        }
        isect->setMotifBoundary(tile  ->getPolygon());
        isect->dump();

        if (doEmit)
            emit sig_motif_modified(isect);
    }
}

////////////////////////////////////////////////////////////////////////////
//
// ExplicitRosetteEditor.java
//
// The editing controls for explicit motifs.  A simple class, because
// (right now) explicit motifs don't have any editing controls.  All
// you can do is ask the motif to be inferred from the rest of the
// Prototype.  So all we need is one button.
//
// If I have time (I've got about 36 hours until the deadline), this
// is the place to expend lots of effort.  Add the ability to edit
// the explicit map directly by hand, beginning with a vertex in the
// centre of every edge of the tile.

// casper - interesting but somewhat inscrutable comment
// this implementation inherits the rosette editor

ExplicitRosetteEditor::ExplicitRosetteEditor(page_motif_maker * ed, QString aname) : ExplicitEditor(ed, aname)
{
    q_slider = new DoubleSliderSet("Explicit Rosette Q (Tip Angle)", 0.0, -3.0, 3.0, 100 );
    s_slider = new SliderSet("Explicit Rosette S (Sides Intersections)", 1, 1, 5);
    r_slider = new DoubleSliderSet("Explicit Rosette R (Flex Point)", 0.5, 0.0, 1.0, 100 );

    addLayout(q_slider);
    addLayout(s_slider);
    addLayout(r_slider);

    connect(q_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(r_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
}

void ExplicitRosetteEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getMotif())
    {
        expRose.reset();
        return;
    }

    expRose =resetMotif(del,MOTIF_TYPE_EXPLICIT_ROSETTE);

    motifToEditor();
    editorToMotif(doEmit);
}

void ExplicitRosetteEditor::motifToEditor()
{
    auto erfig = expRose.lock();
    if (erfig)
    {
        blockSignals(true);

        TilePtr tile = menu->getActiveTile();
        if (tile)
        {
            int n = tile->numPoints();
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

        NamedMotifEditor::motifToEditor();
    }
}

void ExplicitRosetteEditor::editorToMotif(bool doEmit)
{
    auto erfig = expRose.lock();
    if (erfig)
    {
        erfig->dump();
        qreal qval = q_slider->value();
        int   sval = s_slider->value();
        qreal rval = r_slider->value();

        erfig->q = qval;
        erfig->s = sval;
        erfig->r_flexPt = rval;

        NamedMotifEditor::editorToMotif(false);

        auto proto = motifMaker->getSelectedPrototype();
        auto tile  = motifMaker->getActiveTile();
        erfig->setupInfer(proto);
        erfig->newExplicitMap();
        erfig->inferRosette(tile , qval, sval, rval);
        erfig->setMotifBoundary(motifMaker->getActiveTile()->getPolygon());
        erfig->dump();

        if (doEmit)
            emit sig_motif_modified(erfig);
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
    d_slider = new DoubleSliderSet("Explicit Star D", 0.0, 0.0, 1.0, 100);
    s_slider = new SliderSet("Explicit Star S", 0.0, 0.0, 1.0);

    addLayout(d_slider);
    addLayout(s_slider);

    connect(d_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
}

void ExplicitStarEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getMotif())
    {
        expStar.reset();
        return;
    }

    expStar = resetMotif(del,MOTIF_TYPE_EXPLICIT_STAR);

    motifToEditor();
    editorToMotif(doEmit);
}

void ExplicitStarEditor::motifToEditor()
{
    auto esfig = expStar.lock();
    if (esfig)
    {
        TilePtr tile = menu->getActiveTile();
        if (tile)
        {
            int n = tile->numPoints();

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

        NamedMotifEditor::motifToEditor();
    }
}

void ExplicitStarEditor::editorToMotif(bool doEmit)
{
    auto esfig = expStar.lock();
    if (esfig)
    {
        esfig->dump();
        qreal dval = d_slider->value();
        int sval   = s_slider->value();

        esfig->d = dval;
        esfig->s = sval;

        NamedMotifEditor::editorToMotif(false);

        auto proto = motifMaker->getSelectedPrototype();
        auto tile  = motifMaker->getActiveTile();
        esfig->setupInfer(proto);
        esfig->newExplicitMap();
        esfig->inferStar(tile, dval, sval);
        esfig->setMotifBoundary(motifMaker->getActiveTile()->getPolygon());
        esfig->dump();

        if (doEmit)
            emit sig_motif_modified(esfig);
    }
}

//  Make a motif from a feattileure
ExplicitTileEditor::ExplicitTileEditor(page_motif_maker * ed, QString aname) : ExplicitEditor(ed, aname)
{
}

void ExplicitTileEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getMotif())
    {
        tileMotif.reset();
        return;
    }

    tileMotif = resetMotif(del,MOTIF_TYPE_EXPLICIT_TILE);

    motifToEditor();
    editorToMotif(doEmit);
}

void ExplicitTileEditor::editorToMotif(bool doEmit)
{
    auto ffig = tileMotif.lock();
    if (ffig)
    {
        ffig->dump();
        NamedMotifEditor::editorToMotif(false);

        auto proto = motifMaker->getSelectedPrototype();
        auto tile  = motifMaker->getActiveTile();
        ffig->setupInfer(proto);
        ffig->newExplicitMap();
        ffig->inferMotif(tile );
        ffig->setMotifBoundary(motifMaker->getActiveTile()->getPolygon());
        ffig->dump();

        if  (doEmit)
        {
            emit sig_motif_modified(ffig);
        }
    }
}
