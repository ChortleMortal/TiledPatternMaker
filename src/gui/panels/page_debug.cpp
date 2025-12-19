#include <QGroupBox>
#include <QCheckBox>
#include <QMessageBox>

#include "gui/map_editor/map_editor.h"
#include "gui/panels/page_debug.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view_controller.h"
#include "gui/viewers/debug_view.h"
#include "gui/widgets/dlg_textedit.h"
#include "gui/widgets/floatable_tab.h"
#include "gui/widgets/layout_sliderset.h"
#include "gui/widgets/transparent_widget.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/prototype_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_manager.h"
#include "model/mosaics/mosaic_reader.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/styles/filled.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "model/tilings/tiling_manager.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map_verifier.h"
#include "sys/qt/qtapplog.h"
#include "sys/sys/debugflags.h"
#include "sys/sys/fileservices.h"

using std::make_shared;

typedef std::shared_ptr<class Filled>       FilledPtr;

page_debug:: page_debug(ControlPanel * cpanel)  : panel_page(cpanel,PAGE_DEBUG_TOOLS,"Debug Tools")
{
    log      = qtAppLog::getInstance();
    pick     = false;

    // tab1
    QGroupBox   * debugTests    = createDebugTests();
    QGroupBox   * debugSettings = createDebugSettings();
    QGroupBox   * mapvVerify    = createVerifyMaps();
    QGroupBox   * debugMaps     = creatDebugMaps();
    QGroupBox   * meas          = createMeasure();
    QGroupBox   * clns          = createCleanse();

    // tab 2
    QWidget     * dbgFlags = creatDebugFlags();

    QVBoxLayout * vbox1 = new QVBoxLayout;
    QVBoxLayout * vbox2 = new QVBoxLayout;
    QVBoxLayout * vbox3 = new QVBoxLayout;

    vbox1->addWidget(debugSettings);
    vbox1->addStretch();
    vbox1->addWidget(clns);

    vbox2->addWidget(mapvVerify);
    vbox2->addStretch();
    vbox2->addWidget(meas);

    vbox3->addWidget(debugMaps);
    vbox3->addStretch();

    QHBoxLayout * hbox   = new QHBoxLayout();
    hbox->addLayout(vbox1);
    hbox->addLayout(vbox2);
    hbox->addLayout(vbox3);

    QVBoxLayout * layout1 = new QVBoxLayout();
    layout1->addLayout(hbox);
    layout1->addStretch();

    tab1 = new FloatableTab();
    tab1->setLayout(layout1);

    QVBoxLayout * layout2 = new QVBoxLayout();
    layout2->addWidget(dbgFlags);
    layout2->addStretch();

    tab2 = new FloatableTab();
    tab2->setLayout(layout2);

    QVBoxLayout * layout3 = new QVBoxLayout();
    layout3->addWidget(debugTests);
    layout3->addStretch();

    tab3 = new FloatableTab();
    tab3->setLayout(layout3);

    tabWidget = new QTabWidget;
    tabWidget->addTab(tab1, "Debug");
    tabWidget->addTab(tab2, "Debug Flags");
    tabWidget->addTab(tab3, "Debug Tests");
    tabWidget->setCurrentIndex(Sys::config->debugTabIndex);

    vbox->addWidget(tabWidget);
    vbox->addStretch();

    connect(tabWidget, &QTabWidget::currentChanged, this, [](uint index) {Sys::config->debugTabIndex = index; });
    connect(tabWidget, &QTabWidget::tabBarDoubleClicked, this, &page_debug::slot_detach);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &page_debug::slot_refreshFlags);

    QSettings s;

    QString name = QString("panel2/%1/float_tab1").arg(pageName);
    bool tofloat = s.value(name,false).toBool();
    if (tofloat)
        slot_detach(0);

    name = QString("panel2/%1/float_tab2").arg(pageName);
    tofloat = s.value(name,false).toBool();
    if (tofloat)
        slot_detach(1);

    name = QString("panel2/%1/float_tab3").arg(pageName);
    tofloat = s.value(name,false).toBool();
    if (tofloat)
        slot_detach(2);
}

page_debug::~page_debug()
{
    QSettings s;

    QString name = QString("panel2/%1/float_tab1").arg(pageName);
    s.setValue(name,tab1->floating);
    tab1->floating = false;

    name = QString("panel2/%1/float_tab2").arg(pageName);
    s.setValue(name,tab2->floating);
    tab2->floating = false;

    name = QString("panel2/%1/float_tab3").arg(pageName);
    s.setValue(name,tab3->floating);
    tab3->floating = false;
}

void page_debug::slot_detach(int index)
{
    QWidget * widget = tabWidget->widget(index);
    QString title    = tabWidget->tabText(index);

    tabWidget->removeTab(index);

    FloatableTab * floater = dynamic_cast<FloatableTab*>(widget);
    Q_ASSERT(floater);
    floater->detach(tabWidget,title);
}

QGroupBox * page_debug::createDebugSettings()
{
    QCheckBox   * chkSuspendPaint       = new QCheckBox("Don't paint");
    QCheckBox   * chkDontRefresh        = new QCheckBox("Don't Refresh Menus");
    QCheckBox   * chkDontTrap           = new QCheckBox("Don't Trap Log");
    QCheckBox   * chkLayerCen           = new QCheckBox("Show Layer Centre");
    QCheckBox   * chkEnbLog2            = new QCheckBox("Enable LOG2");

    chkEnbLog2->setChecked(Sys::config->enableLog2);
    chkDontRefresh->setChecked(!Sys::updatePanel);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(chkSuspendPaint);
    vbox->addWidget(chkDontRefresh);
    vbox->addWidget(chkDontTrap);
    vbox->addWidget(chkLayerCen);
    vbox->addWidget(chkEnbLog2);

    QGroupBox * debugGroup = new QGroupBox("Debug Settings");
    debugGroup->setLayout(vbox);

    connect(chkSuspendPaint,          &QCheckBox::clicked,       this,   [](bool enb) { Sys::viewController->debugSuspendPaint(enb); } );
    connect(chkDontTrap,              &QCheckBox::clicked,       this,   &page_debug::slot_dontTrapLog);
    connect(chkDontRefresh,           &QCheckBox::clicked,       this,   &page_debug::slot_dontRefresh);
    connect(chkLayerCen,              &QCheckBox::clicked,       this,   &page_debug::slot_viewViewCen);
    connect(chkEnbLog2,               &QCheckBox::clicked,       this,   [](bool enb) { Sys::config->enableLog2 = enb; } );
    connect(chkEnbLog2,               &QCheckBox::clicked,       this,   [](bool enb) { Sys::config->enableLog2 = enb; } );

    return debugGroup;
}

