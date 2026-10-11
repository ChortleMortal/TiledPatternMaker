#pragma once

#include <QGroupBox>
#include <QTextEdit>
#include "gui/widgets/panel_misc.h"
#include "gui/panels/panel_page.h"
#include "gui/widgets/layout_sliderset.h"
#include "model/makers/tiling_maker.h"
#include "sys/enums/etilingmaker.h"

typedef std::shared_ptr<class Tiling>         TilingPtr;
typedef std::shared_ptr<class PlacedTile>     PlacedTilePtr;

typedef std::weak_ptr<class Tiling>           WeakTilingPtr;
typedef std::weak_ptr<class PlacedTile>       WeakPlacedTilePtr;

class EdgePoly;
class TilingMakerView;
class FloatableTab;
class SMXWidget;
class AQPushButton;

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

    void slot_refreshMenu(eTileMenuRefresh scope);
    void slot_viewModeChanged(int id);

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

    void slot_infoTableCellSelected(int row, int col);
    void slot_infoTableMenu(QPointF spt);
    void slot_infoTableHeaderClicked(int index);

    void slot_showExludesInTable(bool checked);
    void slot_fillOnLoad(bool checked);

    void slot_swapTrans();
    void slot_delete_clicked();
    void slot_convert_tile_regularity();
    void slot_uniquify_clicked();
    void slot_setModes(QAbstractButton *btn);
    void slot_setTranslationsMode(bool checked);
    void slot_set_reps(int val);
    void slot_table_menu_edit_tile();
    void slot_menu_includePlaced();
    void slot_exportPoly();
    void slot_importGirihPoly();
    void slot_addGirihShape();
    void slot_trim(qreal valX, qreal valY);

    void singleton_changed(bool checked);
    void slot_hideVectors(bool checked);
    void slot_mergeTilings();

    void slot_setDistortion();
    void slot_loadConstructionLines();
    void slot_setConstructionLinesMode(bool checked);

    void slot_setKbdMode1(QAbstractButton *btn, QButtonGroup *kbdGroup);
    void slot_kbdMode1(TilingMaker::eKeyboardMode mode);

protected:
    FloatableTab * createCreateTab();
    FloatableTab * createAdjustTab();
    FloatableTab * createRepeatTab();
    FloatableTab * createConstructionLinesTab();
    FloatableTab * createDetailsTab();

    AQTableWidget * createTilingTable();
    QWidget       * createDebugInfo();
    QGroupBox     * createModesGroup();
    QGroupBox     * createStackGroup();
    QGroupBox     * createConstructionLinesGroup();
    QGroupBox     * createTilingsGroup();
    QGroupBox     * createTileGroup();
    QGroupBox     * createActionsGroup();
    QGroupBox     * createKbdModesAdjustTab();
    QGroupBox     * createKbdModesCreateTab();
    QGroupBox     * createRepetitionsGroup();
    QGroupBox     * createDistortionGroup();

    QVBoxLayout   * createSecondControlRow();
    QVBoxLayout   * createTranslationsRow();
    QHBoxLayout   * createFillDataRow();
    QVBoxLayout   * createBackgroundInfo();

    void refresh(eTileMenuRefresh scope);
    void refreshMenuStatus();
    void tallyMouseMode();
    void tallyKbdMode();
    void enterConstructionLinesMode(bool checked);
    void initPageStatusString();
    void setViewState(TilingMakerView::ViewMode mode);

    QString getTileInfo(PlacedTilePtr pfp);

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

    WeakTilingPtr   currentTiling;

    QTabWidget    * tabWidget;
    FloatableTab  * controlTab;
    FloatableTab  * stateTab;
    FloatableTab  * repeatTab;
    FloatableTab  * clinesTab;
    FloatableTab  * detailsTab;

    QGroupBox     * tilingsGroup;
    QListWidget   * tilingList;

    QButtonGroup  * viewModeGroup;
    QButtonGroup  * mouseModeBtnGroup;
    QButtonGroup  * kbdBtnGroup1;
    QButtonGroup  * kbdBtnGroup2;
    SMXWidget     * smxwidget1;
    SMXWidget     * smxwidget2;
    SMXWidget     * smxwidgetCL;

    AQPushButton  * drawTransRepeat;   // duplicate of the "Set Translations" mode button, on the Repeat tab
    AQPushButton  * drawConstRepeat;   // duplicate of the "Construction Lines" mode button, on the C-Lines tab

    QCheckBox     * fillBtn;

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
    FullPrecisionDoubleSpinBox * tileRot;
    QComboBox      * girihShapes;

    QCheckBox       * chkSingle;
    AQComboBox      * distortCombo;
    QCheckBox       * chkShowCLines;
    QLabel          * clNotSaved;
    DoubleSpinSet   * distortX;
    DoubleSpinSet   * distortY;
    QCheckBox       * chkBkgd;
    QPushButton     * pbExam;

    QPushButton     * pbRender;
    QCheckBox       * chkPropagate;
    QCheckBox       * chkTileAnchorMode;

    QAbstractButton * lastChecked;
    QAbstractButton * lastChecked2;
};

class BQSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    BQSpinBox(page_tiling_maker * parent, PlacedTilePtr ptp);

    void setValue(int val);

    bool blocked;


    virtual void focusInEvent(QFocusEvent * event) override;
    virtual void focusOutEvent(QFocusEvent * event) override;
    virtual void wheelEvent(QWheelEvent * event) override;
             void mousePressEvent(QMouseEvent *event) override;
             bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    WeakPlacedTilePtr   tile;
    page_tiling_maker * parent;
};

class BQDoubleSpinBox : public FullPrecisionDoubleSpinBox
{
    Q_OBJECT

public:
    BQDoubleSpinBox(page_tiling_maker * parent, PlacedTilePtr ptp);

    void setValue(double val);

    bool blocked;

    virtual void focusInEvent(QFocusEvent * event) override;
    virtual void focusOutEvent(QFocusEvent * event) override;
    virtual void wheelEvent(QWheelEvent * event) override;
            void mousePressEvent(QMouseEvent *event) override;
            bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    WeakPlacedTilePtr   tile;
    page_tiling_maker * parent;
};
