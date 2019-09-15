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

#ifndef FIGURE_EDITORS_H
#define FIGURE_EDITORS_H

#include <QtWidgets>
#include "base/shared.h"
#include "panels/layout_sliderset.h"
#include "panels/panel_misc.h"

class FigureMaker;

// An abstract class for containing the controls related to the editing
// of one kind of figure.  A complex hierarchy of FigureEditors gets built
// up to become the changeable controls for editing figures in FigureMaker.
class FigureEditor : public AQWidget
{
    Q_OBJECT

public:
    FigureEditor(FigureMaker *  editor, QString name);

    virtual FigurePtr getFigure() = 0;
    virtual void resetWithFigure(FigurePtr figure) = 0;

    void    addLayout(QBoxLayout * layout) { vbox->addLayout(layout);}
    void    addWidget(QWidget    * widget) { vbox->addWidget(widget);}

signals:
    void sig_figure_changed();

public slots:
    virtual void updateGeometry() = 0;

protected:
    virtual void updateLimits() = 0;

    FigureMaker	* editor;
    AQVBoxLayout    * vbox;

    DoubleSliderSet	* figureScale;
    DoubleSliderSet	* boundaryScale;
    SliderSet       * boundarySides;

    QString         name;
};

class RadialEditor : public FigureEditor
{
    Q_OBJECT

public:
    RadialEditor(FigureMaker * editor, QString name);

    virtual FigurePtr getFigure() = 0;
    virtual void resetWithFigure( FigurePtr figure ) = 0;

public slots:
    virtual void updateGeometry() = 0;

protected:
    virtual void updateLimits()   = 0;

    SliderSet       *   n;
    DoubleSliderSet *   radial_r;

private:
};

class StarEditor : public RadialEditor
{
    Q_OBJECT

public:
    StarEditor(FigureMaker * editor, QString name);

    virtual FigurePtr getFigure();
    virtual void      resetWithFigure(FigurePtr figure);

public slots:
    virtual void updateGeometry();

protected:
    void    setStar(StarPtr star) { this->star = star; }

    virtual void updateLimits();

    DoubleSliderSet	*	d;
    SliderSet		*	s;

private:
    StarPtr			star;
};


class ConnectStarEditor : public StarEditor
{
    Q_OBJECT

public:
    ConnectStarEditor(FigureMaker * editor, QString name );

    FigurePtr getFigure();
    void      resetWithFigure( FigurePtr figure );

public slots:
    void updateGeometry();

protected:
    void calcScale();
    void updateLimits();

    QPushButton     *   defaultBtn;

private:
    StarConnectPtr starConnect;
};

class ExtendedStarEditor : public StarEditor
{
    Q_OBJECT

public:
    ExtendedStarEditor(FigureMaker * editor, QString name );

    FigurePtr getFigure();
    void resetWithFigure( FigurePtr figure );

public slots:
    void updateGeometry();

protected:
    void updateLimits();

private:
    ExtStarPtr      extended;

    QCheckBox       * extendBox1;
    QCheckBox       * extendBox2;

};

class RosetteEditor : public RadialEditor
{
    Q_OBJECT

public:
    RosetteEditor(FigureMaker * editor, QString name);

    virtual FigurePtr getFigure();
    virtual void resetWithFigure(FigurePtr figure);

public slots:
    virtual void updateGeometry();

protected:
    virtual void updateLimits();

    DoubleSliderSet	*	q;
    DoubleSliderSet	*	k;
    SliderSet       *   s;

private:
    RosettePtr rosette;
};

class ConnectRosetteEditor : public RosetteEditor
{
    Q_OBJECT

public:
    ConnectRosetteEditor(FigureMaker * editor, QString name);

    FigurePtr getFigure();
    void      resetWithFigure( FigurePtr figure );

protected:
    void    updateGeometry();
    void    calcScale();

    QPushButton * defaultBtn;

private:
    void updateLimits();

    RosetteConnectPtr rosetteConnect;
};

class ExtendedRosetteEditor : public RosetteEditor
{
    Q_OBJECT

public:
    ExtendedRosetteEditor(FigureMaker * editor, QString name);

    FigurePtr getFigure();
    void resetWithFigure( FigurePtr figure );

public slots:
    void updateGeometry();

protected:
    void updateLimits();

private:
    ExtRosettePtr   extended;

    QCheckBox       * extendPeriphBox;
    QCheckBox       * extendFreeBox;
    QCheckBox       * connectBoundaryBox;
};

#endif