QGroupBox * page_debug::createDebugTests()
{
    AQPushButton* pTestA                = new AQPushButton("Test A");
    AQPushButton* pTestB                = new AQPushButton("Test B");

    QPushButton * pbReformatDesXMLBtn   = new QPushButton("Reformat Mosaics");
    QPushButton * pbReformatTileXMLBtn  = new QPushButton("Reformat Tilings");
    QPushButton * pbReprocessDesXMLBtn  = new QPushButton("Reprocess Mosaics");
    QPushButton * pbReprocessTileXMLBtn = new QPushButton("Reprocess Tilings");
    QPushButton * pbReformatTemplates   = new QPushButton("Reformat Old Templates");

    QPushButton * pbExamineAllMosaics   = new QPushButton("Examine All Mosaics");
    QPushButton * pbExamineMosaic       = new QPushButton("Examine Current Mosaic");
    QPushButton * pbExamineMosaicXML    = new QPushButton("Examine Worklist Mosaic XML");
    QPushButton * pbVerifyAllTilings    = new QPushButton("Verify All Tilings");
    QPushButton * pbVerifyTiling        = new QPushButton("Verify Current Tiling");
    QPushButton * pbVerifyTileNames     = new QPushButton("Verify Tile Names");

    QPushButton * pbClearMakers         = new QPushButton("Clear Makers");
    QPushButton * pbClearView           = new QPushButton("Clear View");

    AQPushButton* pbPick                = new AQPushButton("Color Picker");
                  colorTxt              = new QLabel();

    QGridLayout * grid = new QGridLayout();
    grid->setHorizontalSpacing(11);

    // TILINGS
    grid->addWidget(pbReformatTileXMLBtn,  0,3);
    grid->addWidget(pbReprocessTileXMLBtn, 1,3);
    grid->addWidget(pbVerifyAllTilings,    2,3);
    grid->addWidget(pbVerifyTiling,        3,3);


    // MOSAICS
    grid->addWidget(pbExamineMosaicXML,    0,2);
    grid->addWidget(pbReformatDesXMLBtn,   1,2);
    grid->addWidget(pbReprocessDesXMLBtn,  2,2);
    grid->addWidget(pbExamineAllMosaics,   3,2);
    grid->addWidget(pbExamineMosaic,       4,2);
    grid->addWidget(pbVerifyTileNames,     5,2);

    // MISC
    grid->addWidget(pbClearMakers,         0,1);
    grid->addWidget(pbClearView,           1,1);
    grid->addWidget(pbReformatTemplates,   2,1);

    // GENERIC
    grid->addWidget(pTestA,                0,0);
    grid->addWidget(pTestB,                1,0);

    // MISC
    grid->addWidget(pbPick,                5,0);
    grid->addWidget(colorTxt,              5,1);

    QGroupBox * debugGroup = new QGroupBox("Debug");
    debugGroup->setLayout(grid);

    connect(pbReformatDesXMLBtn,      &QPushButton::clicked,     this,   [this] { reformatMosaicXML(); });
    connect(pbReprocessDesXMLBtn,     &QPushButton::clicked,     this,   [this] { reprocessMosaicXML(); });
    connect(pbExamineAllMosaics,      &QPushButton::clicked,     this,   [this] { examineAllMosaics(); });
    connect(pbExamineMosaicXML,       &QPushButton::clicked,     this,   [this] { examineMosaicXML(); });
    connect(pbExamineMosaic,          &QPushButton::clicked,     this,   [this] { examineMosaic(); });

    connect(pbReformatTileXMLBtn,     &QPushButton::clicked,     this,   [this] { reformatTilingXML(); });
    connect(pbReprocessTileXMLBtn,    &QPushButton::clicked,     this,   [this] { reprocessTilingXML(); });
    connect(pbReformatTemplates,      &QPushButton::clicked,     this,   [this] { reformatOldTemplates(); });

    connect(pbVerifyTileNames,        &QPushButton::clicked,     this,   [this] { verifyTilingNames(); });
    connect(pbVerifyTiling,           &QPushButton::clicked,     this,   [this] { verifyTiling(); });
    connect(pbVerifyAllTilings,       &QPushButton::clicked,     this,   [this] { verifyAllTilings(); });

    connect(pTestA,                   &AQPushButton::clicked,    this,   [this] { testA(); });
    connect(pTestB,                   &AQPushButton::clicked,    this,   [this] { testB(); });

    connect(pbPick,                   &AQPushButton::clicked,    this,   [this](bool checked) { slot_startPicker(checked); });

    connect(pbClearMakers,            &QPushButton::clicked,     viewControl, [] { Sys::viewController->slot_unloadAll(); });
    connect(pbClearView,              &QPushButton::clicked,     viewControl, [] { Sys::viewController->slot_unloadView(); });

    return debugGroup;
}

QGroupBox * page_debug::createVerifyMaps()
{
    QCheckBox * cbVerifyMaps    = new QCheckBox("Enable Map Verify");
    QCheckBox * cbForceVerifyProtos  = new QCheckBox("Force Verify Protos");

    QCheckBox * cbPopupErrors   = new QCheckBox("Show Map Errors");
    QCheckBox * cbVerifyDump    = new QCheckBox("Dump Maps");
    QCheckBox * cbVerifyVerbose = new QCheckBox("Verbose");

    QCheckBox * cbBuildEmptyNM  = new QCheckBox("Build Neighbours to Verify");

    QVBoxLayout * vbox = new QVBoxLayout;

    vbox->addWidget(cbVerifyMaps);
    vbox->addWidget(cbForceVerifyProtos);
    vbox->addWidget(cbVerifyVerbose);

    vbox->addWidget(cbPopupErrors);
    vbox->addWidget(cbBuildEmptyNM);
    vbox->addWidget(cbVerifyDump);

    QGroupBox * gbVerifyMaps = new QGroupBox("Map Verification");
    gbVerifyMaps->setLayout(vbox);

    cbForceVerifyProtos->setChecked(config->forceVerifyProtos);
    cbVerifyMaps->setChecked(config->verifyMaps);

    cbPopupErrors->setChecked(config->verifyPopup);
    cbVerifyDump->setChecked(config->verifyDump);
    cbVerifyVerbose->setChecked(config->verifyVerbose);
    cbBuildEmptyNM->setChecked(config->buildEmptyNmaps);

    connect(cbForceVerifyProtos, &QCheckBox::clicked, this, &page_debug::slot_verifyProtosClicked);
    connect(cbVerifyMaps,   &QCheckBox::clicked,    this,   &page_debug::slot_verifyMapsClicked);
    connect(cbPopupErrors,  &QCheckBox::clicked,    this,   &page_debug::slot_verifypopupClicked);
    connect(cbVerifyDump,   &QCheckBox::clicked,    this,   &page_debug::slot_verifyDumpClicked);
    connect(cbVerifyVerbose,&QCheckBox::clicked,    this,   &page_debug::slot_verifyVerboseClicked);
    connect(cbBuildEmptyNM, &QCheckBox::clicked,    this,   &page_debug::slot_buildEmptyNMaps);

    return gbVerifyMaps;
}

