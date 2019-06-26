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

#ifndef PAGE_TILING_MAKER_H
#define PAGE_TILING_MAKER_H

#include "panel_page.h"
#include "panels/sliderset.h"

class page_tiling_maker : public panel_page
{
    Q_OBJECT

public:
    page_tiling_maker(ControlPanel * panel);

    void refreshPage() override;
    void onEnter() override;

signals:
    void sig_tilingChanged();

public slots:
    void currentFeature(int index);
    void slot_loadedXML(QString name);
    void slot_loadedTiling (QString name);

private slots:
    void slot_updateTiling();
    void slot_saveTiling();
    void slot_sidesChanged(int row);
    void slot_transformChanged(int row);
    void slot_t1t2Changed(double);
    void slot_nameChanged();
    void slot_authorChanged();
    void slot_descChanged();
    void slot_cellSelected(int row, int col);
    void slot_all_features(bool checked);
    void slot_all_overlaps(bool checked);
    void slot_clearWS();
    void slot_sourceSelect(int id);
    void slot_swapTrans();
    void slot_remove_clicked();
    void slot_setModes(int mode);
    void set_reps();
    void slot_hideTable(bool checked);

protected:
    AQWidget * createTilingDesignerControls();
    AQWidget * createTilingTable();
    void createTopGrid(QVBoxLayout *vbox);

    void clear();
    void displayTilingDesigner(TilingPtr tiling);
    void displayPlacedFeature(PlacedFeaturePtr pf, int row, QString from);

    PlacedFeaturePtr getFeatureRow(int row);
    TilingPtr        getSourceTiling();

private:
    class TilingDesigner * designer;
    TilingPtr       lastTiling;

    bool    hideTable;

    QRadioButton *radioLoadedStyleTileView;
    QRadioButton *radioWSTileView;
    QButtonGroup  tilingGroup3;

    QCheckBox * chk_autoFill;
    QCheckBox * chk_hideTable;

    QWidget      * makerSourceBox;

    QButtonGroup * mouseModeBtnGroup;

    QLineEdit   tile_name;
    QTextEdit   tile_desc;
    QLineEdit   tile_author;

    DoubleSpinSet * t1x;
    DoubleSpinSet * t1y;
    DoubleSpinSet * t2x;
    DoubleSpinSet * t2y;

    QPushButton pbSave;

    QLabel       * debugLabel;
    QTextEdit   pointLabel;

    QTableWidget * tileInfoTable;

    SpinSet    * xRepMin;
    SpinSet    * xRepMax;
    SpinSet    * yRepMin;
    SpinSet    * yRepMax;

    QSignalMapper  sidesMapper;
    QSignalMapper  scaleMapper;
    QSignalMapper  rotMapper;
    QSignalMapper  xMapper;
    QSignalMapper  yMapper;

    QSpinBox * sides;
};

#endif
