#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidgetItem>

#include "gui/panels/page_canvas_settings.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view.h"
#include "gui/top/view_controller.h"
#include "gui/widgets/layout_sliderset.h"
#include "gui/widgets/panel_misc.h"
#include "legacy/design.h"
#include "legacy/design_maker.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/transform.h"
#include "sys/qt/qtapplog.h"

using std::string;
using std::make_shared;

page_canvas_settings::page_canvas_settings(ControlPanel * apanel)  : panel_page(apanel,PAGE_CANVAS_SETTINGS,"Canvas")
{
    QGroupBox * taker  = createTakerStatus();
    QGroupBox * canvas = createCanvasStatus();
    QGroupBox * mosaic = createMosaicSettings();
    QGroupBox * tiling = createTilingSettings();

    QBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(mosaic);
    hbox->addWidget(tiling);

    QVBoxLayout * abox = new QVBoxLayout;
    abox->addWidget(taker);
    abox->addWidget(canvas);
    abox->addSpacing(11);
    abox->addLayout(hbox);
    abox->addStretch();

    vbox->addLayout(abox);
    adjustSize();

    // connections
    connect(tilingMaker,   &TilingMaker::sig_tilingLoading,      this,   &page_canvas_settings::slot_unlockAspectRatio);
    connect(mosaicMaker,   &MosaicMaker::sig_mosaicLoading,      this,   &page_canvas_settings::slot_unlockAspectRatio);
    connect(designMaker,   &DesignMaker::sig_loadingDesign,      this,   &page_canvas_settings::slot_unlockAspectRatio);
    connect(Sys::systemViewController->getView(),  &SystemView::sig_viewSizeChanged,    this,   &page_canvas_settings::sysViewChanged);
}

QGroupBox *page_canvas_settings::createTilingSettings()
{
    QLabel * l1         = new QLabel( "Canvas :");
    sizeTilingCanvasW   = new SpinSet("width   ",0,1,4096);
    sizeTilingCanvasH   = new SpinSet("height  ",0,1,2160);

    QLabel * l3         = new QLabel( "Delta  :");
    sizeTilingDeltaW    = new SpinSet("width   ",0,-2048,2024);
    sizeTilingDeltaH    = new SpinSet("height  ",0,-1080,1080);

    QLabel * l2         = new QLabel( "View :  ");
    sizeTilingViewW     = new SpinSet("width   ",0,1,4096);
    sizeTilingViewH     = new SpinSet("height  ",0,1,2160);

    bkgdColorEdit[TILING_SETTINGS]    = new QLineEdit;
    bkgdColorPatch[TILING_SETTINGS] = new ClickableLabel;

    bkgdColorEdit[TILING_SETTINGS]->setFixedWidth(71);
    bkgdColorPatch[TILING_SETTINGS]->setFixedWidth(121);

    QGridLayout * fillGrid = createFillDataRow(TILING_SETTINGS);

    QGridLayout * grid = new QGridLayout();

    int row = 0;
    grid->addWidget(l2,row,0);
    grid->addLayout(sizeTilingViewW,row,1);
    grid->addLayout(sizeTilingViewH,row,2);

    row++;
    grid->addWidget(l1,row,0);
    grid->addLayout(sizeTilingCanvasW,row,1);
    grid->addLayout(sizeTilingCanvasH,row,2);

    row++;
    grid->addWidget(l3,row,0);
    grid->addLayout(sizeTilingDeltaW,row,1);
    grid->addLayout(sizeTilingDeltaH,row,2);

    QGridLayout * grid2 = new QGridLayout();
    grid2->addWidget(bkgdColorEdit[TILING_SETTINGS],1,0);
    grid2->addWidget(bkgdColorPatch[TILING_SETTINGS],1,1);
    grid2->addLayout(fillGrid,2,0,2,2);

    distortTilingCombo = new AQComboBox();
    distortTilingCombo->addItem("No Distortion",     DISTORT_NONE);
    distortTilingCombo->addItem("Canvas Distortion", DISTORT_CANVAS);
    distortTilingCombo->addItem("Tile Distortion",   DISTORT_TILE);
    distortTilingX   = new DoubleSpinSet("X:",1.0,-9.99,9.99);
    distortTilingX->setSingleStep(0.01);
    distortTilingY   = new DoubleSpinSet("Y:",1.0,-9.99,9.99);
    distortTilingY->setSingleStep(0.01);

    QHBoxLayout * distortBox = new QHBoxLayout;
    distortBox->addWidget(distortTilingCombo);
    distortBox->addLayout(distortTilingX);
    distortBox->addSpacing(5);
    distortBox->addLayout(distortTilingY);
    distortBox->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(grid);
    vbox->addLayout(grid2);
    vbox->addLayout(distortBox);

    QGroupBox * box = new QGroupBox("Tiling Canvas Settings");
    box->setAlignment(Qt::AlignTop);
    box->setLayout(vbox);
    tilingCanvasBox = box;

    connect(sizeTilingCanvasW, &SpinSet::valueChanged, this, &page_canvas_settings::slot_tilingCanvasChanged);
    connect(sizeTilingCanvasH, &SpinSet::valueChanged, this, &page_canvas_settings::slot_tilingCanvasChanged);
    connect(sizeTilingDeltaW,  &SpinSet::valueChanged, this, &page_canvas_settings::slot_tilingDeltaChanged);
    connect(sizeTilingDeltaH,  &SpinSet::valueChanged, this, &page_canvas_settings::slot_tilingDeltaChanged);
    connect(sizeTilingViewW,   &SpinSet::valueChanged, this, &page_canvas_settings::slot_tilingViewChanged);
    connect(sizeTilingViewH,   &SpinSet::valueChanged, this, &page_canvas_settings::slot_tilingViewChanged);

    connect(distortTilingCombo, &QComboBox::currentIndexChanged, this, &page_canvas_settings::slot_tilingDistortionChanged);
    connect(distortTilingX,   &DoubleSpinSet::valueChanged, this, &page_canvas_settings::slot_tilingDistortionChanged);
    connect(distortTilingY,   &DoubleSpinSet::valueChanged, this, &page_canvas_settings::slot_tilingDistortionChanged);

    connect(bkgdColorPatch[TILING_SETTINGS],&ClickableLabel::clicked,this, &page_canvas_settings::backgroundColorDesignPickTiling);
    connect(bkgdColorEdit[TILING_SETTINGS],   &QLineEdit::textChanged, this, &page_canvas_settings::backgroundColorDesignChangedTiling);

    return box;
}