QGroupBox * page_debug::createMeasure()
{
    AQPushButton * pbMeasure = new AQPushButton("Measure");

    connect(pbMeasure, &AQPushButton::clicked, this, &page_debug::slot_measure);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(pbMeasure);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addStretch();

    QGroupBox * gb= new QGroupBox("Measure");
    gb->setLayout(vbox);

    return gb;
}


QGroupBox * page_debug::createCleanse()
{
    QCheckBox * cbCleanseMerges = new QCheckBox("Cleanse Merges (slow)");
    cbCleanseMerges->setChecked(config->slowCleanseMapMerges);

    connect(cbCleanseMerges,&QCheckBox::clicked,    this,   &page_debug::slot_unDupMerges);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(cbCleanseMerges);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addStretch();

    QGroupBox * gb= new QGroupBox("Cleanse");
    gb->setLayout(vbox);

    return gb;
}

QGroupBox * page_debug::creatDebugMaps()
{
    mapGrid = new QGridLayout;

    QCheckBox * chkLines     = new QCheckBox("Lines");
    QCheckBox * chkDBCdirn   = new QCheckBox("Direction");
    QCheckBox * chkArcCen    = new QCheckBox("Arc Centres");
    QCheckBox * chkPoints    = new QCheckBox("Points");
    QCheckBox * chkMarks     = new QCheckBox("Marks");
    QCheckBox * chkCircles   = new QCheckBox("Circles");
    QCheckBox * chkCurves    = new QCheckBox("Curves");

    pbEnbDbgView              = new AQPushButton("Enable View");
    AQPushButton * pbUseProto = new AQPushButton("Use Proto Map");
    QPushButton  * pbClear    = new QPushButton("Clear Map");
    QLabel       * lMap1      = new QLabel("Create map");
    QLabel       * lMap2      = new QLabel("Paint map");

    for (int i=ROW_TOP; i < ROW_BOT; i++)
    {
        mapCreateCounts[i] = new QLabel;
        mapPaintCounts[i] = new QLabel;
    }

    mapGrid->addWidget(pbEnbDbgView,ROW_TOP,D_COL_CHK);
    mapGrid->addWidget(lMap1,       ROW_TOP,D_COL_CREATE);
    mapGrid->addWidget(lMap2,       ROW_TOP,D_COL_PAINT);

    mapGrid->addWidget(pbClear,     ROW_BOT,D_COL_CREATE);
    mapGrid->addWidget(pbUseProto,  ROW_BOT,D_COL_PAINT);

    mapGrid->addWidget(chkMarks,    ROW_MARKS,      D_COL_CHK);
    mapGrid->addWidget(chkPoints,   ROW_PTS,        D_COL_CHK);
    mapGrid->addWidget(chkCircles,  ROW_CIRCS,      D_COL_CHK);
    mapGrid->addWidget(chkLines,    ROW_LINES,      D_COL_CHK);
    mapGrid->addWidget(chkDBCdirn,  ROW_DIRN,       D_COL_CHK);
    mapGrid->addWidget(chkCurves,   ROW_CURVES,     D_COL_CHK);
    mapGrid->addWidget(chkArcCen,   ROW_ARC_CEN,    D_COL_CHK);

    mapGrid->addWidget(mapCreateCounts[ROW_MARKS],   ROW_MARKS,     D_COL_CREATE,   Qt::AlignHCenter);
    mapGrid->addWidget(mapCreateCounts[ROW_PTS],     ROW_PTS,       D_COL_CREATE,   Qt::AlignHCenter);
    mapGrid->addWidget(mapCreateCounts[ROW_CIRCS],   ROW_CIRCS,     D_COL_CREATE,   Qt::AlignHCenter);
    mapGrid->addWidget(mapCreateCounts[ROW_LINES],   ROW_LINES,     D_COL_CREATE,   Qt::AlignHCenter);
    mapGrid->addWidget(mapCreateCounts[ROW_DIRN],    ROW_DIRN,      D_COL_CREATE,   Qt::AlignHCenter);
    mapGrid->addWidget(mapCreateCounts[ROW_CURVES],  ROW_CURVES,    D_COL_CREATE,  Qt::AlignHCenter);
    mapGrid->addWidget(mapCreateCounts[ROW_ARC_CEN], ROW_ARC_CEN,   D_COL_CREATE,  Qt::AlignHCenter);

    mapGrid->addWidget(mapPaintCounts[ROW_MARKS],       ROW_MARKS,  D_COL_PAINT,  Qt::AlignHCenter);
    mapGrid->addWidget(mapPaintCounts[ROW_PTS],         ROW_PTS,    D_COL_PAINT,  Qt::AlignHCenter);
    mapGrid->addWidget(mapPaintCounts[ROW_CIRCS],       ROW_CIRCS,  D_COL_PAINT,  Qt::AlignHCenter);
    mapGrid->addWidget(mapPaintCounts[ROW_LINES],       ROW_LINES,  D_COL_PAINT,  Qt::AlignHCenter);
    mapGrid->addWidget(mapPaintCounts[ROW_DIRN],        ROW_DIRN,   D_COL_PAINT,  Qt::AlignHCenter);
    mapGrid->addWidget(mapPaintCounts[ROW_CURVES],      ROW_CURVES, D_COL_PAINT,  Qt::AlignHCenter);
    mapGrid->addWidget(mapPaintCounts[ROW_ARC_CEN],     ROW_ARC_CEN,D_COL_PAINT,  Qt::AlignHCenter);

    pbEnbDbgView->setChecked(viewControl->isEnabled(VIEW_DEBUG));
    auto dview = Sys::debugView;
    chkLines->setChecked(dview->getShowLines());
    chkDBCdirn->setChecked(dview->getShowDirection());
    chkArcCen->setChecked(dview->getShowArcCentres());
    chkPoints->setChecked(dview->getShowPoints());
    chkMarks->setChecked(dview->getShowMarks());
    chkCircles->setChecked(dview->getShowCircles());
    chkCurves->setChecked(dview->getShowCurves());

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(mapGrid);
    vbox->addStretch();

    QGroupBox * groupBox = new QGroupBox("Debug Map");
    groupBox->setLayout(vbox);

    connect(chkLines,       &QCheckBox::clicked, this, [this](bool checked) { Sys::debugView->showLines(checked);       emit sig_updateView();} );
    connect(chkDBCdirn,     &QCheckBox::clicked, this, [this](bool checked) { Sys::debugView->showDirection(checked);   emit sig_updateView();} );
    connect(chkArcCen,      &QCheckBox::clicked, this, [this](bool checked) { Sys::debugView->showArcCentres(checked);  emit sig_updateView();} );
    connect(chkPoints,      &QCheckBox::clicked, this, [this](bool checked) { Sys::debugView->showPoints(checked);      emit sig_updateView();} );
    connect(chkMarks,       &QCheckBox::clicked, this, [this](bool checked) { Sys::debugView->showMarks(checked);       emit sig_updateView();} );
    connect(chkCurves,      &QCheckBox::clicked, this, [this](bool checked) { Sys::debugView->showCurves(checked);      emit sig_updateView();} );
    connect(chkCircles,     &QCheckBox::clicked, this, [this](bool checked) { Sys::debugView->showCircles(checked);     emit sig_updateView();} );

    connect(pbEnbDbgView,   &AQPushButton::clicked, this, &page_debug::slot_dbgViewClicked);
    connect(pbUseProto,     &AQPushButton::clicked, this, &page_debug::slot_useProtoMap);
    connect(pbClear,        &QPushButton::clicked,  this, []() { Sys::debugView->unloadLayerContent(); Sys::viewController->slot_updateView(); } );

    return groupBox;
}

