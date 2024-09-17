#pragma once
#ifndef EXPLICIT_MOTIF_EDITORS_H
#define EXPLICIT_MOTIF_EDITORS_H

#include "sys/enums/emotiftype.h"
#include "gui/model_editors/motif_edit/regular_motif_editors.h"
#include "model/motifs/girih_motif.h"
#include "model/motifs/hourglass_motif.h"
#include "model/motifs/inferred_motif.h"
#include "model/motifs/intersect_motif.h"
#include "model/motifs/irregular_rosette.h"
#include "model/motifs/irregular_star.h"
#include "model/motifs/tile_motif.h"

using std::shared_ptr;
using std::weak_ptr;

class ExplicitMapEditor : public  NamedMotifEditor
{
    Q_OBJECT

public:
    ExplicitMapEditor(QString aname,DesignElementPtr del, bool doEmit);

protected:
};

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
// centre of every edge of the feature.


class GirihEditor : public NamedMotifEditor
{
public:
    GirihEditor(QString aname,DesignElementPtr del, bool doEmit);

private:
    void editorToMotif(bool doEmit) override;
    void motifToEditor() override;

    DoubleSliderSet * skip;

    weak_ptr<GirihMotif>   w_girih;
};

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
// centre of every edge of the feature.

class HourglassEditor : public NamedMotifEditor
{
public:
    HourglassEditor(QString aname,DesignElementPtr del, bool doEmit);

private:
    void motifToEditor() override;
    void editorToMotif(bool doEmit) override;

    DoubleSliderSet  * d;
    SliderSet        * s;

    weak_ptr<HourglassMotif>  w_hourglass;
};

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
// centre of every edge of the feature.

class InferEditor : public  NamedMotifEditor
{
    Q_OBJECT

public:
    InferEditor(QString aname,DesignElementPtr del, bool doEmit);

private:
    void editorToMotif(bool doEmit) override;

    weak_ptr<InferredMotif>   w_infer;
};

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
// centre of every edge of the feature.

class IntersectEditor : public NamedMotifEditor
{
    Q_OBJECT

public:
    IntersectEditor(QString aname, DesignElementPtr del, bool doEmit);

private:
    void editorToMotif(bool doEmit) override;
    void motifToEditor() override;

    DoubleSliderSet        * skip;
    SliderSet              * s;
    QCheckBox              * progressive_box;

    weak_ptr<IntersectMotif> w_isect;
};

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
// centre of every edge of the feature.

class IrregularRosetteEditor : public NamedMotifEditor
{
    Q_OBJECT

public:
    IrregularRosetteEditor(QString aname,DesignElementPtr del, bool doEmit);

private:
    void motifToEditor() override;
    void editorToMotif(bool doEmit) override;

    DoubleSliderSet	*	q_slider;
    SliderSet       *   s_slider;
    DoubleSliderSet	*	r_slider;
    QComboBox       *   version_combo;

    weak_ptr<IrregularRosette>  w_rose;
};

////////////////////////////////////////////////////////////////////////////
//
// ExplicitStarEditor.java
//
// The controls for editing a Star.  Glue code, just like RosetteEditor.

class IrregularStarEditor : public NamedMotifEditor
{
    Q_OBJECT

public:
    IrregularStarEditor(QString aname,DesignElementPtr del, bool doEmit);

private:
    void editorToMotif(bool doEmit) override;
    void motifToEditor() override;

    SliderSet       * s_slider;
    DoubleSliderSet * d_slider;
    QComboBox       * version_combo;

    weak_ptr<IrregularStar>   w_star;
};


///////////////////////////////////////////////////////////////////////////
//
// ExplicitTileEditor
//
///////////////////////////////////////////////////////////////////////////

class ExplicitTileEditor : public NamedMotifEditor
{
    Q_OBJECT

public:
    ExplicitTileEditor(QString aname,DesignElementPtr del, bool doEmit);

private:
    void editorToMotif(bool doEmit) override;

    weak_ptr<TileMotif> w_tileMotif;
};

////////////////////////////////////////////////////////////////////////////
//
// Irregular No Map Editor
//

class IrregularNoMapEditor : public  NamedMotifEditor
{
    Q_OBJECT

public:
    IrregularNoMapEditor(QString aname,DesignElementPtr del, bool doEmit);

protected:

};

#endif