QGroupBox *page_canvas_settings::createMosaicSettings()
{
    QLabel * l1         = new QLabel( "Canvas :");
    sizeMosaicCanvasW   = new SpinSet("width   ",0,1,4096);
    sizeMosaicCanvasH   = new SpinSet("height  ",0,1,2160);

    QLabel * l3         = new QLabel( "Delta  :");
    sizeMosaicDeltaW    = new SpinSet("width   ",0,-2048,2024);
    sizeMosaicDeltaH    = new SpinSet("height  ",0,-1080,1080);

    QLabel * l2         = new QLabel( "View :  ");
    sizeMosaicViewW     = new SpinSet("width   ",0,1,4096);
    sizeMosaicViewH     = new SpinSet("height  ",0,1,2160);

    bkgdColorEdit[MOSAIC_SETTINGS]    = new QLineEdit;
    bkgdColorPatch[MOSAIC_SETTINGS] = new ClickableLabel;

    bkgdColorEdit[MOSAIC_SETTINGS]->setFixedWidth(71);
    bkgdColorPatch[MOSAIC_SETTINGS]->setFixedWidth(121);

    QGridLayout * fillGrid = createFillDataRow(MOSAIC_SETTINGS);

    QGridLayout * grid = new QGridLayout();
    int row = 0;

    grid->addWidget(l2,row,0);
    grid->addLayout(sizeMosaicViewW,row,1);
    grid->addLayout(sizeMosaicViewH,row,2);

    row++;
    grid->addWidget(l1,row,0);
    grid->addLayout(sizeMosaicCanvasW,row,1);
    grid->addLayout(sizeMosaicCanvasH,row,2);

    row++;
    grid->addWidget(l3,row,0);
    grid->addLayout(sizeMosaicDeltaW,row,1);
    grid->addLayout(sizeMosaicDeltaH,row,2);

    QGridLayout * grid2 = new QGridLayout();
    grid2->addWidget(bkgdColorEdit[MOSAIC_SETTINGS],1,0);
    grid2->addWidget(bkgdColorPatch[MOSAIC_SETTINGS],1,1);
    grid2->addLayout(fillGrid,2,0,2,2);

    distortMosaicCombo = new AQComboBox();
    distortMosaicCombo->addItem("No Distortion",     DISTORT_NONE);
    distortMosaicCombo->addItem("Canvas Distortion", DISTORT_CANVAS);
    distortMosaicCombo->addItem("Tile Distortion",   DISTORT_TILE);
    distortMosaicX   = new DoubleSpinSet("X:",1.0,-9.99,9.99);
    distortMosaicX->setSingleStep(0.01);
    distortMosaicY   = new DoubleSpinSet("Y:",1.0,-9.99,9.99);
    distortMosaicY->setSingleStep(0.01);

    QHBoxLayout * distortBox = new QHBoxLayout;
    distortBox->addWidget(distortMosaicCombo);
    distortBox->addLayout(distortMosaicX);
    distortBox->addSpacing(5);
    distortBox->addLayout(distortMosaicY);
    distortBox->addStretch();

    QVBoxLayout * vb = new QVBoxLayout;
    vb->addLayout(grid);
    vb->addLayout(grid2);
    vb->addLayout(distortBox);

    QGroupBox * box = new QGroupBox("Mosaic Canvas Settings");
    box->setLayout(vb);
    mosaicCanvasBox = box;

    connect(sizeMosaicCanvasW,   &SpinSet::valueChanged,  this, &page_canvas_settings::slot_mosaicCanvasChanged);
    connect(sizeMosaicCanvasH,   &SpinSet::valueChanged,  this, &page_canvas_settings::slot_mosaicCanvasChanged);
    connect(sizeMosaicDeltaW,    &SpinSet::valueChanged,  this, &page_canvas_settings::slot_mosaicDeltaChanged);
    connect(sizeMosaicDeltaH,    &SpinSet::valueChanged,  this, &page_canvas_settings::slot_mosaicDeltaChanged);
    connect(sizeMosaicViewW,     &SpinSet::valueChanged,  this, &page_canvas_settings::slot_mosaicViewChanged);
    connect(sizeMosaicViewH,     &SpinSet::valueChanged,  this, &page_canvas_settings::slot_mosaicViewChanged);

    connect(distortMosaicCombo, &QComboBox::currentIndexChanged, this, &page_canvas_settings::slot_mosaicDistortionChanged);
    connect(distortMosaicX,   &DoubleSpinSet::valueChanged, this, &page_canvas_settings::slot_mosaicDistortionChanged);
    connect(distortMosaicY,   &DoubleSpinSet::valueChanged, this, &page_canvas_settings::slot_mosaicDistortionChanged);

    connect(bkgdColorPatch[MOSAIC_SETTINGS],&ClickableLabel::clicked,this, &page_canvas_settings::backgroundColorDesignPick);
    connect(bkgdColorEdit[MOSAIC_SETTINGS],   &QLineEdit::textChanged, this, &page_canvas_settings::backgroundColorDesignChanged);

    return box;
}

