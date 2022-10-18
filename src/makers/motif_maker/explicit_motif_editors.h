#ifndef EXPLICIT_MOTIF_EDITORS_H
#define EXPLICIT_MOTIF_EDITORS_H

#include "enums/emotiftype.h"
#include "makers/motif_maker/regular_motif_editors.h"

typedef std::shared_ptr<class ExplicitMotif>   ExplicitPtr;
typedef std::weak_ptr<class ExplicitMotif>     WeakExplicitPtr;

class ExplicitEditor : public  NamedMotifEditor
{
    Q_OBJECT

public:
    ExplicitEditor(page_motif_maker * ed, QString aname);

    virtual void setMotif(DesignElementPtr del, bool dcEmit) override;

protected:
    ExplicitPtr resetMotif(DesignElementPtr del, eMotifType figType);

    WeakExplicitPtr    explicitMotif;
    class MotifMaker * motifMaker;
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


class ExplicitGirihEditor : public ExplicitEditor
{
public:
    ExplicitGirihEditor(page_motif_maker *ed, QString aname);

    void setMotif(DesignElementPtr del, bool doEmit) override;

private:
    void editorToMotif(bool doEmit) override;
    void motifToEditor() override;

    WeakExplicitPtr   girih;

    SliderSet       * side;
    DoubleSliderSet * skip;
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

class ExplicitHourglassEditor : public ExplicitEditor
{
private:
    WeakExplicitPtr    hourglass;

    DoubleSliderSet  * d;
    SliderSet        * s;

public:
    ExplicitHourglassEditor(page_motif_maker * ed, QString aname);

    void setMotif(DesignElementPtr del, bool doEmit) override;

private:
    void motifToEditor() override;
    void editorToMotif(bool doEmit) override;
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

class ExplicitInferEditor : public  ExplicitEditor
{
    Q_OBJECT

public:
    ExplicitInferEditor(page_motif_maker *ed, QString aname);

    void setMotif(DesignElementPtr del, bool doEmit) override;

private:
    void editorToMotif(bool doEmit) override;

    WeakExplicitPtr   explicitInfer;
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

class ExplicitIntersectEditor : public ExplicitEditor
{
    Q_OBJECT

public:
    ExplicitIntersectEditor(page_motif_maker * ed, QString aname);

    void setMotif(DesignElementPtr del, bool doEmit) override;

private:
    void editorToMotif(bool doEmit) override;
    void motifToEditor() override;

    WeakExplicitPtr          intersect;

    SliderSet              * side;
    DoubleSliderSet        * skip;
    SliderSet              * s;
    QCheckBox              * progressive_box;
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

class ExplicitRosetteEditor : public ExplicitEditor
{
    Q_OBJECT

public:
    ExplicitRosetteEditor(page_motif_maker *ed, QString aname);

    void setMotif(DesignElementPtr del, bool doEmit) override;

private:
    void motifToEditor() override;
    void editorToMotif(bool doEmit) override;

    DoubleSliderSet	*	q_slider;
    SliderSet       *   s_slider;
    DoubleSliderSet	*	r_slider;

    WeakExplicitPtr     expRose;
};

////////////////////////////////////////////////////////////////////////////
//
// ExplicitStarEditor.java
//
// The controls for editing a Star.  Glue code, just like RosetteEditor.

class ExplicitStarEditor : public ExplicitEditor
{
    Q_OBJECT

public:
    ExplicitStarEditor(page_motif_maker * ed, QString aname);

    void setMotif(DesignElementPtr del, bool doEmit) override;

protected:
    void editorToMotif(bool doEmit) override;
    void motifToEditor() override;

private:
    WeakExplicitPtr   expStar;

    SliderSet       * s_slider;
    DoubleSliderSet * d_slider;
};


///////////////////////////////////////////////////////////////////////////
//
// ExplicitTileEditor
//
///////////////////////////////////////////////////////////////////////////

class ExplicitTileEditor : public ExplicitEditor
{
    Q_OBJECT

public:
    ExplicitTileEditor(page_motif_maker *ed, QString aname);

    void setMotif(DesignElementPtr del, bool doEmit) override;

private:
    void editorToMotif(bool doEmit) override;

    WeakExplicitPtr tileMotif;
};
#endif

