#include <QDebug>
#include <QCheckBox>

#include "gui/model_editors/motif_edit/adaptive_motif_editors.h"
#include "gui/widgets/layout_sliderset.h"
#include "model/makers/motif_maker.h"
#include "model/motifs/explicit_map_motif.h"
#include "sys/sys.h"

using std::make_shared;
using std::dynamic_pointer_cast;

typedef shared_ptr<RadialMotif>    RadialPtr;

////////////////////////////////////////////////////////////////////////////
//
// ExplicitMapEditor
//
////////////////////////////////////////////////////////////////////////////

ExplicitMapEditor::ExplicitMapEditor(QString aname, shared_ptr<ExplicitMapMotif> explMap) : MotifEditor(aname)
{
    Q_ASSERT(explMap);
    setMotif(explMap);

    explMap->build();
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
////////////////////////////////////////////////////////////////////////////

GirihEditor::GirihEditor(QString aname, shared_ptr<AdaptiveGirih> girih) : MotifEditor(aname)
{
    skip = new DoubleSliderSet("Explicit Girih Skip D", 3.0, 0.0, 12.0, 100);

    addLayout(skip);

    connect(skip, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});

    Q_ASSERT(girih);
    w_girih = girih;
    setMotif(girih);
}

void GirihEditor::refresh()
{

    auto girih = w_girih.lock();
    if (girih)
    {
        MotifEditor::refresh();

        qreal skipval = girih->skip;

        blockSignals(true);
        skip->setValue(skipval);
        blockSignals(false);
    }
}