QWidget * page_debug::creatDebugFlags()
{
    QPushButton * pbResetStyles  = new QPushButton("Reset Styles");
    QPushButton * pbResetProtos  = new QPushButton("Reset Protos");
    QPushButton * pbResetMotifs  = new QPushButton("Reset Motifs");
    pbEnbDbgView2                = new AQPushButton("Enable View");
    pbEnbDbgFlags                = new AQPushButton("Enable Flags");
    QPushButton  * pbClearFlags  = new QPushButton("Clear Flags");
    QPushButton  * pbClearMap1   = new QPushButton("Clear Create Map");
    QPushButton  * pbClearMap2   = new QPushButton("Clear Paint Map");

    QPushButton * pbReload        = new QPushButton("Reload");
    QPushButton * pbRefreshView   = new QPushButton("Recreate");
    QPushButton * pbUpdateView    = new QPushButton("Repaint");

    pbEnbDbgFlags->setChecked(Sys::flags->enabled());
    pbEnbDbgView2->setChecked(viewControl->isEnabled(VIEW_DEBUG));

    connect(pbResetStyles,  &QPushButton::clicked,  this, [] { Sys::render(RENDER_RESET_STYLES);} );
    connect(pbResetProtos,  &QPushButton::clicked,  this, [] { Sys::render(RENDER_RESET_PROTOTYPES);} );
    connect(pbResetMotifs,  &QPushButton::clicked,  this, [] { Sys::render(RENDER_RESET_MOTIFS);} );
    connect(pbEnbDbgFlags,  &AQPushButton::clicked, this, &page_debug::slot_dbgFlagsClicked);
    connect(pbEnbDbgView2,  &AQPushButton::clicked, this, &page_debug::slot_dbgViewClicked);
    connect(pbClearFlags,   &QPushButton::clicked,  this, &page_debug::slot_clearFlags);
    connect(pbClearMap1,    &QPushButton::clicked,  this, [] { Sys::debugMapCreate->wipeout();  Sys::viewController->slot_updateView(); } );
    connect(pbClearMap2,    &QPushButton::clicked,  this, [] { Sys::debugMapPaint->wipeout(); Sys::viewController->slot_updateView(); } );
    connect(pbUpdateView,   &QPushButton::clicked, Sys::viewController, &SystemViewController::slot_updateView);
    connect(pbRefreshView,  &QPushButton::clicked, Sys::viewController, &SystemViewController::slot_reconstructView);
    connect(pbReload,       &QPushButton::clicked,  panel,              &ControlPanel::slot_reload);

    pbTrigger              = new AQPushButton("Trigger");
    AQPushButton * pbEdge  = new AQPushButton("Edge select");

    QSpinBox * edgeBox = new QSpinBox();
    edgeBox->setRange(0,999);
    edgeBox->setValue(Sys::flags->getDbgIndex());

    QSpinBox * triggerBox = new QSpinBox();
    triggerBox->setRange(0,999);
    triggerBox->setValue(Sys::flags->getTriggerIndex());

    xBox = new QDoubleSpinBox;
    xBox->setRange(-999,999);
    xBox->setValue(0);
    xBox->setAlignment(Qt::AlignCenter);

    yBox = new QDoubleSpinBox;
    yBox->setRange(-999,999);
    yBox->setValue(0);
    yBox->setAlignment(Qt::AlignCenter);

    QLabel * xlabel      = new QLabel("X:");
    QLabel * ylabel     = new QLabel("Y:");
    QPushButton * xyclr = new QPushButton("Clear");

    connect(pbEdge,         &AQPushButton::clicked, this, &page_debug::slot_edgeSelectClicked);
    connect(pbTrigger,      &QPushButton::clicked,  this, &page_debug::slot_triggerClicked);
    connect(xyclr ,         &QPushButton::clicked,  this, [this] { xBox->setValue(0); yBox->setValue(0); Sys::viewController->repaintView();});
    connect(xBox, &AQDoubleSpinBox::valueChanged,   this, [](qreal val) { Sys::flags->setXval(val,true);  });
    connect(yBox, &AQDoubleSpinBox::valueChanged,   this, [](qreal val) { Sys::flags->setYval(val,true); });
    connect(edgeBox,        &QSpinBox::valueChanged,this, [](int val)   { Sys::flags->setDbgIndex(val); } );
    connect(triggerBox,     &QSpinBox::valueChanged,this, [](int val)   { Sys::flags->setTriggerIndex(val); } );

    QHBoxLayout * h0 = new QHBoxLayout;
    h0->addWidget(pbReload);
    h0->addWidget(pbRefreshView);
    h0->addWidget(pbUpdateView);
    h0->addStretch();

    QHBoxLayout * h1 = new QHBoxLayout;
    h1->addWidget(pbEnbDbgFlags);
    h1->addWidget(pbEnbDbgView2);
    h1->addWidget(pbClearFlags);
    h1->addStretch();

    QHBoxLayout * h2 = new QHBoxLayout;
    h2->addWidget(pbResetStyles);
    h2->addWidget(pbResetProtos);
    h2->addWidget(pbResetMotifs);

    QHBoxLayout * h3 = new QHBoxLayout;
    h3->addWidget(pbEdge);
    h3->addWidget(edgeBox);
    h3->addStretch();

    QHBoxLayout * h4 = new QHBoxLayout;
    h4->addWidget(pbTrigger);
    h4->addWidget(triggerBox);
    h4->addStretch();

    QHBoxLayout * h5 = new QHBoxLayout;
    h5->addWidget(pbClearMap1);
    h5->addWidget(pbClearMap2);
    h5->addStretch();

    QHBoxLayout * h8 = new QHBoxLayout;
    h8->addWidget(xlabel);
    h8->addWidget(xBox);
    h8->addWidget(ylabel);
    h8->addWidget(yBox);
    h8->addWidget(xyclr);
    h8->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout();
    vbox->addLayout(h1);
    vbox->addSpacing(11);
    vbox->addLayout(h0);
    vbox->addSpacing(11);
    vbox->addLayout(h2);
    vbox->addSpacing(11);
    vbox->addLayout(h3);
    vbox->addSpacing(11);
    vbox->addLayout(h4);
    vbox->addSpacing(11);
    vbox->addLayout(h8);
    vbox->addSpacing(11);
    vbox->addLayout(h5);
    vbox->addStretch();

    flagTable = new AQTableWidget();
    flagTable->setColumnCount(4);
    flagTable->setSelectionBehavior(QAbstractItemView::SelectItems);
    flagTable->setSelectionMode(QAbstractItemView::MultiSelection);

    QVBoxLayout  * vbox2 = new QVBoxLayout;
    vbox2->addWidget(flagTable);
    vbox2->addStretch();

    QStringList qslH;
    qslH << "Create" << "Active Create" << "Paint" << "Active Paint";
    flagTable->setHorizontalHeaderLabels(qslH);
    flagTable->verticalHeader()->setVisible(false);

    flagTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    flagTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    flagTable->adjustTableSize();

    connect(flagTable, &QTableWidget::cellPressed, this, &page_debug::slot_flagPressed);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addLayout(vbox2);
    hbox->addStretch();
    hbox->addLayout(vbox);

    QWidget * wDebugFlags = new QWidget();
    wDebugFlags->setLayout(hbox);

    return wDebugFlags;
}

