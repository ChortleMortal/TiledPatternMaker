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
#include "sliderset.h"

class page_position : public panel_page
{
    Q_OBJECT

public:
    page_position(ControlPanel * panel);

    void refreshPage() override;
    void onEnter() override;

    virtual void enterEvent(QEvent * event) Q_DECL_OVERRIDE;
    virtual void leaveEvent(QEvent * event) Q_DECL_OVERRIDE;

signals:
    void sig_separationAbs(qreal x, qreal y);
    void sig_offsetAbs(qreal x, qreal y);
    void sig_originAbs(int x, int y);

private slots:
    void    setScale(int radius);
    void    set_sep(qreal);
    void    set_off(qreal);
    void    set_start(int);

    void    slot_set_bounds(qreal);
    void    slot_set_deltas(qreal);
    void    slot_clear_deltas();

    void    slot_select(int);

protected:
    void        createDesignWidget();
    void        createStylesWidget();

    void        updateDesignWidget();
    void        updateStylesWidget();

    bool        myBlockUpdates;     // don't ask

private:
    QWidget     * designWidget;
    QWidget     * stylesWidget;

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

    DoubleSpinSet * bleft;
    DoubleSpinSet * btop;
    DoubleSpinSet * bwidth;
    DoubleSpinSet * btheta;

    DoubleSpinSet * dleft;
    DoubleSpinSet * dtop;
    DoubleSpinSet * dwidth;
    DoubleSpinSet * dtheta;

    DoubleSpinSet * aleft;
    DoubleSpinSet * atop;
    DoubleSpinSet * awidth;
    DoubleSpinSet * atheta;

    QLabel   * transLabel;
    QLabel   * layerDesc;
    QSpinBox * indexBox;
};

#endif
