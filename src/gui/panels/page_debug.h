#pragma once
#ifndef PAGE_DEBUG_H
#define PAGE_DEBUG_H

class QGroupBox;
class AQPushButton;
class AQTableWidget;
class QDoubleSpinBox;
class QTabWidget;
class FloatableTab;

#include <QMap>
#include "gui/panels/panel_page.h"
#include "sys/geometry/debug_map.h"
#include "sys/qt/qtapplog.h"

typedef std::shared_ptr<class Tiling>  TilingPtr;
typedef std::shared_ptr<class Mosaic>  MosaicPtr;

enum eFlagCol
{
    FLAG_CREATE_INACTIVE = 0,
    FLAG_CREATE_ACTIVE   = 1,
    FLAG_PAINT_INACTIVE  = 2,
    FLAG_PAINT_ACTIVE    = 3
};

class page_debug : public panel_page
{
    Q_OBJECT

public:
    page_debug(ControlPanel * cpanel);
    ~page_debug();

    void    onRefresh() override;
    void    onEnter() override;
    void    onExit() override;

    void    examineAllMosaics();

signals:
    void    sig_testA();
    void    sig_testB();

public slots:

private slots:
    void    slot_detach(int index);

    void    slot_verifyProtosClicked(bool enb);
    void    slot_verifyMapsClicked(bool enb);
    void    slot_verifyDumpClicked(bool enb);
    void    slot_verifypopupClicked(bool enb);
    void    slot_verifyVerboseClicked(bool enb);
    void    slot_unDupMerges(bool enb);
    void    slot_buildEmptyNMaps(bool enb);

    void    slot_dontTrapLog(bool dont);
    void    slot_dontRefresh(bool enb);

    void    slot_dbgViewClicked(bool checked);
    void    slot_dbgFlagsClicked(bool checked);
    void    slot_edgeSelectClicked(bool checked);
    void    slot_triggerClicked(bool checked);
    void    slot_viewViewCen(bool checked);

    void    slot_useProtoMap(bool checked);
    void    slot_measure(bool checked);

    void    slot_refreshFlags();
    void    slot_flagPressed(int row,int col);
    void    slot_clearFlags();

    void    slot_startPicker(bool checked);

protected:
    void    examineMosaic();
    void    examineMosaicXML();
    void    reformatMosaicXML();
    void    reformatOldTemplates();
    void    reformatTilingXML();
    void    reprocessMosaicXML();
    void    reprocessTilingXML();
    void    testA();
    void    testB();
    void    verifyAllTilings();
    void    verifyTiling();
    void    verifyTilingNames();
    void    colorPicker();

    bool    verifyTiling(TilingPtr tiling);
    void    identifyDuplicateTiles(TilingPtr tiling);
    void    examineMosaic(MosaicPtr mosaic, uint test);

    QGroupBox    * createDebugTests();
    QGroupBox    * createDebugSettings();
    QGroupBox    * createVerifyMaps();
    QGroupBox    * creatDebugMaps();
    QWidget      * creatDebugFlags();
    QGroupBox    * createMeasure();
    QGroupBox    * createCleanse();

    QComboBox    * debugTests;

    AQPushButton * pbEnbDbgView;
    AQPushButton * pbEnbDbgView2;
    AQPushButton * pbEnbDbgFlags;
    AQPushButton * pbTrigger;
    QGridLayout  * mapGrid;

    QDoubleSpinBox * xBox;
    QDoubleSpinBox * yBox;

private:
    qtAppLog     * log;
    QTimer       * timer;

    QTabWidget   * tabWidget;
    FloatableTab * tab1;
    FloatableTab * tab2;
    FloatableTab * tab3;

    AQTableWidget * flagTable;
    QLabel       * mapCreateCounts[ROW_SIZE];
    QLabel       * mapPaintCounts[ROW_SIZE];

    bool           pick;
    QLabel       * colorTxt;
};

#endif