void  page_debug::onEnter()
{
    slot_refreshFlags();
    timer->start(500);
}

void page_debug::onExit()
{
    timer->stop();
}

void  page_debug::onRefresh()
{
    if (pick)
        colorPicker();

    pbEnbDbgView->blockSignals(true);
    pbEnbDbgView->setChecked(viewControl->isEnabled(VIEW_DEBUG));
    pbEnbDbgView->blockSignals(false);

    pbEnbDbgView2->blockSignals(true);
    pbEnbDbgView2->setChecked(viewControl->isEnabled(VIEW_DEBUG));
    pbEnbDbgView2->blockSignals(false);

    pbEnbDbgFlags->blockSignals(true);
    pbEnbDbgFlags->setChecked(Sys::flags->enabled());
    pbEnbDbgFlags->blockSignals(false);

    QList<int> ql1 = Sys::debugMapCreate->numInfo();
    QList<int> ql2 = Sys::debugMapPaint->numInfo();

    for (int i= ROW_MARKS; i < ROW_BOT; i++)
    {
        mapCreateCounts[i]->setText(QString::number(ql1[i]));
        mapPaintCounts[i]->setText(QString::number(ql2[i]));
    }

    xBox->blockSignals(true);
    xBox->setValue(Sys::flags->getXval());
    xBox->blockSignals(false);
    yBox->blockSignals(true);
    yBox->setValue(Sys::flags->getYval());
    yBox->blockSignals(false);
}

void page_debug::slot_refreshFlags()
{
    static uint ref = 0;

    uint cref = Sys::flags->getRef();
    if (cref == ref)
        return;
    ref = cref;

    flagTable->clearContents();

    uint repaintCount = 0;
    uint styleCount   = 0;
    //uint motifCount   = 0;

    const QVector<DbgFlag*> & flags = Sys::flags->getFlags();
    for (auto & aflag : flags)
    {
        switch (aflag->type)
        {
        case FLAG_REPAINT:
            repaintCount++;
            break;
        case FLAG_CREATE_STYLE:
            styleCount++;
            break;
        case FLAG_CREATE_MOTIF:
        case NUM_DFLAG_TYPES:
            break;
        }
    }
    uint maxCount = qMax(repaintCount,styleCount);
    flagTable->setRowCount(maxCount);

    QMap<QString,QTableWidgetItem *> createFlags;
    QMap<QString,QTableWidgetItem *> paintFlags;

    for (auto i = flags.cbegin(), end = flags.cend(); i != end; ++i)
    {
        QTableWidgetItem * item;
        const DbgFlag * flag = *i;
        item  = new QTableWidgetItem(flag->name);
        item->setData(Qt::UserRole,flag->flag);

        switch (flag->type)
        {
        case FLAG_REPAINT:
            paintFlags[flag->name] = item;
            break;
        case FLAG_CREATE_STYLE:
        case FLAG_CREATE_MOTIF:
            createFlags[flag->name] = item;
            break;
        case NUM_DFLAG_TYPES:
            break;
        }
    }

    // the two maps are now sorted by name
    int  row = 0;
    for (auto it = createFlags.keyValueBegin(); it != createFlags.keyValueEnd(); ++it)
    {
        QTableWidgetItem * item = it->second;
        DbgFlag * flag          = Sys::flags->find(it->first);

        if (flag->enb)
        {
            flagTable->setItem(row, FLAG_CREATE_ACTIVE,item);
            item->setSelected(true);
        }
        else
        {
            flagTable->setItem(row, FLAG_CREATE_INACTIVE,item);
            item->setSelected(false);
        }
        row++;
    }
    row = 0;
    for (auto it = paintFlags.keyValueBegin(); it != paintFlags.keyValueEnd(); ++it)
    {
        QTableWidgetItem * item = it->second;
        DbgFlag * flag          = Sys::flags->find(it->first);
        if (flag->enb)
        {
            flagTable->setItem(row, FLAG_PAINT_ACTIVE,item);
            item->setSelected(true);
        }
        else
        {
            flagTable->setItem(row, FLAG_PAINT_INACTIVE,item);
            item->setSelected(false);
        }
        row++;
    }

    flagTable->adjustTableSize();
}