QGroupBox *page_canvas_settings::createTakerStatus()
{
    QLabel * actualLabel = new QLabel( "View Size :  ");
    QLabel * wLabel      = new QLabel( "width   ");
    QLabel * hLabel      = new QLabel( "height  ");
    QLabel * bLabel      = new QLabel( "Background Color :");

    takerSizeW          = new QLineEdit();
    takerSizeW->setReadOnly(true);
    takerSizeW->setMaximumWidth(35);

    takerSizeH          = new QLineEdit();
    takerSizeH->setReadOnly(true);
    takerSizeH->setMaximumWidth(35);

    QHBoxLayout * sbox  = new QHBoxLayout;
    sbox->addWidget(actualLabel);
    sbox->addWidget(wLabel);
    sbox->addWidget(takerSizeW);
    sbox->addSpacing(11);
    sbox->addWidget(hLabel);
    sbox->addWidget(takerSizeH);
    sbox->addStretch();

    bkgdColorEdit[TAKER]  = new QLineEdit;
    bkgdColorPatch[TAKER] = new ClickableLabel;
    takerColorSource      = new QLineEdit;

    bkgdColorEdit[TAKER]->setFixedWidth(71);
    bkgdColorPatch[TAKER]->setFixedWidth(121);

    takerSizeW->setReadOnly(true);
    takerSizeH->setReadOnly(true);
    bkgdColorEdit[TAKER]->setReadOnly(true);
    takerColorSource->setReadOnly(true);

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(bLabel);
    hbox2->addWidget(bkgdColorEdit[TAKER]);
    hbox2->addWidget(bkgdColorPatch[TAKER]);
    hbox2->addWidget(takerColorSource);
    hbox2->addStretch();

    QVBoxLayout * vbox= new QVBoxLayout;
    vbox->addLayout(sbox);
    vbox->addLayout(hbox2);

    QGroupBox * tbox =  new QGroupBox("Taker :");
    tbox->setLayout(vbox);
    return tbox;
}

