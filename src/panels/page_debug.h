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

#ifndef PAGE_DEBUG_H
#define PAGE_DEBUG_H

#include "panel_page.h"

class DoubleSpinSet;

class page_debug : public panel_page
{
    Q_OBJECT

public:
    page_debug(ControlPanel * cpanel);

    void refreshPage() override;
    void onEnter() override;
    void onExit() override;

signals:
    void    sig_cycle();
    void    sig_compareImageFiles(QString,QString);
    void    sig_view_image(QString file);

public slots:
    void    slot_compareResult(QString result);
    void    slot_setImage0(QString name);
    void    slot_setImage1(QString name);

private slots:
    void    slot_verifyTilingNames();
    void    slot_reformatDesignXML();
    void    slot_reformatTilingXML();
    void    slot_reprocessDesignXML();
    void    slot_reprocessTilingXML();

    void    slot_autoCycleClicked(bool enb);
    void    slot_stopIfDiffClicked(bool enb);
    void    slot_gridModelClicked(bool enb);
    void    slot_cycleModeChanged(int row);
    void    slot_cycleIntervalChanged(int value);
    void    slot_gridWidthChanged(qreal value);
    void    selectDir0();
    void    selectDir1();
    void    swapDirs();

    void    slot_startCycle();
    void    slot_selectImage0();
    void    slot_viewImage0();
    void    slot_selectImage1();
    void    slot_viewImage1();
    void    slot_compareImages();
    void    slot_transparentClicked(bool checked);
    void    slot_differencesClicked(bool checked);
    void    slot_ping_pongClicked(bool checked);
    void    slot_side_by_sideClicked(bool checked);

protected:
    void  ViewImage(QString file);

private:
    QComboBox   * cycleCombo;

    QLineEdit   * comp0;
    QLineEdit   * comp1;

    QLineEdit   * imageName0;
    QLineEdit   * imageName1;
    QLineEdit   * imageCompareResult;
    QPushButton * selectImage0;
    QPushButton * viewImage0;
    QPushButton * selectImage1;
    QPushButton * viewImage1;
    QPushButton * compareImage;

    QCheckBox   * transparent;
    QCheckBox   * differences;
    QCheckBox   * ping_pong;
    QCheckBox   * side_by_side;

    QCheckBox   * cbGridModel;
    DoubleSpinSet * gridWidth;
};

#endif