void page_debug::slot_flagPressed(int row, int col)
{
    QString name2;
    eDbgFlag flag;

    QTableWidgetItem * item = flagTable->item(row,col);

    if (item)
    {
        name2 = item->text();
        flag  = (eDbgFlag) item->data(Qt::UserRole).toUInt();
    }

    if (!item || name2.isEmpty())
    {
        QTableWidgetItem * item2 = nullptr;
        switch (col)
        {
        case FLAG_CREATE_INACTIVE:
            item2 = flagTable->item(row, FLAG_CREATE_ACTIVE);
            break;
        case FLAG_CREATE_ACTIVE:
            item2 = flagTable->item(row, FLAG_CREATE_INACTIVE);
            break;
        case FLAG_PAINT_INACTIVE:
            item2 = flagTable->item(row,FLAG_PAINT_ACTIVE);
            break;
        case FLAG_PAINT_ACTIVE:
            item2 = flagTable->item(row,FLAG_PAINT_INACTIVE);
            break;
        }

        if (item2 == nullptr)
        {
            // both are empty - put in empty placedholder
            flagTable->setItem(row, col, new QTableWidgetItem);
            flagTable->item(row, col)->setFlags(Qt::NoItemFlags);
            return;
        }

        // item2 exists - but could be empty placeholder
        name2 = item2->text();
        flag  = (eDbgFlag) item2->data(Qt::UserRole).toUInt();
        if (name2.isEmpty())
        {
            Q_ASSERT(item2);
            item2->setFlags(Qt::NoItemFlags);
            Q_ASSERT(item == nullptr);
            flagTable->setItem(row, col, new QTableWidgetItem);
            flagTable->item(row, col)->setFlags(Qt::NoItemFlags);
            return;
        }
    }
    // we have a flag name
    Sys::flags->toggleFlag(flag);
}


void page_debug ::slot_clearFlags()
{
    Sys::flags->clearFlags();
}

void page_debug::verifyTilingNames()
{
    TilingManager tm;
    bool rv = tm.verifyTilingFiles();

    QMessageBox box(this);
    if (rv)
    {
        box.setIcon(QMessageBox::Information);
        box.setText("Tiling Names Verified: OK");
    }
    else
    {
        box.setIcon(QMessageBox::Warning);
        box.setText("ERROR in verifying tiling names. See log");
    }
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void page_debug::reformatMosaicXML()
{
    QMessageBox box(this);
    box.setIcon(QMessageBox::Question);
    box.setText("Reformat All XML mosaic files or a single XML file?");
    QPushButton * yesBtn = box.addButton("Yes",QMessageBox::YesRole);
    QPushButton * selBtn = box.addButton("Select file",QMessageBox::NoRole);
    box.exec();
    if (box.clickedButton() == selBtn)
    {
        QString fileName = QFileDialog::getOpenFileName(this,"Select XML File",Sys::config->rootMediaDir, "Image Files (*.xml)");
        VersionedFile vf(fileName);
        bool rv =  FileServices::reformatXML(vf);
        if (rv)
        {
            QMessageBox box2(this);
            box2.setIcon(QMessageBox::Information);
            box2.setText(QString("Reformat of %1 : OK").arg(fileName));
            box2.setStandardButtons(QMessageBox::Ok);
            box2.exec();
        }
        else
        {
            QMessageBox box2(this);
            box2.setIcon(QMessageBox::Warning);
            box2.setText(QString("Reformat of %1 : FAILED").arg(fileName));
            box2.setStandardButtons(QMessageBox::Ok);
            box2.exec();
        }
        return;
    }
    else
    {
        Q_ASSERT(box.clickedButton() == yesBtn);
        QMessageBox box(this);
        box.setIcon(QMessageBox::Question);
        box.setText("Reformat XML: this is very drastic. Are you sure?");
        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        box.setDefaultButton(QMessageBox::No);
        if (box.exec() == QMessageBox::No)
        {
            return;
        }

        qDebug() << "Reformatting designs...";

        int goodDes = 0;
        int badDes  = 0;
        VersionFileList files = FileServices::getFiles(FILE_MOSAIC);
        for (auto & file : files)
        {
            bool rv =  FileServices::reformatXML(file);
            if (rv)
                goodDes++;
            else
                badDes++;
        }
        qDebug() << "Reformatted" << goodDes << "good designs, " << badDes << "bad designs";

        QMessageBox box2(this);
        box2.setIcon(QMessageBox::Information);
        box2.setText(QString("Reformat XML: %1 good designs, %2 bad designs").arg(goodDes).arg(badDes));
        box2.setStandardButtons(QMessageBox::Ok);
        box2.exec();
    }
}

void page_debug::reformatTilingXML()
{
    QMessageBox box(this);
    box.setIcon(QMessageBox::Question);
    box.setText("Reformat XML: this is very drastic. Are you sure?");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }
    int badTiles  = 0;
    int goodTiles = 0;

    qDebug() << "Reformatting tilings...";

    VersionFileList files = FileServices::getFiles(FILE_TILING);
    for (auto & file : files)
    {
        bool rv =  FileServices::reformatXML(file);
        if (rv)
            goodTiles++;
        else
            badTiles++;
    }
    qDebug() << "Reformatted" << goodTiles << "good tilings, " << badTiles << "bad tilings";

    QMessageBox box2(this);
    box2.setIcon(QMessageBox::Information);
    box2.setText(QString("Reformat XML: %1 good tilings, %2 bad tilings").arg(goodTiles).arg(badTiles));
    box2.setStandardButtons(QMessageBox::Ok);
    box2.exec();
}

void page_debug::reprocessMosaicXML()
{
    QMessageBox box(this);
    box.setIcon(QMessageBox::Question);
    box.setText("Reprocessing Design XML: this is very drastic. Are you sure?");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }

    qDebug() << "Reprocessing designs...";

    int goodDes = 0;
    int badDes  = 0;
    VersionFileList files = FileServices::getMosaicFiles(ALL_MOSAICS);
    for (VersionedFile & file : files)
    {
        MosaicPtr mosaic = mosaicMaker->loadMosaic(file);
        if (mosaic)
        {
            bool rv = mosaicMaker->saveMosaic(mosaic,true);
            if (rv)
                goodDes++;
            else
                badDes++;
        }
        else
        {
            badDes++;
        }
    }

    qDebug() << "Reprocessed" << goodDes << "good designs, " << badDes << "bad designs";

    QMessageBox box2(this);
    box2.setIcon(QMessageBox::Information);
    box2.setText(QString("Reprocess Design XML: %1 good designs, %2 bad designs").arg(goodDes).arg(badDes));
    box2.setStandardButtons(QMessageBox::Ok);
    box2.exec();
}