QGroupBox *page_canvas_settings::createCanvasStatus()
{
    QLabel * actualLabel = new QLabel( "Actual :");
    QLabel * wLabel      = new QLabel( "width   ");
    QLabel * hLabel      = new QLabel( "height  ");

    actualSizeW          = new QLineEdit();
    actualSizeW->setReadOnly(true);
    actualSizeW->setMaximumWidth(35);

    actualSizeH          = new QLineEdit();
    actualSizeH->setReadOnly(true);
    actualSizeH->setMaximumWidth(35);

    QHBoxLayout * wdBox  = new QHBoxLayout;
    wdBox->addWidget(wLabel);
    wdBox->addWidget(actualSizeW);
    wdBox->addStretch();

    QHBoxLayout * htBox  = new QHBoxLayout;
    htBox->addWidget(hLabel);
    htBox->addWidget(actualSizeH);
    htBox->addStretch();

    chkLockAspectRatio = new QCheckBox("Lock Aspect Ratio");
    chkLockAspectRatio->setChecked(Sys::lockViewAspectRatio);

    aspectRatioVal = new QLineEdit();
    aspectRatioVal->setReadOnly(true);
    aspectRatioVal->setMaximumWidth(71);

    QHBoxLayout * aspBox = new QHBoxLayout;
    aspBox->addWidget(chkLockAspectRatio);
    aspBox->addWidget(aspectRatioVal);
    aspBox->addStretch();

    l_primaryLayer = new QLabel();

#ifdef VARIABLE_BOUNDS
    ds_left  = new DoubleSpinSet("left",0,-3480,3480);
    ds_top   = new DoubleSpinSet("top",0,-2160,2160);
    ds_width = new DoubleSpinSet("width",1,1,100);

    QHBoxLayout * boundsbox = new QHBoxLayout;
    boundsbox->addLayout(ds_left);
    boundsbox->addLayout(ds_top);
    boundsbox->addLayout(ds_width);
#endif

    l_xform   = new QLabel();
    l_canvas  = new QLabel();
    l_layer   = new QLabel();

    bkgdColorEdit[CANVAS]    = new QLineEdit;
    bkgdColorPatch[CANVAS] = new ClickableLabel;

    bkgdColorEdit[CANVAS]->setFixedWidth(71);
    bkgdColorPatch[CANVAS]->setFixedWidth(121);
    //sizeW[VIEW_STATUS]->setReadOnly(true);
    //sizeH[VIEW_STATUS]->setReadOnly(true);
    bkgdColorEdit[CANVAS]->setReadOnly(true);

#ifdef VARIABLE_BOUNDS
    connect(ds_left,               &DoubleSpinSet::valueChanged, this, &page_canvas_settings::slot_boundsChanged);
    connect(ds_top,                &DoubleSpinSet::valueChanged, this, &page_canvas_settings::slot_boundsChanged);
    connect(ds_width,              &DoubleSpinSet::valueChanged, this, &page_canvas_settings::slot_boundsChanged);
#endif

    QGridLayout * grid = new QGridLayout;

    int row = 0;
    grid->addWidget(l_primaryLayer,row,0,1,3);

    row++;
    grid->addWidget(actualLabel,row,0);
    grid->addLayout(wdBox,row,1);
    grid->addLayout(htBox,row,2);
    grid->addLayout(aspBox,row,3);

    QHBoxLayout * hbox1 = new QHBoxLayout;
    hbox1->addLayout(grid);
    hbox1->addStretch();

    QHBoxLayout * hbox3 = new QHBoxLayout;
    hbox3->addWidget(bkgdColorEdit[CANVAS]);
    hbox3->addWidget(bkgdColorPatch[CANVAS]);
    hbox3->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout();
    vbox->addLayout(hbox1);
    vbox->addLayout(hbox3);
#ifdef VARIABLE_BOUNDS
    vbox->addLayout(boundsbox);
#endif
    vbox->addStretch(1);
    vbox->addWidget(l_xform);
    vbox->addWidget(l_canvas);
    vbox->addWidget(l_layer);
    vbox->addStretch(3);

    viewBox = new QGroupBox();
    viewBox->setLayout(vbox);

    connect(chkLockAspectRatio, &QCheckBox::clicked, this, &page_canvas_settings::slot_chkAspectRatio);
    return viewBox;
}

QGridLayout * page_canvas_settings::createFillDataRow(eSettingsGroup group)
{
    QGridLayout * grid = new QGridLayout;

    const int rmin = -99;
    const int rmax =  99;

    chkSingle[group] = new QCheckBox("Singleton");
    xRepMin[group] = new AQSpinBox();
    xRepMax[group] = new AQSpinBox();
    yRepMin[group] = new AQSpinBox();
    yRepMax[group] = new AQSpinBox();

    xRepMin[group]->setRange(rmin,rmax);
    xRepMax[group]->setRange(rmin,rmax);
    yRepMin[group]->setRange(rmin,rmax);
    yRepMax[group]->setRange(rmin,rmax);

    grid->addWidget(chkSingle[group],0,0,1,2);

    grid->addWidget(new QLabel("xMin"),1,0);
    grid->addWidget(new QLabel("xMax"),1,1);
    grid->addWidget(new QLabel("yMin"),1,2);
    grid->addWidget(new QLabel("yMax"),1,3);

    grid->addWidget(xRepMin[group],2,0);
    grid->addWidget(xRepMax[group],2,1);
    grid->addWidget(yRepMin[group],2,2);
    grid->addWidget(yRepMax[group],2,3);

    if (group == MOSAIC_SETTINGS)
    {
        connect(chkSingle[group], &QCheckBox::clicked, this, &page_canvas_settings::singleton_changed_des);

        connect(xRepMin[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsMosaic(int)));
        connect(xRepMax[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsMosaic(int)));
        connect(yRepMin[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsMosaic(int)));
        connect(yRepMax[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsMosaic(int)));
    }
    else if (group == TILING_SETTINGS)
    {
        connect(chkSingle[group], &QCheckBox::clicked, this, &page_canvas_settings::singleton_changed_tile);
        connect(xRepMin[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsTiling(int)));
        connect(xRepMax[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsTiling(int)));
        connect(yRepMin[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsTiling(int)));
        connect(yRepMax[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsTiling(int)));
    }
    else
    {
        chkSingle[group]->setEnabled(false);
        xRepMin[group]->setReadOnly(true);
        xRepMax[group]->setReadOnly(true);
        yRepMin[group]->setReadOnly(true);
        yRepMax[group]->setReadOnly(true);
    }
    return grid;
}


