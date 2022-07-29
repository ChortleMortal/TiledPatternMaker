#ifndef PAGE_TILING_MAKER_H
#define PAGE_TILING_MAKER_H

#include "widgets/panel_page.h"
#include "widgets/panel_misc.h"
#include "enums/estatemachineevent.h"

typedef std::shared_ptr<class BackgroundImage>  BkgdImgPtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class PlacedFeature>    PlacedFeaturePtr;
typedef std::weak_ptr<class   PlacedFeature>    WeakPlacedFeaturePtr;
typedef std::weak_ptr<class   Feature>          WeakFeaturePtr;

class QGroupBox;
class QTextEdit;
class QCheckBox;

class EdgePoly;
class DoubleSpinSet;
class AQSpinBox;
class AQDoubleSpinBox;

enum epageTi
{
    TI_LOCATION,
    TI_SHOW,
    TI_TYPE_PFP,    // regular, poly, or girih
    TI_FEAT_SIDES,
    TI_FEAT_ROT,
    TI_FEAT_SCALE,
    TI_SCALE,
    TI_ROT,
    TI_X,
    TI_Y,
    TI_CW,
    TI_FEAT_ADDR
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

    void slot_buildMenu();
    void slot_refreshMenuData();
    void slot_currentFeature(int index);

    void slot_currentTilingChanged(int index);
    void slot_sidesChanged(int col);
    void slot_f_rotChanged(int col);
    void slot_f_scaleChanged(int col);
    void slot_transformChanged(int col);
    void slot_showFeatureChanged(int col);
    void slot_t1t2Changed(double val);
    void slot_t1t2LenChanged(double val);
    void slot_t1t2AngleChanged(double val);

    void slot_cellSelected(int row, int col);
    void slot_showTable(bool checked);
    void slot_all_features(bool checked);
    void slot_showDebug(bool checked);
    void slot_autofill(bool checked);
    void slot_swapTrans();
    void slot_delete_clicked();
    void slot_make_irregular_clicked();
    void slot_uniquify_clicked();
    void slot_setModes(QAbstractButton *btn);
    void slot_set_reps(int val);
    void slot_menu(QPointF spt);
    void slot_menu_edit_feature();
    void slot_menu_includePlaced();
    void slot_exportPoly();
    void slot_importPoly();
    void slot_addGirihShape();

    void tableHeaderClicked(int index);
    void slot_trim(qreal valX, qreal valY);

    void singleton_changed(bool checked);

protected:
    AQTableWidget * createTilingTable();
    AQWidget      * createDebugInfo();
    QGroupBox     * createModesGroup();
    QGroupBox     * createActionsGroup();
    QHBoxLayout   * createControlRow();
    QHBoxLayout   * createTableControlRow();
    AQWidget      * createTranslationsRow();
    QHBoxLayout   * createFillDataRow();

    void setup();
    void refreshMenuData();
    void buildTableEntry(PlacedFeaturePtr pf, int col, QString inclusion);
    void refreshTableEntry(PlacedFeaturePtr pf, int col, QString inclusion);

    void loadTilingCombo(TilingPtr selected);
    void tallyMouseMode();

    void updateFeaturePointInfo(PlacedFeaturePtr pfp);
    void pushTilingToMotifMaker(eSM_Event event);

    PlacedFeaturePtr getFeatureColumn(int col);

private:
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

    AQWidget      * translationsWidget;
    AQTableWidget * tileInfoTable;
    AQWidget      * debugWidget;

    QTextEdit    * featureInfo;
    QLabel       * statusLabel;
    QLabel       * debugLabel1;
    QLabel       * debugLabel2;
    QLabel       * overlapStatus;

    AQSpinBox    * xRepMin;
    AQSpinBox    * xRepMax;
    AQSpinBox    * yRepMin;
    AQSpinBox    * yRepMax;

    AQSpinBox      * sides;
    AQDoubleSpinBox* featRot;
    QComboBox      * girihShapes;

    QCheckBox     * chk_hideTiling;
    QCheckBox     * chkSingle;
};

#endif
