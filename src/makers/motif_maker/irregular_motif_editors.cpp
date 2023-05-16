#include <QDebug>
#include <QCheckBox>

#include "makers/motif_maker/irregular_motif_editors.h"
#include "motifs/explicit_map_motif.h"
#include "motifs/radial_motif.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "mosaic/design_element.h"
#include "panels/page_motif_maker.h"
#include "tile/tile.h"
#include "widgets/layout_sliderset.h"

using std::make_shared;
using std::dynamic_pointer_cast;

typedef shared_ptr<RadialMotif>    RadialPtr;

////////////////////////////////////////////////////////////////////////////
//
// ExplicitMapEditor
//

ExplicitMapEditor::ExplicitMapEditor(QString aname) : NamedMotifEditor(aname)
{
    prototypeMaker = PrototypeMaker::getInstance();
}

void ExplicitMapEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    wExplicitMapMotif.reset();
    wDel = del;
    if (!del || !del->getMotif())
    {
        return;
    }

    auto motif = del->getMotif();
    if (motif->getMotifType() == MOTIF_TYPE_EXPLICIT_MAP)
    {
        try
        {
            auto explMap = dynamic_pointer_cast<ExplicitMapMotif>(motif);
            NamedMotifEditor::setMotif(explMap,doEmit);
            wExplicitMapMotif = explMap;
        }
        catch (...)
        {
           qWarning() << "Bad cast in ExplicitMapEditor (1)";
        }
    }
    else
    {
        auto map = motif->getMotifMap();
        auto explMap = make_shared<ExplicitMapMotif>(map);
        del->setMotif(explMap);
        NamedMotifEditor::setMotif(explMap,doEmit);
        wExplicitMapMotif = explMap;
    }

    motifToEditor();
    editorToMotif(doEmit);
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

GirihEditor::GirihEditor(QString aname) : ExplicitMapEditor (aname)
{
    skip = new DoubleSliderSet("Explicit Girih Skip D", 3.0, 0.0, 12.0, 100);

    addLayout(skip);

    connect(skip, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
}

void  GirihEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    w_girih.reset();
    wDel = del;
    if (!del || !del->getMotif())
    {
        return;
    }

    auto motif = del->getMotif();
    if (motif->getMotifType() == MOTIF_TYPE_GIRIH)
    {
        auto girih = dynamic_pointer_cast<GirihMotif>(motif);
        w_girih = girih;
        NamedMotifEditor::setMotif(girih,doEmit);
    }
    else
    {
        auto  girih = make_shared<GirihMotif>(*motif.get());
        del->setMotif(girih);
        w_girih = girih;
        NamedMotifEditor::setMotif(girih,doEmit);
    }

    motifToEditor();
    editorToMotif(doEmit);
}

void GirihEditor::motifToEditor()
{
    auto girih = w_girih.lock();
    if (!girih)
        return;

    qreal skipval = girih->skip;

    blockSignals(true);
    skip->setValue(skipval);
    blockSignals(false);

    NamedMotifEditor::motifToEditor();
}

