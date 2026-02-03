#ifndef PAGE_TILING_MAKER_H
#define PAGE_TILING_MAKER_H

#include "gui/panels/panel_misc.h"
#include "gui/panels/panel_page.h"
#include "gui/widgets/layout_sliderset.h"
#include "model/makers/tiling_maker.h"
#include "sys/enums/etilingmaker.h"

typedef std::shared_ptr<class Tiling>         TilingPtr;
typedef std::shared_ptr<class PlacedTile>     PlacedTilePtr;

typedef std::weak_ptr<class Tiling>           wTilingPtr;
typedef std::weak_ptr<class PlacedTile>       WeakPlacedTilePtr;

class EdgePoly;
class TilingMakerView;
class FloatableTab;
class SMXWidget;

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
    ~page_tiling_maker();

    void onRefresh()        override;
    void onEnter()          override;
    void onExit()           override;
    bool canExit()          override;

private slots:
    void slot_detach(int index);

    void slot_clearTiling();
    void slot_reloadTiling();
    void slot_duplicateTiling();

    void slot_refreshMenu(eTileMenuRefresh scop = TMR_ALL);

    void slot_currentItemChanged(QListWidgetItem * current, QListWidgetItem * prev);
    void slot_itemChanged(QListWidgetItem * item);
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

    void slot_showExludes(bool checked);
    void slot_showDebug(bool checked);
    void slot_autofill(bool checked);
    void slot_showTranslations(bool checked);

    void slot_swapTrans();
    void slot_delete_clicked();
    void slot_convert_tile_regularity();
    void slot_uniquify_clicked();
    void slot_setModes(QAbstractButton *btn);
    void slot_set_reps(int val);
    void slot_menu(QPointF spt);
    void slot_table_menu_edit_tile();
    void slot_menu_includePlaced();
    void slot_exportPoly();
    void slot_importGirihPoly();
    void slot_addGirihShape();

    void tableHeaderClicked(int index);
    void slot_trim(qreal valX, qreal valY);

    void singleton_changed(bool checked);
    void reps_changed(bool checked);
    void slot_hideVectors(bool checked);
    void slot_mergeTilings();

    void slot_setKbdMode1(QAbstractButton *btn, QButtonGroup *kbdGroup);
    void slot_kbdMode1(eTMKbdMode mode);

protected:
    FloatableTab * createControlTab();
    FloatableTab * createStateTab();

    AQTableWidget * createTilingTable();
    QWidget       * createDebugInfo();
    QGroupBox     * createModesGroup();
    QGroupBox     * createActionsGroup();
    QGroupBox     * createTilingsGroup();
    QGroupBox     * createKbdModes(QButtonGroup *kbdGroup, SMXWidget *smxwidget);
    QGroupBox     * createRepetitionsGroup();

    QHBoxLayout   * createSecondControlRow();
    QVBoxLayout   * createTranslationsRow();
    QHBoxLayout   * createFillDataRow();
    QVBoxLayout   * createBackgroundInfo();

    void refresh(eTileMenuRefresh scope);
    void refreshMenuStatus();
    void tallyMouseMode();
    void tallyKbdMode();
    void initPageStatusString();

    QString getTileInfo(PlacedTilePtr pfp);

    void          selectTileColumn(PlacedTilePtr pfp);
    PlacedTilePtr getTileColumn(int col);
    int           getColumn(PlacedTilePtr pfp);

private:
    void __refreshTiling();
    void __refreshTilingSelector();
    void __refreshTilingHeader();
    void __refreshTilingTable();
    void __refreshMenuStatus();
    void __refreshOther(bool clear);
    void __buildTilingTable();

    void buildTableEntry(PlacedTilePtr pf, int col, QString inclusion);
    void refreshTableEntry(PlacedTilePtr pf, int col, QString inclusion);

    wTilingPtr      currentTiling;

    QTabWidget    * tabWidget;
    FloatableTab  * controlTab;
    FloatableTab  * stateTab;

    QGroupBox     * tilingsGroup;
    QListWidget   * tilingList;

    QButtonGroup  * mouseModeBtnGroup;
    QButtonGroup  * kbdBtnGroup1;
    QButtonGroup  * kbdBtnGroup2;
    SMXWidget     * smxwidget1;
    SMXWidget     * smxwidget2;

    QCheckBox     * fillVectorsChk;

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
    QLabel       * stackLabel;

    AQSpinBox    * xRepMin;
    AQSpinBox    * xRepMax;
    AQSpinBox    * yRepMin;
    AQSpinBox    * yRepMax;

    AQSpinBox      * sides;
    AQDoubleSpinBox* tileRot;
    QComboBox      * girihShapes;

    QRadioButton    * chkSingle;
    QRadioButton    * chkReps;
    QCheckBox       * chkShowDebug;
    QCheckBox       * chkBkgd;
    QPushButton     * pbExam;

    QPushButton     * pbRender;
    QCheckBox       * chkPropagate;

    QAbstractButton * lastChecked;
};

class BQSpinBox : public AQSpinBox
{
public:
    BQSpinBox(page_tiling_maker * parent, PlacedTilePtr ptp);

    virtual void  enterEvent(QEnterEvent *event) override;

protected:
    WeakPlacedTilePtr   tile;
    page_tiling_maker * parent;
};

class BQDoubleSpinBox : public AQDoubleSpinBox
{
public:
    BQDoubleSpinBox(page_tiling_maker * parent, PlacedTilePtr ptp);

    virtual void  enterEvent(QEnterEvent *event) override;

protected:
    WeakPlacedTilePtr   tile;
    page_tiling_maker * parent;
};

#endif