void page_debug::examineMosaicXML()
{
    VersionFileList files = FileServices::getMosaicFiles(WORKLIST);
    for (VersionedFile & file : files)
    {
        xml_document doc;
        xml_parse_result result = doc.load_file(file.getPathedName().toStdString().c_str());
        if (result == false)
        {
            qWarning() << file.getVersionedName().get() << result.description();
            continue;
        }
        xml_node node = doc.child("vector");
        Q_ASSERT(node);

        xml_attribute vattr = node.attribute("version");
        QString vstring = vattr.value();
        int version = vstring.toInt();

        xml_node dnode = node.child("design");
        Q_ASSERT(dnode);
        xml_node bnode = dnode.child("BackgroundImage");
        if (!bnode)
        {
            qDebug() << file.getVersionedName().get() << "BackgroundImage not found";
            continue;
        }

        qreal scale = 1.0;
        qreal rot = 0;
        qreal x = 0;
        qreal y = 0;
        QPointF center;
        xml_node n = bnode.child("Scale");
        if (n)
        {
            QString str = n.child_value();
            scale = str.toDouble();
        }

        n = bnode.child("Rot");
        if (n)
        {
            QString str = n.child_value();
            rot = str.toDouble();
        }

        n = bnode.child("X");
        if (n)
        {
            QString str= n.child_value();
            x = str.toDouble();
        }

        n = bnode.child("Y");
        if (n)
        {
            QString str = n.child_value();
            y = str.toDouble();
        }

        n = bnode.child("Center");
        if (n)
        {
            // depracted
            QString str = n.child_value();
            QStringList qsl = str.split(",");
            qreal x = qsl[0].toDouble();
            qreal y = qsl[1].toDouble();
            center = QPointF(x,y);
        }

        qDebug() << file.getVersionedName().get() << "version" << version << "scale" << scale << "rot" << rot << "x" << x << "y" << y << "center" << center;
        n = bnode.child("Perspective");
        if (n)
        {
            QString str = n.child_value();
            QTransform t = MosaicReader::getQTransform(str);
            qDebug() << file.getVersionedName().get() << "Perespective" << t;
        }
    }
}

void page_debug::reprocessTilingXML()
{
    QMessageBox box(this);
    box.setIcon(QMessageBox::Question);
    box.setText("Reprocssing Tiling XML: this is very drastic. Are you sure?");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }
    int badTiles  = 0;
    int goodTiles = 0;

    qDebug() << "Reprocessing tilings...";

    VersionFileList files = FileServices::getTilingFiles(ALL_TILINGS);
    for (VersionedFile & file : files)
    {
        bool rv = false;

        TilingManager tm;
        TilingPtr tp = tm.loadTiling(file,TILM_LOAD_SINGLE);
        if (tp)
        {
            Q_ASSERT(tp->getVName().get() == file.getVersionedName().get());
            rv = tm.saveTiling(tp,file,true);
        }
        if (rv)
            goodTiles++;
        else
            badTiles++;
    }

    qDebug() << "Reformatted" << goodTiles << "good tilings, " << badTiles << "bad tilings";

    QMessageBox box2(this);
    box2.setIcon(QMessageBox::Information);
    box2.setText(QString("Reformat XML: %1 good tilings, %2 bad tilings").arg(goodTiles).arg(badTiles));
    box2.setStandardButtons(QMessageBox::Ok);
    box2.exec();
}

void page_debug::slot_verifyProtosClicked(bool enb)
{
    config->forceVerifyProtos = enb;
}

void page_debug::slot_verifyMapsClicked(bool enb)
{
    config->verifyMaps = enb;
}

void page_debug::slot_verifyDumpClicked(bool enb)
{
    config->verifyDump = enb;
}

void page_debug::slot_verifypopupClicked(bool enb)
{
    config->verifyPopup = enb;
}

void page_debug::slot_verifyVerboseClicked(bool enb)
{
    config->verifyVerbose = enb;
}

void page_debug::slot_unDupMerges(bool enb)
{
    config->slowCleanseMapMerges = enb;
}

void page_debug::slot_buildEmptyNMaps(bool enb)
{
    config->buildEmptyNmaps = enb;
}

void page_debug::verifyTiling()
{
    // the strategy is to build a map and then verify that
    TilingPtr tiling = tilingMaker->getTilings().first();
    verifyTiling(tiling);
}

bool page_debug::verifyTiling(TilingPtr tiling)
{
    log->trap(true);
    qInfo().noquote() << "Verifying tiling :" << tiling->getVName().get();
    bool rv = true;
    PlacedTiles tilingUnit = tiling->unit().getIncluded();
    for (const auto & placedTile : std::as_const(tilingUnit))
    {
        auto tile = placedTile->getTile();
        if (!tile->isClockwise())
        {
            qWarning() << "Clockwise Tile";
            rv = false;
        }
    }

    log->suspend(true);

    bool intrinsicOverlaps = tiling->hasIntrinsicOverlaps();
    bool tiledOverlaps     = tiling->hasTiledOverlaps();

    MapPtr protomap = tiling->debug_createProtoMap();

    log->suspend(false);

    MapVerifier mv(protomap);
    bool protomap_verify   = mv.verify(true);
    bool protomap_overlaps = protomap->hasIntersectingEdges();

    log->suspend(true);

    MapPtr fillmap = tiling->debug_createFilledMap();

    log->suspend(false);

    MapVerifier mv2(fillmap);
    bool fillmap_verify   = mv2.verify(true);
    bool fillmap_overlaps = fillmap->hasIntersectingEdges();

    QString name = tiling->getVName().get();
    if (intrinsicOverlaps)
    {
        qWarning() << name << ": intrinsic overlaps";
        rv = false;
    }
    if (tiledOverlaps)
    {
        qWarning() << name << ": tiled overlaps";
        rv = false;
    }
    if (!protomap_verify)
    {
        qWarning() << name << ": proto map verify errors";
        rv = false;
    }
    if (protomap_overlaps)
    {
        qWarning() << name << ": proto map intersecting edges";
        rv = false;
    }
    if (!fillmap_verify)
    {
        qWarning() << name << ": fill map verify errors";
        rv = false;
    }
    if (fillmap_overlaps)
    {
        qWarning() << name << ": fill map intersecting edges";
        rv = false;
    }

    if (!rv)
    {
        DlgTextEdit dlg(this);
        const auto & msgs = qtAppLog::getTrap();
        dlg.set(msgs);
        dlg.exec();
    }
    log->trap(false);
    return rv;
}

void page_debug::identifyDuplicateTiles(TilingPtr tiling)
{
    qDebug() << "Looking for duplicates -" << tiling->getVName().get();
    for (auto & tile1 : tiling->unit().getUniqueTiles())
    {
        for (auto & tile2 : tiling->unit().getUniqueTiles())
        {
            if (tile1 == tile2)
            {
                qInfo() << "DUP:" << tiling->getVName().get() << tile1.get() << "equals" << tile2.get();
            }
        }
    }
    qDebug() << "Looking for duplicates - end";
}