void  page_canvas_settings::onRefresh()
{
    //if (!refresh) return;

    qtAppLog::getInstance()->suspend(true);
    blockPage(true);

    refreshTaker();
    refreshTiling();
    refreshMosaic();
    refreshSystemView();
    tallyAspectRatio();

    blockPage(false);
    qtAppLog::getInstance()->suspend(false);

}

void  page_canvas_settings::refreshTaker()
{
    const Taker * taker = Sys::systemViewController->getTaker();
    QSize sz      = taker->getSize();
    QColor color  = taker->getColor();
    Layer::Type evt = taker->getColorSource();

    takerSizeW->setText(QString::number(sz.width()));
    takerSizeH->setText(QString::number(sz.height()));

    bkgdColorEdit[TAKER]->setText(color.name(QColor::HexArgb));

    QVariant variant = color;
    QString colcode  = variant.toString();
    bkgdColorPatch[TAKER]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    takerColorSource->setText(sLayerType2[(int)evt]);
}

void  page_canvas_settings::refreshTiling()
{
    // tiling settings
    TilingPtr tiling = tilingMaker->getSelectedTiling();
    if (tiling)
    {
        Canvas * canvas = tiling->hdr().getTilingCanvas();
        QSize csize  = canvas->getRawCanvasSize();
        QSize dsize  = canvas->getDeltaCanvasSize();
        QSize vsize  = canvas->getViewSize();

        sizeTilingCanvasW->setValue(csize.width());
        sizeTilingCanvasH->setValue(csize.height());
        sizeTilingDeltaW->setValue(dsize.width());
        sizeTilingDeltaH->setValue(dsize.height());
        sizeTilingViewW->setValue(vsize.width());
        sizeTilingViewH->setValue(vsize.height());

        // distortion
        eDistortionMode distortMode; qreal distortValX,distortValY;
        canvas->getDistortion().get(distortMode,distortValX,distortValY);
        distortTilingCombo->select(distortMode);
        distortTilingX->setValue(distortValX);
        distortTilingY->setValue(distortValY);

        // background color
        QColor qc = config->getBkgdColor(Layer::Type::Tiling);
        bkgdColorEdit[TILING_SETTINGS]->setText(qc.name(QColor::HexArgb));
        QVariant variant = qc;
        QString colcode  = variant.toString();
        bkgdColorPatch[TILING_SETTINGS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

        // repeats
        const FillData & fd = canvas->getFillData();
        displayFillData(fd,TILING_SETTINGS);
    }
}

void  page_canvas_settings::refreshMosaic()
{
    // mosaic/design settings;
    Canvas * mset = getMosaicOrDesignCanvas();

    // size
    QSize vsize = mset->getViewSize();
    QSize csize = mset->getRawCanvasSize();
    QSize dsize = mset->getDeltaCanvasSize();

    sizeMosaicCanvasW->setValue(csize.width());
    sizeMosaicCanvasH->setValue(csize.height());
    sizeMosaicDeltaW->setValue(dsize.width());
    sizeMosaicDeltaH->setValue(dsize.height());
    sizeMosaicViewW->setValue(vsize.width());
    sizeMosaicViewH->setValue(vsize.height());

    // distortion
    eDistortionMode distortMode; qreal distortValX,distortValY;
    mset->getDistortion().get(distortMode,distortValX,distortValY);
    distortMosaicCombo->select(distortMode);
    distortMosaicX->setValue(distortValX);
    distortMosaicY->setValue(distortValY);

    // background color
    QColor qc = getMosaicOrDesignColor();
    bkgdColorEdit[MOSAIC_SETTINGS]->setText(qc.name(QColor::HexArgb));
    QVariant variant = qc;
    QString colcode  = variant.toString();
    bkgdColorPatch[MOSAIC_SETTINGS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    // repeats
    const FillData & fd = mset->getFillData();
    displayFillData(fd,MOSAIC_SETTINGS);
}

void  page_canvas_settings::refreshSystemView()
{
    // Canvas status
    QSize size       = Sys::systemViewController->viewSize();

    actualSizeW->setText(QString::number(size.width()));
    actualSizeH->setText(QString::number(size.height()));

    // background color
    QColor qc = Sys::systemViewController->getViewBackgroundColor();
    QVariant qv = qc;
    QString colcode  = qv.toString();
    bkgdColorEdit[CANVAS]->setText(qc.name(QColor::HexArgb));
    bkgdColorPatch[CANVAS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    // a stylesheet on QGroupBox suppresses its automatic title-clearance math, so the
    // border/title spacing has to be restated explicitly or the border draws through the title
    const QString highlight =
        "QGroupBox { border: 2px solid limegreen; margin-top: 1.2em; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 3px; }";
    tilingCanvasBox->setStyleSheet(Sys::getActiveCanvasOwner() == ACTIVE_TILING ? highlight : "");
    mosaicCanvasBox->setStyleSheet(Sys::getActiveCanvasOwner() == ACTIVE_MOSAIC ? highlight : "");

    viewBox->setTitle(Sys::getActiveCanvasOwner() == ACTIVE_TILING ? "System View :  Tiling" : "System View :  Mosaic");
    viewBox->setStyleSheet(
        "QGroupBox { margin-top: 1.2em; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 3px; color: limegreen; font-weight: bold; }");

    // frame settings
    LayerPtr layer     = viewControl->getSelectedPrimaryLayer();
    if (layer)
    {
        Layer::Type viewType = layer->type();
        l_primaryLayer->setText(QString("Primary Layer :  %1").arg(sLayerType[(int)viewType]));

#ifdef VARIABLE_BOUNDS
        auto bounds = Sys::getActiveCanvas()->getBounds();
        ds_left->setValue(bounds.left);
        ds_top->setValue(bounds.top);
        ds_width->setValue(bounds.width);
#endif

        l_xform->setText( "<pre>Canvas " + Transform::info(layer->getCanvasTransform()) + "</pre>");
        l_canvas->setText("<pre>Model  " + Transform::info(layer->getModelTransform())  + "</pre>");
        l_layer->setText( "<pre>Layer  " + Transform::info(layer->getLayerTransform())  + "</pre>");
    }
}

void page_canvas_settings::onEnter()
{
}

void page_canvas_settings::slot_chkAspectRatio(bool checked)
{
    Sys::lockViewAspectRatio = checked;
    tallyAspectRatio();     // calulates too
}

void page_canvas_settings::slot_unlockAspectRatio()
{
    Sys::lockViewAspectRatio = false;
    tallyAspectRatio();     // calulates too
}

void page_canvas_settings::slot_set_repsMosaic(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    FillData fd;
    fd.set(chkSingle[MOSAIC_SETTINGS]->isChecked(),xRepMin[MOSAIC_SETTINGS]->value(), xRepMax[MOSAIC_SETTINGS]->value(), yRepMin[MOSAIC_SETTINGS]->value(), yRepMax[MOSAIC_SETTINGS]->value());

    auto mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        Canvas * canvas = mosaic->getMosaicCanvas();
        canvas->setFillData(fd);
    }

    Sys::render(RENDER_RESET_PROTOTYPES);
}

void page_canvas_settings::slot_set_repsTiling(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    FillData fd;
    fd.set(chkSingle[TILING_SETTINGS]->isChecked(),xRepMin[TILING_SETTINGS]->value(), xRepMax[TILING_SETTINGS]->value(), yRepMin[TILING_SETTINGS]->value(), yRepMax[TILING_SETTINGS]->value());

    TilingPtr tiling = tilingMaker->getSelectedTiling();
    if (!tiling) return;

    Canvas * canvas = tiling->hdr().getTilingCanvas();
    canvas->setFillData(fd);

    if (viewControl->isEnabled(Layer::Type::TilingMaker)
        || viewControl->isEnabled(Layer::Type::Tiling)
        || viewControl->isEnabled(Layer::Type::MapEditor))
    {
        emit sig_reconstructView();
    }
    else
    {
        Sys::render(RENDER_RESET_PROTOTYPES);
    }
}

void page_canvas_settings::slot_mosaicViewChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeMosaicViewW->value(),sizeMosaicViewH->value());
    if (Sys::lockViewAspectRatio)
    {
        sz = SystemView::enforceAspectRatio(sz);
    }

    Canvas * canvas = mosaicMaker->getMosaic()->getMosaicCanvas();
    canvas->setViewSize(sz);

    emit sig_reconstructView();
}

