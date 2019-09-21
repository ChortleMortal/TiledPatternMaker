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
#include "panels/layout_sliderset.h"
#include "panels/layout_transform.h"
#include "makers/tilingmaker.h"

enum epageTi
{
    TI_INDEX,
    TI_SIDES,
    TI_SCALE,
    TI_ROT,
    TI_X,
    TI_Y,
    TI_REGULAR,
    TI_CW,
    TI_FEAT_ADDR,
    TI_LOCATION
};

#undef LAYER_XFORM_INFO

class page_tiling_maker : public panel_page
{
    Q_OBJECT

public:
    page_tiling_maker(ControlPanel * panel);

    void refreshPage() override;
    void onEnter() override;
    void onExit() override;

signals:
    void sig_tilingChanged();
    void sig_loadTiling(QString name);

public slots:
    void currentFeature(int index);
    void slot_loadedXML(QString name);
    void slot_loadedTiling (QString name);

private slots:
    void refreshMenu();
    void slot_updateTiling();
    void slot_saveTiling();
    void slot_sidesChanged(int row);
    void slot_transformChanged(int row);
    void slot_t1t2Changed(double val);
    void slot_nameChanged();
    void slot_authorChanged();
    void slot_descChanged();
    void slot_cellSelected(int row, int col);
    void slot_all_features(bool checked);
    void slot_clearWS();
    void slot_sourceSelect(int id);
    void slot_swapTrans();
    void slot_remove_clicked();
    void slot_setModes(int mode);
    void set_reps();
    void slot_hideTable(bool checked);
    void slot_reloadTiling();
    void slot_loadBackground();
    void slot_bkgdImageChanged();
    void slot_bkgdTransformChanged();
    void slot_adjustBackground();
    void slot_saveAdjustedBackground();
    void slot_menu(QPointF spt);
    void slot_menu_edit_feature();
    void slot_menu_includePlaced();
    void slot_exportPoly();
    void slot_importPoly();
    void slot_addGirihShape();

    void slot_kbdXform(bool checked);
    void slot_moveX(int amount);
    void slot_moveY(int amount);
    void slot_rotate(int amount);
    void slot_scale(int amount);

    void tableHeaderClicked(int index);
    void slot_trim(qreal valX, qreal valY);

protected:
    AQWidget * createTiliingMakerControls();
    AQWidget * createTilingTable();
    void       createTopGrid();

    void clear();
    void displayPlacedFeatureInTable(PlacedFeaturePtr pf, int row, QString from);
    void updateFeaturePointInfo(PlacedFeaturePtr pfp);

    void useBackground(TilingPtr tiling);

    PlacedFeaturePtr getFeatureRow(int row);
    TilingPtr        getSourceTiling();

private:
    TilingMaker * tilingMaker;

    TilingPtr       lastTiling;

    bool    hideTable;

    QRadioButton *radioLoadedStyleTileView;
    QRadioButton *radioWSTileView;
    QButtonGroup  tilingGroup3;

    QCheckBox * chk_autoFill;
    QCheckBox * chk_hideTable;
    QCheckBox * chk_showOverlaps;

    QWidget      * makerSourceBox;

    QButtonGroup * mouseModeBtnGroup;

    QLineEdit   tile_name;
    QTextEdit   tile_desc;
    QLineEdit   tile_author;
    QTextEdit   featureInfo;

    DoubleSpinSet * t1x;
    DoubleSpinSet * t1y;
    DoubleSpinSet * t2x;
    DoubleSpinSet * t2y;

    QLabel       * debugLabel;

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

    QSpinBox      * sides;
    QComboBox     * girihShapes;

    LayoutTransform bkgdLayout;
#ifdef LAYER_XFORM_INFO
    LayoutTransform layerXform;
    LayoutTransform layerDeltas;
#endif
    QCheckBox     * showBkgd;
    QCheckBox     * hideTiling;
    QCheckBox     * perspectiveBkgd;
    QCheckBox     * transformBkgd;
    QCheckBox     * kbdXform;

    QGroupBox     * bkgdGroup;
};

#endif
