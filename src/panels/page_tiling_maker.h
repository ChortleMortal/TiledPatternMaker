#ifndef PAGE_TILING_MAKER_H
#define PAGE_TILING_MAKER_H

#include "widgets/panel_page.h"
#include "widgets/panel_misc.h"

typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class PlacedTile>       PlacedTilePtr;
typedef std::weak_ptr<class   PlacedTile>       WeakPlacedTilePtr;

class EdgePoly;
class DoubleSpinSet;
class AQSpinBox;
class AQDoubleSpinBox;

class TilingMakerView;

enum epageTi
{
    TI_LOCATION,
    TI_SHOW,
    TI_TYPE_PFP,    // regular, poly, or girih
    TI_TILE_ROT,
    TI_TILE_SCALE,
    TI_TILE_SIDES,
    TI_PLACEMENT_SCALE,
    TI_PLACEMENT_ROT,
    TI_PLACEMENT_X,
    TI_PLACEMENT_Y,
    TI_INFO_CW,
    TI_TILE_ADDR
};

class page_tiling_maker : public panel_page
{
    Q_OBJECT

public:
    page_tiling_maker(ControlPanel * panel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override;
    bool canExit() override;

    void buildMenu();

private slots:
    void slot_clearTiling();
    void slot_reloadTiling();
    void slot_undo();

    void slot_buildMenu();
    void slot_refreshMenuData();
    void slot_currentTile(PlacedTilePtr pfp);

    void slot_currentTilingChanged(int index);
    void slot_sidesChanged(int col);
    void slot_tileRotChanged(int col);
    void slot_tileScaleChanged(int col);
    void slot_placedTranslateChanged(int col);
    void slot_placedScaleChanged(int col);
    void slot_placedRotateChanged(int col);
    void slot_showTileChanged(int col);
    void slot_t1t2Changed(double val);
    void slot_t1t2LenChanged(double val);
    void slot_t1t2AngleChanged(double val);

    void slot_cellSelected(int row, int col);
    void slot_showTable(bool checked);
    void slot_all_tiles(bool checked);
    void slot_showDebug(bool checked);
    void slot_autofill(bool checked);
    void slot_swapTrans();
    void slot_delete_clicked();
    void slot_convert_tile_regularity();
    void slot_uniquify_clicked();
    void slot_setModes(QAbstractButton *btn);
    void slot_set_reps(int val);
    void slot_menu(QPointF spt);
    void slot_menu_edit_tile();
    void slot_menu_includePlaced();
    void slot_exportPoly();
    void slot_importPoly();
    void slot_addGirihShape();

    void tableHeaderClicked(int index);
    void slot_trim(qreal valX, qreal valY);

    void singleton_changed(bool checked);

protected:
    AQTableWidget * createTilingTable();
    QWidget       * createDebugInfo();
    QGroupBox     * createModesGroup();
    QGroupBox     * createActionsGroup();
    QHBoxLayout   * createControlRow();
    QHBoxLayout   * createTableControlRow();
    QWidget       * createTranslationsRow();
    QHBoxLayout   * createFillDataRow();

    void setup();
    void refreshMenuData();
    void buildTableEntry(PlacedTilePtr pf, int col, QString inclusion);
    void refreshTableEntry(PlacedTilePtr pf, int col, QString inclusion);

    void loadTilingCombo(TilingPtr selected);
    void tallyMouseMode();

    void updateTilePointInfo(PlacedTilePtr pfp);

    PlacedTilePtr getTileColumn(int col);
    int              getColumn(PlacedTilePtr pfp);

private:
    TilingMakerView * tmView;

    QComboBox     * tilingCombo;

    QButtonGroup  * mouseModeBtnGroup;

    DoubleSpinSet * t1x;
    DoubleSpinSet * t1y;
    DoubleSpinSet * t2x;
    DoubleSpinSet * t2y;

    DoubleSpinSet * T1len;
    DoubleSpinSet * T2len;
    DoubleSpinSet * T1angle;
    DoubleSpinSet * T2angle;

    QWidget       * translationsWidget;
    AQTableWidget * tileInfoTable;
    QWidget       * debugWidget;

    QTextEdit    * tileInfo;
    QLabel       * statusLabel;
    QLabel       * debugLabel1;
    QLabel       * debugLabel2;
    QLabel       * overlapStatus;
    QLabel       * undoStatus;
    QLabel       * loadedLabel;

    AQSpinBox    * xRepMin;
    AQSpinBox    * xRepMax;
    AQSpinBox    * yRepMin;
    AQSpinBox    * yRepMax;

    AQSpinBox      * sides;
    AQDoubleSpinBox* tileRot;
    QComboBox      * girihShapes;

    QCheckBox     * chk_hideTiling;
    QCheckBox     * chkSingle;
    QCheckBox     * chk_showDebug;
};

#endif