void page_debug::verifyAllTilings()
{
    qDebug() << "Verifying all tilings...";

    VersionFileList files = FileServices::getTilingFiles(ALL_TILINGS);
    for (VersionedFile & file : files)
    {
        qInfo() << "==========" << file.getVersionedName().get() << "==========";
        VersionFileList uses = FileServices::whereTilingUsed(file.getVersionedName());
        if (uses.size() == 0)
        {
            qInfo() << "     not used";
            continue;
        }

        TilingManager tm;
        TilingPtr tp = tm.loadTiling(file,TILM_LOAD_SINGLE);
        if (!tp)
        {
            QMessageBox box(this);
            box.setIcon(QMessageBox::Warning);
            box.setText(QString("Error loading tiling: %1").arg(file.getVersionedName().get()));
            box.setStandardButtons(QMessageBox::Ok);
            box.exec();
            continue;
        }

        //verifyTiling(tp);
        identifyDuplicateTiles(tp);

    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("All tilings verification complete");
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void page_debug::examineAllMosaics()
{
    qDebug() << "Examining all Mosaics...";

    VersionFileList files = FileServices::getFiles(FILE_MOSAIC);
    for (VersionedFile & file : files)
    {
        qDebug() << "LOG2 ==========" << file.getVersionedName().get() << "==========";

        MosaicReader reader(Sys::viewController);
        MosaicPtr mosaic = reader.readXML(file);
        if (!mosaic)
        {
            QString str = QString("Load ERROR - %1").arg(reader.getFailMessage());
            QMessageBox box(panel);
            box.setIcon(QMessageBox::Warning);
            box.setText(str);
            box.exec();
            continue;
        }

        examineMosaic(mosaic,2);
    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("All Mosaics examined");
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void page_debug::examineMosaic()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    examineMosaic(mosaic,1);
}

void page_debug::examineMosaic(MosaicPtr mosaic, uint test)
{
    if (test == 1)
    {
        qInfo() << "examining mosaic :" << mosaic->getName().get() << "- START";

        QVector<TilingPtr> tilings = mosaic->getTilings();
        TilingPtr tiling = tilings.first();     // yeah only one for this exercise
        qInfo().noquote() << "tiling:" << tiling->getVName().get();

        verifyTiling(tiling);

        const StyleSet & styles = mosaic->getStyleSet();
        for (auto & style : styles)
        {
            FilledPtr fp = std::dynamic_pointer_cast<Filled>(style);
            if (fp)
                qInfo() << fp->getStyleDesc() << "algorithm:" << fp->getAlgorithm() << "proto" << style->getPrototype().get();
            else
            {
                auto proto = style->getPrototype();
                if (proto)
                    qInfo() << style->getStyleDesc() << "proto" << proto.get();
                else
                    qInfo() << style->getStyleDesc() << "No prototype";
            }
        }

        QVector<ProtoPtr> protos = mosaic->getPrototypes();
        for (auto & prototype : protos)
        {
            qInfo() << "proto" << prototype.get();

            log->suspend(true);
            MapPtr map = prototype->getProtoMap();
            log->suspend(false);

            MapVerifier mv(map);
            mv.verify(true);

            if (map->hasIntersectingEdges())
            {
                qInfo() << prototype.get() << "has intersecting edges";
            }
            qDebug().noquote() << map->displayVertexEdgeCounts();
        }
        qInfo() << "examining mosaic :" << mosaic->getName().get() << "- END";
    }
    else if (test == 2)
    {
        auto proto = mosaic->getPrototypes().first();
        auto map   = proto->getProtoMap();
        NeighbourMap nmap(map);
        nmap.examine();
    }
}

void page_debug::slot_dontTrapLog(bool dont)
{
    Sys::dontTrapLog = dont;
}

void page_debug::slot_dontRefresh(bool enb)
{
    Sys::updatePanel = !enb;
}

void  page_debug::slot_dbgViewClicked(bool checked)
{
    panel->delegateView(VIEW_DEBUG,checked);
    Sys::viewController->slot_reconstructView();
}

void  page_debug::slot_dbgFlagsClicked(bool checked)
{
    Sys::flags->enable(checked);
    if (checked == false)
    {
        Sys::debugMapCreate->wipeout();
        Sys::debugMapPaint->wipeout();
    }
    Sys::render(RENDER_RESET_ALL);
    Sys::viewController->slot_updateView();
}

void  page_debug::slot_edgeSelectClicked(bool checked)
{
    Sys::flags->setIndexEnable(checked);
    Sys::render(RENDER_RESET_STYLES);
}

void  page_debug::slot_triggerClicked(bool checked)
{
    if (checked)
    {
        Sys::flags->trigger();
        QThread::msleep(350);
        pbTrigger->setChecked(false);
    }
}

void  page_debug::slot_viewViewCen(bool checked)
{
    config->showCenterDebug = checked;
    emit sig_updateView();
}

void page_debug::slot_useProtoMap(bool checked)
{
    if (checked)
    {
        Map * protomap = Sys::prototypeMaker->getSelectedPrototype()->getProtoMap().get();
        if (protomap)
        {
           Sys::debugMapCreate->set(protomap,Qt::blue,Qt::red);
        }
    }
    else
    {
        Sys::debugMapCreate->wipeout();
        Sys::render(RENDER_RESET_MOTIFS);
    }
    emit sig_updateView();
}

void page_debug::slot_measure(bool checked)
{
    Sys::debugView->setCanMeasure(checked);
    if (checked)
    {
        slot_dbgViewClicked(true);
    }
    else
    {
        Sys::debugView->clearMeasurements();
        emit sig_updateView();
    }
}

void page_debug::reformatOldTemplates()
{
    qInfo() << "Reformatting old construction line templates";
    Sys::config->mapedOldTemplates = true;
    VersionFileList xfl = FileServices::getFiles(FILE_TEMPLATE);
    for (VersionedFile xfile : std::as_const(xfl))
    {
        qDebug() << "Processing" << xfile.getVersionedName().get();
        Sys::mapEditor->unload();
        bool rv = Sys::mapEditor->loadTemplate(xfile,false);
        if (rv)
        {
            rv = Sys::mapEditor->saveTemplate(xfile.getVersionedName());
            if (rv)
            {
                rv = QFile::remove(xfile.getPathedName());
            }
        }
        if (!rv) qWarning() << "FAILED";
    }
    qInfo() << "Reformatting complete";
}

void  page_debug::testA()
{
#if 0
    QPointF a(100,200);
    qDebug() << a;
    QLineF l1(QPointF(0,0),a);
    qDebug() << l1.length() << l1.angle();
    QLineF l3 = l1.unitVector();
    qDebug() << l3;

    Geo::normalizeD(a);
    qDebug() << a;
    QLineF l2(QPointF(0,0),a);
    qDebug() << l2.length() << l2.angle();

    emit sig_testA();
#endif
    Sys::debugView->do_testA(true);
}

void  page_debug::testB()
{
    //emit panel->sig_id_layers();
    //emit sig_testB();
    Sys::debugView->do_testB(true);
}

void page_debug::slot_startPicker(bool checked)
{
    pick = checked;
}

void page_debug::colorPicker()
{
    QPoint cursorPos = QCursor::pos();
    QScreen *screen = QGuiApplication::primaryScreen();
    QPixmap screenshot = screen->grabWindow(0);
    QImage image = screenshot.toImage();

    QColor color = image.pixelColor(cursorPos);
    colorTxt->setText(color.name());
}
