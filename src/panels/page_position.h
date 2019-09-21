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

#ifndef PAGE_POSITION_H
#define PAGE_POSITION_H

#include "panel_page.h"
#include "layout_sliderset.h"

enum ppCols
{
    PP_LEFT,
    PP_TOP,
    PP_SCALE,
    PP_ROT,
    PP_LAYER_T,
    PP_CLEAR
};

class page_position : public panel_page
{
    Q_OBJECT

public:
    page_position(ControlPanel * cpanel);

    void refreshPage() override;
    void onEnter() override;

signals:
    void sig_separationAbs(qreal x, qreal y);
    void sig_offsetAbs(qreal x, qreal y);
    void sig_originAbs(int x, int y);

private slots:
    void    setScale(int radius);
    void    set_sep(qreal);
    void    set_off(qreal);
    void    set_start(int);

    void    slot_set_deltas(int row);
    void    slot_clear_deltas(int row);

protected:
    void    createDesignWidget();
    void    createLayerTable();

    void    updateDesignWidget();
    void    updateLayerTable();
    void    addLayerToTable(Layer * layer, int row);

private:
    QWidget      * designWidget;
    QTableWidget * layerTable;

    SliderSet   * xSliderSet;
    SliderSet   * ySliderSet;
    SliderSet   * scaleSliderSet;
    QSpinBox    * scaleSpin;
    QSpinBox    * xStart;
    QSpinBox    * yStart;

    QDoubleSpinBox * xSep;
    QDoubleSpinBox * ySep;
    QDoubleSpinBox * xOff;
    QDoubleSpinBox * yOff;

    QSignalMapper  leftMapper;
    QSignalMapper  topMapper;
    QSignalMapper  widthMapper;
    QSignalMapper  rotMapper;
    QSignalMapper  clearMapper;
};

#endif