void page_canvas_settings::slot_mosaicDistortionChanged()
{
    if (pageBlocked()) return;

    Distortion d;
    d.set(static_cast<eDistortionMode>(distortMosaicCombo->currentData().toInt()),distortMosaicX->value(),distortMosaicY->value());

    Canvas * canvas = mosaicMaker->getMosaic()->getMosaicCanvas();
    canvas->setDistortion(d);

    // distortion bakes into the prototype map (Prototype::_buildPrototypeMap), so it must
    // reset prototypes/styles too, not just repaint with the old cached geometry
    Sys::render(RENDER_RESET_PROTOTYPES);
}

void page_canvas_settings::slot_tilingViewChanged(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    QSize sz = QSize(sizeTilingViewW->value(),sizeTilingViewH->value());
    if (Sys::lockViewAspectRatio)
    {
        sz = SystemView::enforceAspectRatio(sz);
    }

    TilingPtr tiling  = tilingMaker->getSelectedTiling();

    Canvas * canvas  = tiling->hdr().getTilingCanvas();
    canvas->setViewSize(sz);

    emit sig_reconstructView();
}

void page_canvas_settings::slot_tilingDistortionChanged()
{
    if (pageBlocked()) return;

    TilingPtr tiling = tilingMaker->getSelectedTiling();
    if (!tiling) return;

    Distortion d;
    d.set(static_cast<eDistortionMode>(distortTilingCombo->currentData().toInt()),distortTilingX->value(),distortTilingY->value());

    Canvas * canvas = tiling->hdr().getTilingCanvas();
    canvas->setDistortion(d);

    Sys::render(RENDER_RESET_PROTOTYPES);
}

