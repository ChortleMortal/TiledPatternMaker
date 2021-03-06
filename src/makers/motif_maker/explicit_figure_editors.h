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

#ifndef EXPLICIT_FIGURE_EDITORS_H
#define EXPLICIT_FIGURE_EDITORS_H

#include <QtWidgets>
#include "figure_editors.h"
#include "enums/efigtype.h"

typedef std::shared_ptr<class ExplicitFigure>   ExplicitPtr;

class ExplicitEditor : public  FigureEditor
{
    Q_OBJECT

public:
    ExplicitEditor(page_motif_maker * ed, QString aname);

    virtual FigurePtr getFigure() override;

    virtual void resetWithFigure(FigurePtr fig, bool dcEmit) override;

protected:
    ExplicitPtr resetFigure(FigurePtr fig, eFigType figType);

    ExplicitPtr explicitFig;
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

    FigurePtr  getFigure() override;
    void resetWithFigure(FigurePtr fig, bool doEmit) override;

private:
    void updateFigure(bool doEmit) override;
    void updateEditor() override;

    ExplicitPtr       girihFig;
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
    ExplicitPtr        hourglassFig;
    DoubleSliderSet  * d;
    SliderSet        * s;

public:
    ExplicitHourglassEditor(page_motif_maker * ed, QString aname);

    FigurePtr getFigure() override;
    void resetWithFigure(FigurePtr fig, bool doEmit) override;

private:
    void updateEditor() override;
    void updateFigure(bool doEmit) override;
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

    FigurePtr getFigure() override;

    void resetWithFigure(FigurePtr fig, bool doEmit) override;

private:
    void updateFigure(bool doEmit) override;

    ExplicitPtr   explicitInferFig;
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

    FigurePtr getFigure() override;
    void resetWithFigure(FigurePtr fi, bool doEmit) override;

private:
    void updateFigure(bool doEmit) override;
    void updateEditor() override;

    ExplicitPtr              intersect;

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

    FigurePtr getFigure() override;

    void resetWithFigure(FigurePtr fig, bool doEmit) override;

private:
    void updateEditor() override;
    void updateFigure(bool doEmit) override;

    DoubleSliderSet	*	q_slider;
    SliderSet       *   s_slider;
    DoubleSliderSet	*	r_slider;

    ExplicitPtr expRoseFig;
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

    FigurePtr getFigure() override;

    void resetWithFigure(FigurePtr fig, bool doEmit) override;

protected:
    void updateFigure(bool doEmit) override;
    void updateEditor() override;

private:
    ExplicitPtr     expStarFig;

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

    FigurePtr getFigure() override;
    void resetWithFigure(FigurePtr fig, bool doEmit) override;

private:
    void updateFigure(bool doEmit) override;

    ExplicitPtr featFig;
};
#endif

