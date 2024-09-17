#include <QDebug>
#include <QCheckBox>

#include "gui/model_editors/motif_edit/irregular_motif_editors.h"
#include "model/motifs/explicit_map_motif.h"
#include "model/motifs/radial_motif.h"
#include "model/makers/prototype_maker.h"
#include "model/prototypes/design_element.h"
#include "gui/panels/page_motif_maker.h"
#include "model/tilings/tile.h"
#include "gui/widgets/layout_sliderset.h"

using std::make_shared;
using std::dynamic_pointer_cast;

typedef shared_ptr<RadialMotif>    RadialPtr;

#if (QT_VERSION >= QT_VERSION_CHECK(6,7,0))
#define CBOX_STATECHANGE &QCheckBox::checkStateChanged
#else
#define CBOX_STATECHANGE &QCheckBox::stateChanged
#endif

////////////////////////////////////////////////////////////////////////////
//
// ExplicitMapEditor
//

ExplicitMapEditor::ExplicitMapEditor(QString aname, DesignElementPtr del, bool doEmit) : NamedMotifEditor(aname)
{
    wDel = del;
    if (!del || !del->getMotif())
    {
        return;
    }

    auto motif = del->getMotif();

    shared_ptr<ExplicitMapMotif> explMap;
    if (motif->getMotifType() == MOTIF_TYPE_EXPLICIT_MAP)
    {
        explMap = dynamic_pointer_cast<ExplicitMapMotif>(motif);
    }
    else
    {
        auto map = motif->getMotifMap();
        explMap = make_shared<ExplicitMapMotif>(map);
        del->setMotif(explMap);
    }

    explMap->setTile(del->getTile());

    NamedMotifEditor::setMotif(explMap,doEmit);

    motifToEditor();
    editorToMotif(doEmit);
    explMap->buildMotifMap();
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

GirihEditor::GirihEditor(QString aname, DesignElementPtr del, bool doEmit) : NamedMotifEditor (aname)
{
    skip = new DoubleSliderSet("Explicit Girih Skip D", 3.0, 0.0, 12.0, 100);

    addLayout(skip);

    connect(skip, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});

    w_girih.reset();
    wDel = del;
    if (!del || !del->getMotif())
    {
        return;
    }

    auto motif = del->getMotif();

    shared_ptr<GirihMotif> girih;
    if (motif->getMotifType() == MOTIF_TYPE_GIRIH)
    {
        girih = dynamic_pointer_cast<GirihMotif>(motif);
    }
    else
    {
        girih = make_shared<GirihMotif>(*motif.get());
        del->setMotif(girih);
    }

    w_girih = girih;
    girih->setTile(del->getTile());
    NamedMotifEditor::setMotif(girih,doEmit);

    GirihEditor::motifToEditor();
    GirihEditor::editorToMotif(doEmit);
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

        girih->buildMotifMap();

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

HourglassEditor::HourglassEditor(QString aname, DesignElementPtr del, bool doEmit) : NamedMotifEditor(aname)
{
    // Hourglass panel.
    d = new DoubleSliderSet("Explicit Hourglass D", 2.0, 1.0, 12.0, 100);
    s = new SliderSet("Explicit Hourglass SH", 1, 1, 12);

    addLayout(d);
    addLayout(s);

    connect(d, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});

    w_hourglass.reset();
    wDel = del;
    if (!del || !del->getMotif())
    {
        return;
    }

    auto motif = del->getMotif();

    shared_ptr<HourglassMotif> hour;
    if (motif->getMotifType() == MOTIF_TYPE_HOURGLASS)
    {
        hour = dynamic_pointer_cast<HourglassMotif>(motif);
    }
    else
    {
        hour = make_shared<HourglassMotif>(*motif.get());
        del->setMotif(hour);
    }

    w_hourglass = hour;
    hour->setTile(del->getTile());

    HourglassEditor::motifToEditor();
    HourglassEditor::editorToMotif(doEmit);
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

        hour->buildMotifMap();

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

InferEditor::InferEditor(QString aname, DesignElementPtr del, bool doEmit) : NamedMotifEditor(aname)
{
    w_infer.reset();
    wDel  = del;
    if (!del || !del->getMotif())
    {
        return;
    }

    auto motif = del->getMotif();
    auto proto = Sys::prototypeMaker->getSelectedPrototype();

    shared_ptr<InferredMotif> infer;
    if (motif->getMotifType() == MOTIF_TYPE_INFERRED)
    {
        infer = dynamic_pointer_cast<InferredMotif>(motif);
    }
    else
    {
        infer = make_shared<InferredMotif>(motif);
        del->setMotif(infer);
    }

    w_infer = infer;
    infer->setTile(del->getTile());
    infer->setupInfer(proto);

    NamedMotifEditor::setMotif(infer,doEmit);

    InferEditor::motifToEditor();
    InferEditor::editorToMotif(doEmit);
}

void InferEditor::editorToMotif(bool doEmit)
{
    auto infer = w_infer.lock();
    if (infer)
    {
        infer->dump();
        NamedMotifEditor::editorToMotif(false);

        infer->buildMotifMap();

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

IntersectEditor::IntersectEditor(QString aname, DesignElementPtr del, bool doEmit) : NamedMotifEditor(aname)
{
    skip = new DoubleSliderSet("Explicit Intersect Sides Skip", 3, 0, 12, 100);
    s    = new SliderSet("Explicit Intersect S", 1, 1, 12);
    progressive_box = new QCheckBox("Progressive");

    addLayout(skip);
    addLayout(s);
    addWidget(progressive_box);

    connect(skip, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s,    &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(progressive_box, CBOX_STATECHANGE,    this, [this]() { editorToMotif(true);});

    w_isect.reset();
    wDel = del;
    if (!del || !del->getMotif())
    {
        return;
    }

    auto motif = del->getMotif();

    shared_ptr<IntersectMotif> isect;
    if (motif->getMotifType() == MOTIF_TYPE_INTERSECT)
    {
        isect = dynamic_pointer_cast<IntersectMotif>(motif);
    }
    else
    {
        isect = make_shared<IntersectMotif>(*motif.get());
        del->setMotif(isect);
    }

    w_isect = isect;
    isect->setTile(del->getTile());

    NamedMotifEditor::setMotif(isect,doEmit);

    IntersectEditor::motifToEditor();
    IntersectEditor::editorToMotif(doEmit);
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

        isect->buildMotifMap();

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

IrregularRosetteEditor::IrregularRosetteEditor(QString aname, DesignElementPtr del, bool doEmit) : NamedMotifEditor(aname)
{
    q_slider = new DoubleSliderSet("Explicit Rosette Q (Tip Angle)", 0.0, -3.0, 3.0, 100 );
    r_slider = new DoubleSliderSet("Explicit Rosette R (Flex Point)", 0.5, -1.0, 1.0, 100 );
    s_slider = new SliderSet("Explicit Rosette S (Sides Intersections)", 1, 1, 5);
    version_combo = new QComboBox();
    version_combo->addItem("Version 1",1);
    version_combo->addItem("Version 2",2);
    version_combo->addItem("Version 3",3);
    version_combo->setFixedWidth(91);

    addLayout(q_slider);
    addLayout(r_slider);
    addLayout(s_slider);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(version_combo);
    hbox->addStretch();
    addLayout(hbox);

    connect(q_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(r_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(version_combo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this,[this]() { editorToMotif(true);});

    w_rose.reset();
    wDel = del;
    if (!del || !del->getMotif())
    {
        return;
    }

    auto motif = del->getMotif();

    shared_ptr<IrregularRosette> rose;
    if (motif->getMotifType() == MOTIF_TYPE_IRREGULAR_ROSETTE)
    {
        rose = dynamic_pointer_cast<IrregularRosette>(motif);
    }
    else
    {
        rose = make_shared<IrregularRosette>(*motif.get());
        del->setMotif(rose);
    }

    w_rose = rose;
    rose->setTile(del->getTile());

    NamedMotifEditor::setMotif(rose,doEmit);

    IrregularRosetteEditor::motifToEditor();
    IrregularRosetteEditor::editorToMotif(doEmit);
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
        r_slider->setValues(rose->r,-1.0,1.0);

        blockSignals(false);

        int ver   = rose->getVersion();
        int index = version_combo->findData(ver);
        version_combo->blockSignals(true);
        version_combo->setCurrentIndex(index);
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
        int   ver  = version_combo->currentData().toInt();

        rose->q = qval;
        rose->s = sval;
        rose->r = rval;
        rose->setVersion(ver);

        NamedMotifEditor::editorToMotif(false);

        rose->buildMotifMap();

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

IrregularStarEditor::IrregularStarEditor(QString aname, DesignElementPtr del, bool doEmit) : NamedMotifEditor(aname)
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

    w_star.reset();
    wDel = del;
    if (!del || !del->getMotif())
    {
        return;
    }

    auto motif = del->getMotif();

    shared_ptr<IrregularStar> star;
    if (motif->getMotifType() == MOTIF_TYPE_IRREGULAR_STAR)
    {
        star = dynamic_pointer_cast<IrregularStar>(motif);
    }
    else
    {
        star = make_shared<IrregularStar>(*motif.get());
        del->setMotif(star);
    }

    w_star = star;
    star->setTile(del->getTile());

    NamedMotifEditor::setMotif(star,doEmit);

    IrregularStarEditor::motifToEditor();
    IrregularStarEditor::editorToMotif(doEmit);
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
        int ver   = star->getVersion();
        int index = version_combo->findData(ver);
        version_combo->blockSignals(true);
        version_combo->setCurrentIndex(index);
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
        int   ver  = version_combo->currentData().toInt();

        star->d = dval;
        star->s = sval;
        star->setVersion(ver);

        NamedMotifEditor::editorToMotif(false);

        star->buildMotifMap();

        //star->dump();

        if (doEmit)
            emit sig_motif_modified(star);
    }
}

////////////////////////////////////////////////////////////////////////////
///
//  Make a motif from a Tile
//

ExplicitTileEditor::ExplicitTileEditor(QString aname, DesignElementPtr del, bool doEmit) : NamedMotifEditor(aname)
{
    w_tileMotif.reset();
    wDel = del;
    if (!del || !del->getMotif())
    {
        return;
    }

    auto motif = del->getMotif();

    shared_ptr<TileMotif> tmotif;
    if (motif->getMotifType() == MOTIF_TYPE_EXPLCIT_TILE)
    {
        tmotif = dynamic_pointer_cast<TileMotif>(motif);
    }
    else
    {
        tmotif = make_shared<TileMotif>(*motif.get());
        del->setMotif(tmotif);
    }

    w_tileMotif = tmotif;
    tmotif->setTile(del->getTile());

    NamedMotifEditor::setMotif(tmotif,doEmit);

    motifToEditor();
    ExplicitTileEditor::editorToMotif(doEmit);
}

void ExplicitTileEditor::editorToMotif(bool doEmit)
{
    auto etile = w_tileMotif.lock();
    if (etile)
    {
        etile->dump();

        NamedMotifEditor::editorToMotif(false);

        etile->buildMotifMap();

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

IrregularNoMapEditor::IrregularNoMapEditor(QString aname, DesignElementPtr del, bool doEmit) : NamedMotifEditor(aname)
{
    wDel = del;
    if (!del)
    {
        return;
    }

    auto motif = del->getMotif();
    auto nomap = make_shared<IrregularNoMap>(*motif.get());
    nomap->setTile(del->getTile());

    NamedMotifEditor::setMotif(nomap,doEmit);

    motifToEditor();
    editorToMotif(doEmit);
}