void page_canvas_settings::slot_mosaicCanvasChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeMosaicCanvasW->value(),sizeMosaicCanvasH->value());

    Canvas * canvas = mosaicMaker->getMosaic()->getMosaicCanvas();
    canvas->setCanvasSize(sz);

    if (Sys::getActiveCanvasOwner() == ACTIVE_MOSAIC)
        viewControl->setTakerViewSize(sz);

    emit sig_reconstructView();
}

void page_canvas_settings::slot_mosaicDeltaChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeMosaicDeltaW->value(),sizeMosaicDeltaH->value());

    Canvas * canvas = mosaicMaker->getMosaic()->getMosaicCanvas();
    canvas->setDeltaCanvasSize(sz);

    emit sig_reconstructView();
}

void page_canvas_settings::slot_tilingCanvasChanged(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    QSize sz = QSize(sizeTilingCanvasW->value(),sizeTilingCanvasH->value());

    TilingPtr tiling  = tilingMaker->getSelectedTiling();

    Canvas * canvas  = tiling->hdr().getTilingCanvas();
    canvas->setCanvasSize(sz);

    if (Sys::getActiveCanvasOwner() == ACTIVE_TILING)
        viewControl->setTakerViewSize(sz);

    emit sig_reconstructView();
}

void page_canvas_settings::slot_tilingDeltaChanged(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    QSize sz = QSize(sizeTilingDeltaW->value(),sizeTilingDeltaH->value());

    TilingPtr tiling  = tilingMaker->getSelectedTiling();

    Canvas * canvas  = tiling->hdr().getTilingCanvas();
    canvas->setDeltaCanvasSize(sz);

    emit sig_reconstructView();
}

Canvas * page_canvas_settings::getMosaicOrDesignCanvas()
{
    if (viewControl->isEnabled(Layer::Type::LegacyDesign))
    {
        QVector<DesignPtr> & designs = designMaker->getActiveDesigns();
        if (designs.count())
        {
            DesignPtr dp = designs.first();
            return dp->getDesignCanvas();
        }
    }

    // drops thru
    return mosaicMaker->getMosaic()->getMosaicCanvas();
}

QColor page_canvas_settings::getMosaicOrDesignColor()
{
    if (viewControl->isEnabled(Layer::Type::LegacyDesign))
    {
        return config->getBkgdColor(Layer::Type::LegacyDesign);
    }
    else
    {
        return mosaicMaker->getMosaic()->getBkgdColor();
    }
}

void page_canvas_settings::setMosaicOrDesignColor(QColor color)
{
    if (viewControl->isEnabled(Layer::Type::LegacyDesign))
    {
        config->setBkgdColor(Layer::Type::LegacyDesign,color);
    }
    else
    {
        mosaicMaker->setBkgdColor(color);
    }
}


#if 0
void page_canvas_settings::setMosaicOrDesignCanvas(Canvas & ms)
{
    if (viewControl->isEnabled(VIEW_LEGACY))
    {
        QVector<DesignPtr> & designs = designMaker->getActiveDesigns();
        if (designs.count())
        {
            DesignPtr dp = designs.first();
            dp->setDesignCanvas(ms);
        }
    }

    // drops thru
    mosaicMaker->getMosaic()->setMosaicCanvas(ms);
}
#endif

