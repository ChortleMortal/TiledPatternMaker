#ifndef EXPLICIT_FIGURE_EDITORS_H
#define EXPLICIT_FIGURE_EDITORS_H

#include "enums/efigtype.h"
#include "makers/motif_maker/figure_editors.h"

typedef std::shared_ptr<class ExplicitFigure>   ExplicitPtr;
typedef std::weak_ptr<class ExplicitFigure>     WeakExplicitPtr;

class ExplicitEditor : public  FigureEditor
{
    Q_OBJECT

public:
    ExplicitEditor(page_motif_maker * ed, QString aname);

    virtual void setFigure(DesignElementPtr del, bool dcEmit) override;

protected:
    ExplicitPtr resetFigure(DesignElementPtr del, eFigType figType);

    WeakExplicitPtr explicitFig;
};

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


class ExplicitGirihEditor : public ExplicitEditor
{
public:
    ExplicitGirihEditor(page_motif_maker *ed, QString aname);

    void setFigure(DesignElementPtr del, bool doEmit) override;

private:
    void editorToFigure(bool doEmit) override;
    void figureToEditor() override;

    WeakExplicitPtr   girihFig;

    SliderSet       * side;
    DoubleSliderSet * skip;
};

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

class ExplicitHourglassEditor : public ExplicitEditor
{
private:
    WeakExplicitPtr    hourglassFig;

    DoubleSliderSet  * d;
    SliderSet        * s;

public:
    ExplicitHourglassEditor(page_motif_maker * ed, QString aname);

    void setFigure(DesignElementPtr del, bool doEmit) override;

private:
    void figureToEditor() override;
    void editorToFigure(bool doEmit) override;
};

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

class ExplicitInferEditor : public  ExplicitEditor
{
    Q_OBJECT

public:
    ExplicitInferEditor(page_motif_maker *ed, QString aname);

    void setFigure(DesignElementPtr del, bool doEmit) override;

private:
    void editorToFigure(bool doEmit) override;

    WeakExplicitPtr   explicitInferFig;
};

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

class ExplicitIntersectEditor : public ExplicitEditor
{
    Q_OBJECT

public:
    ExplicitIntersectEditor(page_motif_maker * ed, QString aname);

    void setFigure(DesignElementPtr del, bool doEmit) override;

private:
    void editorToFigure(bool doEmit) override;
    void figureToEditor() override;

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
// The editing controls for explicit figures.  A simple class, because
// (right now) explicit figures don't have any editing controls.  All
// you can do is ask the figure to be inferred from the rest of the
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

    void setFigure(DesignElementPtr del, bool doEmit) override;

private:
    void figureToEditor() override;
    void editorToFigure(bool doEmit) override;

    DoubleSliderSet	*	q_slider;
    SliderSet       *   s_slider;
    DoubleSliderSet	*	r_slider;

    WeakExplicitPtr     expRoseFig;
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

    void setFigure(DesignElementPtr del, bool doEmit) override;

protected:
    void editorToFigure(bool doEmit) override;
    void figureToEditor() override;

private:
    WeakExplicitPtr   expStarFig;

    SliderSet       * s_slider;
    DoubleSliderSet * d_slider;
};


///////////////////////////////////////////////////////////////////////////
//
// ExplicitFeatureEditor
//
///////////////////////////////////////////////////////////////////////////

class ExplicitFeatureEditor : public ExplicitEditor
{
    Q_OBJECT

public:
    ExplicitFeatureEditor(page_motif_maker *ed, QString aname);

    void setFigure(DesignElementPtr del, bool doEmit) override;

private:
    void editorToFigure(bool doEmit) override;

    WeakExplicitPtr featFig;
};
#endif

