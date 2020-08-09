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
#include "makers/tiling_maker/tiling_maker.h"

enum epageTi
{
    TI_TYPE_PFP,    // regular, poly, or girih
    TI_FEAT_SIDES,
    TI_FEAT_ROT,
    TI_SCALE,
    TI_ROT,
    TI_X,
    TI_Y,
    TI_CW,
    TI_FEAT_ADDR,
    TI_LOCATION
};

class page_tiling_maker : public panel_page
{
    Q_OBJECT

public:
    page_tiling_maker(ControlPanel * panel);

    void refreshPage() override;
    void onEnter() override;
    void onExit() override;
    bool canExit() override;

    void buildMenu();
    void refreshMenuData();

signals:
    void sig_tilingChanged();
    void sig_loadTiling(QString name);
    void sig_reload();

public slots:
    void slot_currentFeature(int index);
    void slot_loadedXML(QString name);
    void slot_loadedTiling (QString name);
    void slot_buildMenu();
    void slot_refreshMenu();
    void slot_unload();

private slots:
    void slot_reloadTiling();
    void slot_replaceTilingInStyles();

    void slot_sidesChanged(int col);
    void slot_f_rotChanged(int col);
    void slot_transformChanged(int col);
    void slot_t1t2Changed(double val);

    void slot_cellSelected(int row, int col);
    void slot_hideTable(bool checked);
    void slot_all_features(bool checked);
    void slot_showDebug(bool checked);
    void slot_autofill(bool checked);
    void slot_clearWS();
    void slot_clearTiling();
    void slot_swapTrans();
    void slot_remove_clicked();
    void slot_uniquify_clicked();
    void slot_setModes(int mode);
    void slot_menu(QPointF spt);
    void slot_menu_edit_feature();
    void slot_menu_includePlaced();
    void slot_exportPoly();
    void slot_importPoly();
    void slot_addGirihShape();

    void slot_moveX(int amount);
    void slot_moveY(int amount);
    void slot_rotate(int amount);
    void slot_scale(int amount);

    void tableHeaderClicked(int index);
    void slot_trim(qreal valX, qreal valY);

    void slot_loadBackground();
    void slot_adjustBackground();
    void slot_saveAdjustedBackground();
    void slot_setBkgdXform();
    void slot_setBkgd();

protected:

    AQTableWidget * createTilingTable();
    AQWidget      * createDebugInfo();
    QGroupBox     * createBackgroundGroup();
    QGroupBox     * createModesGroup();
    QGroupBox     * createActionsGroup();
    QHBoxLayout   * createControlRow();
    QHBoxLayout   * createTableControlRow();
    AQWidget      * createTranslationsRow();

    void clear();
    void buildTableEntry(PlacedFeaturePtr pf, int col, QString inclusion);
    void refreshTableEntry(PlacedFeaturePtr pf, int col, QString inclusion);
    void updateFeaturePointInfo(PlacedFeaturePtr pfp);

    void displayBackgroundStatus(TilingPtr tiling);

    PlacedFeaturePtr getFeatureColumn(int col);

private:
    TilingMaker * tilingMaker;

    QLabel    * currentTiling;

    QCheckBox * chk_autoFill;
    QCheckBox * chk_hideTable;
    QCheckBox * chk_showDebug;
    QCheckBox * chk_showOverlaps;

    QButtonGroup * mouseModeBtnGroup;

    DoubleSpinSet * t1x;
    DoubleSpinSet * t1y;
    DoubleSpinSet * t2x;
    DoubleSpinSet * t2y;

    AQWidget      * translationsWidget;
    AQTableWidget * tileInfoTable;
    AQWidget      * debugWidget;

    QTextEdit    * featureInfo;
    QLabel       * debugLabel1;
    QLabel       * debugLabel2;


    QSignalMapper  f_sidesMapper;
    QSignalMapper  f_rotMapper;
    QSignalMapper  scaleMapper;
    QSignalMapper  rotMapper;
    QSignalMapper  xMapper;
    QSignalMapper  yMapper;

    QSpinBox      * sides;
    QDoubleSpinBox* featRot;
    QComboBox     * girihShapes;

    LayoutTransform bkgdLayout;

    QCheckBox     * chk_showBkgd;
    QCheckBox     * chk_hideTiling;
    QCheckBox     * chk_adjustBkgd;
    QCheckBox     * chk_xformBkgd;

    QGroupBox     * bkgdGroup;

};

#endif