void  GirihEditor::editorToMotif(bool doEmit)
{
    auto girih = w_girih.lock();
    if (girih)
    {
        MotifEditor::editorToMotif(false);

        qreal star_skip = skip->value();
        girih->skip  = star_skip;

        girih->build();

        if (doEmit)
        {
            motifModified(girih);
        }
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
////////////////////////////////////////////////////////////////////////////

HourglassEditor::HourglassEditor(QString aname, shared_ptr<AdaptiveHourglass> hour) : MotifEditor(aname)
{
    // Hourglass panel.
    d = new DoubleSliderSet("Explicit Hourglass D", 2.0, 1.0, 12.0, 100);
    s = new SliderSet("Explicit Hourglass SH", 1, 1, 12);

    addLayout(d);
    addLayout(s);

    connect(d, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});

    Q_ASSERT(hour);
    w_hourglass = hour;
    hour->setTile(hour->getTile());
    setMotif(hour);
}

void HourglassEditor::refresh()
{
    auto hour = w_hourglass.lock();
    if (hour)
    {
        MotifEditor::refresh();

        auto tile  = hour->getTile();
        if (tile)
        {
            int n = hour->getN();

            qreal dmin = 1.0;
            qreal dmax = 0.5 * static_cast<qreal>(n);
            qreal dcur = hour->d;
            qreal dd   = std::min(dmax - 0.5, std::max(dmin, dcur));

            qreal smin = 1.0;
            qreal smax = ceil( 0.5 * static_cast<qreal>(n));
            qreal scur = hour->s;
            qreal ss   = floor(std::min( smax, std::max(smin, scur)));

            blockSignals(true);
            d->setValues(dd, dmin, dmax);
            s->setValues(static_cast<int>(ss), static_cast<int>(smin), n);
            blockSignals(false);
        }
        else
        {
            blockSignals(true);
            d->setValue(hour->d);
            s->setValue(hour->s);
            blockSignals(false);
        }
    }
}

void HourglassEditor::editorToMotif(bool doEmit)
{
    auto hour = w_hourglass.lock();
    if (hour)
    {
        qreal dval = d->value();
        int sval   = s->value();

        hour->d = dval;
        hour->s = sval;

        MotifEditor::editorToMotif(false);

        hour->build();

        if  (doEmit)
        {
            motifModified(hour);
        }
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
////////////////////////////////////////////////////////////////////////////

InferEditor::InferEditor(QString aname, shared_ptr<AdaptiveInference> infer) : MotifEditor(aname)
{
    auto proto = Sys::motifMaker->motifData.getActivePrototype();

    Q_ASSERT(infer);
    infer->setupInfer(proto);
    w_infer = infer;
    setMotif(infer);
}

void InferEditor::editorToMotif(bool doEmit)
{
    auto infer = w_infer.lock();
    if (infer)
    {
        MotifEditor::editorToMotif(false);

        infer->build();

        if (doEmit)
        {
            motifModified(infer);
        }
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
////////////////////////////////////////////////////////////////////////////

IntersectEditor::IntersectEditor(QString aname, shared_ptr<AdaptiveIntersect> isect) : MotifEditor(aname)
{
    skip = new DoubleSliderSet("Explicit Intersect Sides Skip", 3, 0, 12, 100);
    s    = new SliderSet("Explicit Intersect S", 1, 1, 12);
    progressive_box = new QCheckBox("Progressive");

    addLayout(skip);
    addLayout(s);
    addWidget(progressive_box);

    connect(skip, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s,    &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(progressive_box, &QCheckBox::clicked, this, [this]() { editorToMotif(true);});

    Q_ASSERT(isect);
    w_isect = isect;
    setMotif(isect);
}

void IntersectEditor::refresh()
{
    auto isect = w_isect.lock();
    if (isect)
    {
        MotifEditor::refresh();

        blockSignals(true);
        skip->setValue(isect->skip);
        s->setValue(isect->s);
        progressive_box->setChecked(isect->progressive);
        blockSignals(false);
    }
}

void IntersectEditor::editorToMotif(bool doEmit)
{
    auto isect = w_isect.lock();
    if (isect)
    {
        MotifEditor::editorToMotif(false);

        qreal starSkip      = skip->value();
        int sval            = s->value();
        bool progressive    = progressive_box->isChecked();

        isect->skip  = starSkip;
        isect->s     = sval;
        isect->progressive = progressive;

        isect->build();

        if (doEmit)
        {
            motifModified(isect);
        }
    }
}

////////////////////////////////////////////////////////////////////////////
//
// ExplicitRosetteEditor.javaX
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
////////////////////////////////////////////////////////////////////////////

IrregularRosetteEditor::IrregularRosetteEditor(QString aname, shared_ptr<AdaptiveRosette> rose) : MotifEditor(aname)
{
    q_slider = new DoubleSliderSet("Explicit Rosette Q (Tip Angle)", 0.0, -3.0, 3.0, 100 );
    r_slider = new DoubleSliderSet("Explicit Rosette R (Flex Point)", 0.5, -1.0, 1.0, 100 );
    s_slider = new SliderSet("Explicit Rosette S (Sides Intersections)", 1, 1, 5);
    k_slider = new DoubleSliderSet("Explicit Rosette K (Knee Angle, v2)", 0.0, -90.0, 90.0, 10);

    addLayout(q_slider);
    addLayout(r_slider);
    addLayout(s_slider);
    addLayout(k_slider);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    addLayout(hbox);

    connect(q_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(r_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(k_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});

    Q_ASSERT(rose);
    w_rose = rose;
    setMotif(rose);
}

void IrregularRosetteEditor::refresh()
{
    auto rose = w_rose.lock();
    if (rose)
    {
        MotifEditor::refresh();

        blockSignals(true);

        auto tile  = rose->getTile();
        if (tile)
        {
            int n = tile->numPoints();
            qreal smin = 1.0;
            qreal smax = ceil( 0.5 * static_cast<qreal>(n));
            qreal scur = rose->s;
            qreal ss   = floor(std::min(smax, std::max(smin, scur)));
            s_slider->setValues(static_cast<int>(ss), static_cast<int>(smin), static_cast<int>(smax));
        }
        else
        {
            int ss = rose->s;
            s_slider->setValues(ss, 1.0, 5);
        }

        q_slider->setValues(rose->q, -3.0, 3.0);       // DAC was -1.0, 1.0
        r_slider->setValues(rose->r,-1.0,1.0);
        k_slider->setValues(rose->k,-90.0,90.0);

        blockSignals(false);
    }
}

void IrregularRosetteEditor::editorToMotif(bool doEmit)
{
    auto rose = w_rose.lock();
    if (rose)
    {
        MotifEditor::editorToMotif(false);

        qreal qval = q_slider->value();
        int   sval = s_slider->value();
        qreal rval = r_slider->value();
        qreal kval = k_slider->value();

        rose->q = qval;
        rose->s = sval;
        rose->r = rval;
        rose->k = kval;

        rose->build();

        if (doEmit)
        {
            motifModified(rose);
        }
    }
}

////////////////////////////////////////////////////////////////////////////
//
// ExplicitStarEditor.java
//
// The controls for editing a Star.  Glue code, just like RosetteEditor.
//
////////////////////////////////////////////////////////////////////////////
IrregularStarEditor::IrregularStarEditor(QString aname, shared_ptr<AdaptiveStar> star) : MotifEditor(aname)
{
    d_slider = new DoubleSliderSet("Explicit Star D", 0.0, 0.0, 1.0, 100);
    s_slider = new SliderSet("Explicit Star S", 0.0, 0.0, 1.0);

    addLayout(d_slider);
    addLayout(s_slider);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    addLayout(hbox);

    connect(d_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});

    Q_ASSERT(star);
    w_star = star;
    setMotif(star);
}

void IrregularStarEditor::refresh()
{
    auto star = w_star.lock();
    if (star)
    {
        auto tile  = star->getTile();
        if (tile)
        {
            MotifEditor::refresh();

            int n = tile->numPoints();
#if 0
            qreal dmin = 1.0;
            qreal dmax = 0.5 * static_cast<qreal>(n);
            qreal dcur = star->d;
            qreal dd   = std::min(dmax - 0.5, std::max(dmin, dcur));

            qreal smin = 1.0;
            qreal smax = ceil( 0.5 * static_cast<qreal>(n));
            qreal scur = star->s;
            qreal ss   = floor(std::min(smax, std::max(smin, scur)));

            blockSignals(true);
            d_slider->setValues(dd, dmin, dmax);
            s_slider->setValues(static_cast<int>(ss), static_cast<int>(smin), static_cast<int>(smax));
            blockSignals(false);
#else
            qreal  dd = star->d;
            int    ss = star->s;

            blockSignals(true);
            d_slider->setValues(dd, 0.5, n * 2.0);
            s_slider->setValues(ss, 1, n * 2);
            blockSignals(false);
#endif
        }
        else
        {
            blockSignals(true);
            d_slider->setValue(star->d);
            s_slider->setValue(star->s);
            blockSignals(false);
        }
    }
    else
    {
        blockSignals(true);
        d_slider->setValue(0);
        s_slider->setValue(0);
        blockSignals(false);
    }
}

void IrregularStarEditor::editorToMotif(bool doEmit)
{
    auto star = w_star.lock();
    if (star)
    {
        MotifEditor::editorToMotif(false);

        qreal dval = d_slider->value();
        int sval   = s_slider->value();

        star->d = dval;
        star->s = sval;

        star->build();

        if (doEmit)
        {
            motifModified(star);
        }
    }
}

////////////////////////////////////////////////////////////////////////////
///
//  Make a motif from a Tile
//
////////////////////////////////////////////////////////////////////////////

ExplicitTileEditor::ExplicitTileEditor(QString aname, shared_ptr<AdaptiveTile> tmotif) : MotifEditor(aname)
{
    Q_ASSERT(tmotif);
    w_tileMotif = tmotif;
    setMotif(tmotif);
}

void ExplicitTileEditor::editorToMotif(bool doEmit)
{
    auto etile = w_tileMotif.lock();
    if (etile)
    {
        MotifEditor::editorToMotif(false);

        etile->build();

        if  (doEmit)
        {
            motifModified(etile);
        }
    }
}

////////////////////////////////////////////////////////////////////////////
//
// Irregular No Map Editor
//
////////////////////////////////////////////////////////////////////////////

IrregularNoMapEditor::IrregularNoMapEditor(QString aname, shared_ptr<IrregularNoMap> nomap) : MotifEditor(aname)
{
    Q_ASSERT(nomap);
    setMotif(nomap);
}