void  GirihEditor::editorToMotif(bool doEmit)
{
    auto girih = w_girih.lock();
    if (girih)
    {
        girih->dump();
        qreal star_skip = skip->value();
        girih->skip  = star_skip;

        NamedMotifEditor::editorToMotif(false);

        girih->buildMotifMaps();

        auto del = wDel.lock();
        if (del)
        {
           auto tile  = del->getTile();
            girih->setMotifBoundary(tile->getPolygon());
        }

        girih->dump();

        if (doEmit)
            emit sig_motif_modified(girih);
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

HourglassEditor::HourglassEditor(QString aname) : ExplicitMapEditor(aname)
{
    // Hourglass panel.
    d = new DoubleSliderSet("Explicit Hourglass D", 2.0, 1.0, 12.0, 100);
    s = new SliderSet("Explicit Hourglass SH", 1, 1, 12);

    addLayout(d);
    addLayout(s);

    connect(d, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
}

void HourglassEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    w_hourglass.reset();
    wDel = del;
    if (!del || !del->getMotif())
    {
        return;
    }

    auto motif = del->getMotif();

    if (motif->getMotifType() == MOTIF_TYPE_HOURGLASS)
    {
        auto hour = dynamic_pointer_cast<HourglassMotif>(motif);
        w_hourglass = hour;
        NamedMotifEditor::setMotif(hour,doEmit);
    }
    else
    {
        auto hour = make_shared<HourglassMotif>(*motif.get());
        del->setMotif(hour);
        w_hourglass = hour;
        NamedMotifEditor::setMotif(hour,doEmit);
    }

    motifToEditor();
    editorToMotif(doEmit);
}

void HourglassEditor::motifToEditor()
{
    auto hour = w_hourglass.lock();
    if (hour)
    {
        auto del = wDel.lock();
        if (del)
        {
            auto tile  = del->getTile();
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
        else
        {
            blockSignals(true);
            d->setValue(hour->d);
            s->setValue(hour->s);
            blockSignals(false);
        }
    }
    NamedMotifEditor::motifToEditor();
}

void HourglassEditor::editorToMotif(bool doEmit)
{
    auto hour = w_hourglass.lock();
    if (hour)
    {
        hour->dump();
        qreal dval = d->value();
        int sval   = s->value();

        hour->d = dval;
        hour->s = sval;

        NamedMotifEditor::editorToMotif(false);

        hour->buildMotifMaps();

        auto del = wDel.lock();
        if (del)
        {
            auto tile  = del->getTile();
            hour->setMotifBoundary(tile->getPolygon());
        }

        hour->dump();

        if  (doEmit)
            emit sig_motif_modified(hour);
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

InferEditor::InferEditor(QString aname) : ExplicitMapEditor(aname)
{
}

void InferEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    w_infer.reset();
    wDel  = del;
    if (!del || !del->getMotif())
    {
        return;
    }

    auto motif = del->getMotif();
    auto proto = prototypeMaker->getProtoMakerData()->getSelectedPrototype();

    if (motif->getMotifType() == MOTIF_TYPE_INFERRED)
    {
        auto infer = dynamic_pointer_cast<InferredMotif>(motif);
        infer->setupInfer(proto);
        w_infer = infer;
        NamedMotifEditor::setMotif(infer,doEmit);
    }
    else
    {
        auto infer = make_shared<InferredMotif>(motif);
        qDebug() << "The new motif is:" << infer.get();
        infer->setupInfer(proto);
        del->setMotif(infer);
        w_infer = infer;
        NamedMotifEditor::setMotif(infer,doEmit);
    }

    motifToEditor();
    editorToMotif(doEmit);
}

void InferEditor::editorToMotif(bool doEmit)
{
    auto infer = w_infer.lock();
    if (infer)
    {
        infer->dump();
        NamedMotifEditor::editorToMotif(false);

        infer->buildMotifMaps();

        auto del = wDel.lock();
        if (del)
        {
            auto tile  = del->getTile();
            infer->setMotifBoundary(tile->getPolygon());
        }

        infer->dump();

        if (doEmit)
            emit sig_motif_modified(infer);
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

IntersectEditor::IntersectEditor(QString aname) : ExplicitMapEditor(aname)
{
    skip = new DoubleSliderSet("Explicit Intersect Sides Skip", 3, 0, 12, 100);
    s    = new SliderSet("Explicit Intersect S", 1, 1, 12);
    progressive_box = new QCheckBox("Progressive");

    addLayout(skip);
    addLayout(s);
    addWidget(progressive_box);

    connect(skip, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s,    &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(progressive_box, &QCheckBox::stateChanged, this, [this]() { editorToMotif(true);});
}

void IntersectEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    w_isect.reset();
    wDel = del;
    if (!del || !del->getMotif())
    {
        return;
    }

    auto motif = del->getMotif();

    if (motif->getMotifType() == MOTIF_TYPE_INTERSECT)
    {
        auto isect = dynamic_pointer_cast<IntersectMotif>(motif);
        w_isect = isect;
        NamedMotifEditor::setMotif(isect,doEmit);
    }
    else
    {
        auto isect = make_shared<IntersectMotif>(*motif.get());
        del->setMotif(isect);
        w_isect = isect;
        NamedMotifEditor::setMotif(isect,doEmit);
    }

    motifToEditor();
    editorToMotif(doEmit);
}

void IntersectEditor::motifToEditor()
{
    auto isect = w_isect.lock();
    if (isect)
    {
        blockSignals(true);
        skip->setValue(isect->skip);
        s->setValue(isect->s);
        progressive_box->setChecked(isect->progressive);
        blockSignals(false);

        NamedMotifEditor::motifToEditor();
    }
}

void IntersectEditor::editorToMotif(bool doEmit)
{
    auto isect = w_isect.lock();
    if (isect)
    {
        isect->dump();
        qreal starSkip      = skip->value();
        int sval            = s->value();
        bool progressive    = progressive_box->isChecked();

        isect->skip  = starSkip;
        isect->s     = sval;
        isect->progressive = progressive;

        NamedMotifEditor::editorToMotif(false);

        isect->buildMotifMaps();

        auto del = wDel.lock();
        if (del)
        {
            auto tile  = del->getTile();
            isect->setMotifBoundary(tile->getPolygon());
        }

        isect->dump();

        if (doEmit)
            emit sig_motif_modified(isect);
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
// this implementation inherits the rosette editor

IrregularRosetteEditor::IrregularRosetteEditor(QString aname) : ExplicitMapEditor(aname)
{
    q_slider = new DoubleSliderSet("Explicit Rosette Q (Tip Angle)", 0.0, -3.0, 3.0, 100 );
    s_slider = new SliderSet("Explicit Rosette S (Sides Intersections)", 1, 1, 5);
    r_slider = new DoubleSliderSet("Explicit Rosette R (Flex Point)", 0.5, 0.0, 1.0, 100 );
    version_combo = new QComboBox();
    version_combo->addItem("Version 1",1);
    version_combo->addItem("Version 2",2);
    version_combo->setFixedWidth(91);

    addLayout(q_slider);
    addLayout(s_slider);
    addLayout(r_slider);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(version_combo);
    hbox->addStretch();
    addLayout(hbox);

    connect(q_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(r_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(version_combo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this,[this]() { editorToMotif(true);});
}

void IrregularRosetteEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    w_rose.reset();
    wDel = del;
    if (!del || !del->getMotif())
    {
        return;
    }

    auto motif = del->getMotif();

    if (motif->getMotifType() == MOTIF_TYPE_IRREGULAR_ROSETTE)
    {
        auto rose = dynamic_pointer_cast<IrregularRosette>(motif);
        w_rose = rose;
        NamedMotifEditor::setMotif(rose,doEmit);
    }
    else
    {
        auto rose = make_shared<IrregularRosette>(*motif.get());
        del->setMotif(rose);
        w_rose = rose;
        NamedMotifEditor::setMotif(rose,doEmit);
    }

    motifToEditor();
    editorToMotif(doEmit);
}

void IrregularRosetteEditor::motifToEditor()
{
    auto rose = w_rose.lock();
    if (rose)
    {
        blockSignals(true);

        auto del = wDel.lock();
        if (del)
        {
            auto tile  = del->getTile();
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
        }
        else
        {
            int ss = rose->s;
            s_slider->setValues(ss, 1.0, 5);
        }

        q_slider->setValues(rose->q, -3.0, 3.0);       // DAC was -1.0, 1.0
        r_slider->setValues(rose->r,0.0,1.0);

        blockSignals(false);

        int ver = rose->getVersion();
        version_combo->blockSignals(true);
        version_combo->setCurrentIndex(ver -1);
        version_combo->blockSignals(false);

        NamedMotifEditor::motifToEditor();
    }
}

void IrregularRosetteEditor::editorToMotif(bool doEmit)
{
    auto rose = w_rose.lock();
    if (rose)
    {
        rose->dump();
        qreal qval = q_slider->value();
        int   sval = s_slider->value();
        qreal rval = r_slider->value();
        int   ver  = version_combo->currentIndex() + 1;

        rose->q = qval;
        rose->s = sval;
        rose->r = rval;
        rose->setVersion(ver);

        NamedMotifEditor::editorToMotif(false);

        rose->buildMotifMaps();

        auto del = wDel.lock();
        if (del)
        {
            auto tile  = del->getTile();
            rose->setMotifBoundary(tile->getPolygon());
        }

        rose->dump();

        if (doEmit)
        {
            qDebug() << "sig_motif_modified";
            emit sig_motif_modified(rose);
        }
    }
}

////////////////////////////////////////////////////////////////////////////
//
// ExplicitStarEditor.java
//
// The controls for editing a Star.  Glue code, just like RosetteEditor.
// caser pthis implementation inherits the rosette editor

IrregularStarEditor::IrregularStarEditor(QString aname) : ExplicitMapEditor(aname)
{
    d_slider = new DoubleSliderSet("Explicit Star D", 0.0, 0.0, 1.0, 100);
    s_slider = new SliderSet("Explicit Star S", 0.0, 0.0, 1.0);
    version_combo = new QComboBox();
    version_combo->addItem("Version 1",1);
    version_combo->addItem("Version 2",2);
    version_combo->addItem("Version 3",3);
    version_combo->setFixedWidth(91);

    addLayout(d_slider);
    addLayout(s_slider);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(version_combo);
    hbox->addStretch();
    addLayout(hbox);

    connect(d_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(version_combo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this,[this]() { editorToMotif(true);});
}

void IrregularStarEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    w_star.reset();
    wDel = del;
    if (!del || !del->getMotif())
    {
        return;
    }

    auto motif = del->getMotif();

    if (motif->getMotifType() == MOTIF_TYPE_IRREGULAR_STAR)
    {
        auto star = dynamic_pointer_cast<IrregularStar>(motif);
        w_star = star;
        NamedMotifEditor::setMotif(star,doEmit);
    }
    else
    {
        auto star = make_shared<IrregularStar>(*motif.get());
        del->setMotif(star);
        w_star = star;
        NamedMotifEditor::setMotif(star,doEmit);
    }

    motifToEditor();
    editorToMotif(doEmit);
}

void IrregularStarEditor::motifToEditor()
{
    auto star = w_star.lock();
    if (star)
    {
        auto del = wDel.lock();
        if (del)
        {
            auto tile  = del->getTile();
            if (tile)
            {
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
            d_slider->setValue(star->d);
            s_slider->setValue(star->s);
            blockSignals(false);
        }
        int ver = star->getVersion();
        version_combo->blockSignals(true);
        version_combo->setCurrentIndex(ver -1);
        version_combo->blockSignals(false);

        NamedMotifEditor::motifToEditor();
    }
}

void IrregularStarEditor::editorToMotif(bool doEmit)
{
    auto star = w_star.lock();
    if (star)
    {
        //star->dump();
        qreal dval = d_slider->value();
        int sval   = s_slider->value();
        int   ver  = version_combo->currentIndex() + 1;

        star->d = dval;
        star->s = sval;
        star->setVersion(ver);

        NamedMotifEditor::editorToMotif(false);

        star->buildMotifMaps();

        auto del = wDel.lock();
        if (del)
        {
            auto tile  = del->getTile();
            star->setMotifBoundary(tile->getPolygon());
        }

        //star->dump();

        if (doEmit)
            emit sig_motif_modified(star);
    }
}

////////////////////////////////////////////////////////////////////////////
///
//  Make a motif from a Tile
//

ExplicitTileEditor::ExplicitTileEditor(QString aname) : ExplicitMapEditor(aname)
{
}

void ExplicitTileEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    w_tileMotif.reset();
    wDel = del;
    if (!del || !del->getMotif())
    {
        return;
    }

    auto motif = del->getMotif();

    if (motif->getMotifType() == MOTIF_TYPE_EXPLCIT_TILE)
    {
        auto tmotif = dynamic_pointer_cast<TileMotif>(motif);
        tmotif->setup(del->getTile());
        w_tileMotif = tmotif;
        NamedMotifEditor::setMotif(tmotif,doEmit);
    }
    else
    {
        auto tmotif = make_shared<TileMotif>(*motif.get());
        tmotif->setup(del->getTile());
        del->setMotif(tmotif);
        w_tileMotif = tmotif;
        NamedMotifEditor::setMotif(tmotif,doEmit);
    }

    motifToEditor();
    editorToMotif(doEmit);
}

void ExplicitTileEditor::editorToMotif(bool doEmit)
{
    auto etile = w_tileMotif.lock();
    if (etile)
    {
        etile->dump();

        NamedMotifEditor::editorToMotif(false);

        etile->buildMotifMaps();

        auto del = wDel.lock();
        if (del)
        {
            auto tile  = del->getTile();
            etile->setMotifBoundary(tile->getPolygon());
        }

        etile->dump();

        if  (doEmit)
        {
            emit sig_motif_modified(etile);
        }
    }
}

////////////////////////////////////////////////////////////////////////////
//
// Irregular No Map Editor
//

IrregularNoMapEditor::IrregularNoMapEditor(QString aname) : NamedMotifEditor(aname)
{
    prototypeMaker = PrototypeMaker::getInstance();
}

void IrregularNoMapEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    wDel = del;
    if (!del)
    {
        return;
    }

    auto motif = make_shared<IrregularMotif>();
    NamedMotifEditor::setMotif(motif,doEmit);

    motifToEditor();
    editorToMotif(doEmit);
}
