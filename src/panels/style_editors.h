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

#ifndef STYLED_EDITORS_H
#define STYLED_EDITORS_H

#include "panels/layout_sliderset.h"
#include "panels/panel_misc.h"
#include "style/Colored.h"
#include "style/Plain.h"
#include "style/Emboss.h"
#include "style/Filled.h"
#include "style/Interlace.h"
#include "style/Sketch.h"
#include "style/Thick.h"
#include "style/TileColors.h"

class WorkspaceViewer;

class StyleEditor : public QObject
{
    Q_OBJECT

public:
    StyleEditor();
    StylePtr          style;

signals:
    void    sig_viewWS();
    void    sig_colorsChanged();
    void    sig_update();

protected:
    WorkspaceViewer * viewer;

};


// Editor for the colored style: allow choosing the color.
class ColoredEditor : public StyleEditor
{
    Q_OBJECT

public:
    ColoredEditor(Colored * c, QTableWidget * table);

public slots:
    void slot_colorsChanged();

private slots:
    void slot_transparencyChanged(qreal val);
    void slot_pickColor();

protected:
    int             rows;

private:
    void updateTransparency();

    Colored         * colored;
    QTableWidget    * table;
    AQWidget        * colorwidget;
    QPushButton     * color_button;
    DoubleSliderSet * transparency;
};

// Editor for the thick style.
class ThickEditor : public ColoredEditor
{
    Q_OBJECT

public:
    ThickEditor(Thick * o, QTableWidget * table);


 private slots:
    void  slot_widthChanged(int width);
    void  slot_outlineChanged(int state);

private:
    Thick     * thick;
    SliderSet * width_slider;
    QCheckBox * outline_checkbox;
};

class StyleColorFillSet;
class StyleColorFillGroup;

// Editor for the filled style.
class FilledEditor : public StyleEditor
{
    Q_OBJECT

public:
    FilledEditor(Filled * f, QTableWidget * table, QVBoxLayout *parmsCtrl );
    ~FilledEditor();

    Filled * getFilled() { return filled; }

public slots:
    void slot_colorsChanged();

private slots:
    void slot_insideChanged(int state);
    void slot_outsideChanged(int state);
    void slot_editB();
    void slot_editW();
    void slot_algo(int index);
    void slot_viewFaces();
    void slot_setSelect(int face);

protected:
    void        displayParms();
    void        displayParms01();
    void        displayParms2();
    void        displayParms3();

private:
    QTableWidget * table;
    Filled       * filled;
    QCheckBox    * inside_checkbox;
    QCheckBox    * outside_checkbox;
    QVBoxLayout  * vbox;

    StyleColorFillSet   * fillSet;
    StyleColorFillGroup * fillGroup;
};


// Editor for the embossed style.
class EmbossEditor : public ThickEditor
{
    Q_OBJECT

public:
    EmbossEditor(Emboss * e, QTableWidget * table);

public slots:
    void slot_anlgeChanged(int angle);

private:
    Emboss      * emboss;
    SliderSet   * angle_slider;
};

// Editor for teh interlaced style.
class InterlaceEditor : public ThickEditor
{
    Q_OBJECT

public:
    InterlaceEditor(Interlace * i, QTableWidget * table);

private slots :
    void slot_gapChanged(qreal gap);
    void slot_shadowChanged(qreal shadow);
    void slot_includeTipVerticesChanged(int state);

private:
    Interlace * interlace;

    DoubleSliderSet * gap_slider;
    DoubleSliderSet * shadow_slider;
    QCheckBox       * tipVert_checkbox;
};

// Editor for the colored style: allow choosing the color.
class TileColorsEditor : public StyleEditor
{
    Q_OBJECT

public:
    TileColorsEditor(TileColors * c, QTableWidget * table);

public slots:

private slots:

protected:

private:
    TileColors * colored;
};
#endif
