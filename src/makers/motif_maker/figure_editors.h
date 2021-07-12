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
#include "panels/panel_misc.h"

class page_motif_maker;
class SliderSet;
class DoubleSliderSet;
class AQWidget;

typedef std::shared_ptr<class Figure>           FigurePtr;
typedef std::shared_ptr<class Star>             StarPtr;
typedef std::shared_ptr<class Rosette>          RosettePtr;
typedef std::shared_ptr<class StarConnectFigure>    StarConnectPtr;
typedef std::shared_ptr<class RosetteConnectFigure> RosetteConnectPtr;
typedef std::shared_ptr<class ExtendedStar>     ExtStarPtr;
typedef std::shared_ptr<class ExtendedRosette>  ExtRosettePtr;

// An abstract class for containing the controls related to the editing
// of one kind of figure.  A complex hierarchy of FigureEditors gets built
// up to become the changeable controls for editing figures in FigureMaker.
class FigureEditor : public AQWidget
{
    Q_OBJECT

public:
    FigureEditor(page_motif_maker * fm, QString figname);

    virtual FigurePtr getFigure()  { return figure; }
    virtual void      resetWithFigure(FigurePtr fig, bool doEmit);

    void    addLayout(QBoxLayout * layout) { vbox->addLayout(layout);}
    void    addWidget(QWidget    * widget) { vbox->addWidget(widget);}

signals:
    void sig_figure_changed(FigurePtr fig);

protected:
    virtual void updateFigure(bool doEmit);
    virtual void updateEditor();

    FigurePtr       figure;
    QString         name;

    page_motif_maker * menu;
    AQVBoxLayout    * vbox;

    DoubleSliderSet	* figureScale;
    DoubleSliderSet	* boundaryScale;
    SliderSet       * boundarySides;
    DoubleSliderSet * figureRotate;
};

class StarEditor : public FigureEditor
{
    Q_OBJECT

public:
    StarEditor(page_motif_maker * fm, QString figname);

    virtual FigurePtr getFigure() override;
    virtual void      resetWithFigure(FigurePtr fig, bool doEmit) override;

protected:
    virtual void updateFigure(bool doEmit) override;
    virtual void updateEditor() override;

    SliderSet       *   n_slider;
    DoubleSliderSet	*	d_slider;
    SliderSet		*	s_slider;

private:
    StarPtr			star;
};


class RosetteEditor : public FigureEditor
{
    Q_OBJECT

public:
    RosetteEditor(page_motif_maker *fm, QString figname);

    virtual FigurePtr getFigure() override;
    virtual void      resetWithFigure(FigurePtr fig, bool doEmit) override;

protected:
    virtual void updateFigure(bool doEmit) override;
    virtual void updateEditor() override;

    SliderSet       *   n_slider;
    DoubleSliderSet	*	q_slider;
    DoubleSliderSet	*	k_slider;
    SliderSet       *   s_slider;

private:
    RosettePtr rosette;
};

class ConnectStarEditor : public StarEditor
{
    Q_OBJECT

public:
    ConnectStarEditor(page_motif_maker * fm, QString figname);

    FigurePtr getFigure() override;
    virtual void resetWithFigure(FigurePtr fig, bool doEmit) override;

public slots:

protected:
    void calcScale();

    QPushButton     *   defaultBtn;

private:
    StarConnectPtr starConnect;
};

class ConnectRosetteEditor : public RosetteEditor
{
    Q_OBJECT

public:
    ConnectRosetteEditor(page_motif_maker *fm, QString figname);

    FigurePtr getFigure() override;
    virtual void  resetWithFigure(FigurePtr fig, bool doEmit) override;

protected:
    void    calcScale();

    QPushButton * defaultBtn;

private:
    RosetteConnectPtr rosetteConnect;
};

class ExtendedStarEditor : public StarEditor
{
    Q_OBJECT

public:
    ExtendedStarEditor(page_motif_maker * fm, QString figname);

    FigurePtr getFigure() override;
    virtual void resetWithFigure(FigurePtr fig, bool doEmit) override;

protected:
    virtual void updateFigure(bool doEmit) override;
    virtual void updateEditor() override;

private:
    ExtStarPtr      extended;

    QCheckBox       * extendBox1;
    QCheckBox       * extendBox2;

};

class ExtendedRosetteEditor : public RosetteEditor
{
    Q_OBJECT

public:
    ExtendedRosetteEditor(page_motif_maker *fm, QString figname);

    FigurePtr getFigure() override;
    virtual void resetWithFigure(FigurePtr fig, bool doEmit) override;

protected:
    virtual void updateFigure(bool doEmit) override;
    virtual void updateEditor() override;

private:
    ExtRosettePtr   extended;

    QCheckBox       * extendPeriphBox;
    QCheckBox       * extendFreeBox;
    QCheckBox       * connectBoundaryBox;
};

#endif