void page_canvas_settings::singleton_changed_des(bool checked)
{
    FillData fd;
    if (!checked)
        fd.set(false,-3, 3, -3, 3);
    else
        fd.set(true,0,0,0,0);

    auto mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        Canvas * cs = mosaic->getMosaicCanvas();
        cs->setFillData(fd);
    }

    Sys::render(RENDER_RESET_PROTOTYPES);
}

void page_canvas_settings::singleton_changed_tile(bool checked)
{
    FillData fd;
    if (!checked)
        fd.set(false,-3, 3, -3, 3);
    else
        fd.set(true,0,0,0,0);

    TilingPtr tiling = tilingMaker->getSelectedTiling();
    if (!tiling) return;
    
    Canvas * cs = tiling->hdr().getTilingCanvas();
    cs->setFillData(fd);

    Sys::render(RENDER_RESET_PROTOTYPES);
}

void page_canvas_settings::backgroundColorDesignPick()
{
    QColor color = getMosaicOrDesignColor();

    AQColorDialog dlg(color,this);
    dlg.setCurrentColor(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        bkgdColorEdit[MOSAIC_SETTINGS]->setText(color.name(QColor::HexArgb));

        QVariant variant = color;
        QString colcode  = variant.toString();
        bkgdColorPatch[MOSAIC_SETTINGS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    }
    emit sig_reconstructView();
}

void page_canvas_settings::backgroundColorDesignChanged(const QString & str)
{
    if (pageBlocked()) return;

    QColor color = QColor(str);
    setMosaicOrDesignColor(color);

    emit sig_reconstructView();
}

void page_canvas_settings::backgroundColorDesignPickTiling()
{
    QColor color = config->getBkgdColor(Layer::Type::Tiling);

    AQColorDialog dlg(color,this);
    dlg.setCurrentColor(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        config->setBkgdColor(Layer::Type::Tiling,color);
        bkgdColorEdit[TILING_SETTINGS]->setText(color.name(QColor::HexArgb));

        QVariant variant = color;
        QString colcode  = variant.toString();
        bkgdColorPatch[TILING_SETTINGS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    }
    emit sig_reconstructView();
}

void page_canvas_settings::backgroundColorDesignChangedTiling(const QString & str)
{
    if (pageBlocked()) return;

    QColor color = QColor(str);
    config->setBkgdColor(Layer::Type::Tiling,color);

    emit sig_reconstructView();
}
#ifdef VARIABLE_BOUNDS
void page_canvas_settings::slot_boundsChanged()
{
    Bounds bounds(ds_left->value(), ds_top->value(), ds_width->value());
    viewControl->getCanvas().setBounds(bounds);

    QVector<Layer*> layers = Sys::sysview->getActiveLayers();
    for (auto layer : layers)
    {
        layer->forceLayerRecalc(true);
    }
}
#endif

void page_canvas_settings::displayFillData(const FillData & fd, eSettingsGroup group)
{
    blockSignals(true);

    int xMin,xMax,yMin,yMax;
    bool singleton;
    fd.get(singleton,xMin,xMax,yMin,yMax);

    chkSingle[group]->setChecked(singleton);

    if (!singleton)
    {
        xRepMin[group]->setDisabled(false);
        xRepMax[group]->setDisabled(false);
        yRepMin[group]->setDisabled(false);
        yRepMax[group]->setDisabled(false);

        xRepMin[group]->setValue(xMin);
        xRepMax[group]->setValue(xMax);
        yRepMin[group]->setValue(yMin);
        yRepMax[group]->setValue(yMax);
    }
    else
    {
        xRepMin[group]->setValue(0);
        xRepMax[group]->setValue(0);
        yRepMin[group]->setValue(0);
        yRepMax[group]->setValue(0);

        xRepMin[group]->setDisabled(true);
        xRepMax[group]->setDisabled(true);
        yRepMin[group]->setDisabled(true);
        yRepMax[group]->setDisabled(true);
    }

    blockSignals(false);
}

void page_canvas_settings::tallyAspectRatio()
{
    chkLockAspectRatio->setChecked(Sys::lockViewAspectRatio);
    if (Sys::lockViewAspectRatio)
    {
        QSize size = Sys::systemViewController->viewSize();
        Sys::aspectRatio = (qreal)size.width()/(qreal)size.height();        // calculation
        aspectRatioVal->setText(QString::number(Sys::aspectRatio,'f',8));
    }
    else
    {
        aspectRatioVal->setText("");
        Sys::aspectRatio = -1;
    }
}

void page_canvas_settings::sysViewChanged(QSize oldSize, QSize newSize)
{
    Q_UNUSED(oldSize);
    Q_UNUSED(newSize);
    tallyAspectRatio();
}
