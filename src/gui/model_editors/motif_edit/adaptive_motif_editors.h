#pragma once

#include "gui/model_editors/motif_edit/radial_motif_editors.h"
#include "model/motifs/adaptive_girih.h"
#include "model/motifs/adaptive_hourglass.h"
#include "model/motifs/adaptive_inference.h"
#include "model/motifs/adaptive_intersect.h"
#include "model/motifs/adaptive_rosette.h"
#include "model/motifs/adaptive_star.h"
#include "model/motifs/adaptive_tile.h"
#include "model/motifs/explicit_map_motif.h"

using std::shared_ptr;
using std::weak_ptr;

////////////////////////////////////////////////////////////////////////////
//
// ExplicitMapEditor
//
////////////////////////////////////////////////////////////////////////////
class ExplicitMapEditor : public  MotifEditor
{
public:
    ExplicitMapEditor(QString aname, shared_ptr<ExplicitMapMotif> explMap);

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


class GirihEditor : public MotifEditor
{
public:
    GirihEditor(QString aname, shared_ptr<AdaptiveGirih> girih);

private:
    void editorToMotif(bool doEmit) override;
    void refresh() override;

    DoubleSliderSet * skip;

    weak_ptr<AdaptiveGirih>   w_girih;
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

class HourglassEditor : public MotifEditor
{
public:
    HourglassEditor(QString aname,shared_ptr<AdaptiveHourglass> hour);

private:
    void editorToMotif(bool doEmit) override;
    void refresh() override;

    DoubleSliderSet  * d;
    SliderSet        * s;

    weak_ptr<AdaptiveHourglass>  w_hourglass;
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

class InferEditor : public  MotifEditor
{
    Q_OBJECT

public:
    InferEditor(QString aname, shared_ptr<AdaptiveInference> infer);

private:
    void editorToMotif(bool doEmit) override;

    weak_ptr<AdaptiveInference>   w_infer;
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

class IntersectEditor : public MotifEditor
{
    Q_OBJECT

public:
    IntersectEditor(QString aname, shared_ptr<AdaptiveIntersect> isect);

private:
    void editorToMotif(bool doEmit) override;
    void refresh() override;

    DoubleSliderSet        * skip;
    SliderSet              * s;
    QCheckBox              * progressive_box;

    weak_ptr<AdaptiveIntersect> w_isect;
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

class IrregularRosetteEditor : public MotifEditor
{
    Q_OBJECT

public:
    IrregularRosetteEditor(QString aname, shared_ptr<AdaptiveRosette> rose);

private:
    void editorToMotif(bool doEmit) override;
    void refresh() override;

    DoubleSliderSet	*	q_slider;
    SliderSet       *   s_slider;
    DoubleSliderSet	*	r_slider;
    DoubleSliderSet *   k_slider;

    weak_ptr<AdaptiveRosette>  w_rose;
};

////////////////////////////////////////////////////////////////////////////
//
// ExplicitStarEditor.java
//
// The controls for editing a Star.  Glue code, just like RosetteEditor.
////////////////////////////////////////////////////////////////////////////

class IrregularStarEditor : public MotifEditor
{
    Q_OBJECT

public:
    IrregularStarEditor(QString aname,shared_ptr<AdaptiveStar> star);

private:
    void editorToMotif(bool doEmit) override;
    void refresh() override;

    SliderSet       * s_slider;
    DoubleSliderSet * d_slider;

    weak_ptr<AdaptiveStar>   w_star;
};

///////////////////////////////////////////////////////////////////////////
//
// ExplicitTileEditor
//
///////////////////////////////////////////////////////////////////////////

class ExplicitTileEditor : public MotifEditor
{
    Q_OBJECT

public:
    ExplicitTileEditor(QString aname, shared_ptr<AdaptiveTile> tmotif);

private:
    void editorToMotif(bool doEmit) override;

    weak_ptr<AdaptiveTile> w_tileMotif;
};

////////////////////////////////////////////////////////////////////////////
//
// Irregular No Map Editor
//

class IrregularNoMapEditor : public  MotifEditor
{
public:
    IrregularNoMapEditor(QString aname,shared_ptr<IrregularNoMap> nomap);
};
