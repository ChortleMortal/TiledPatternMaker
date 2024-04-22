#include <QGroupBox>
#include <QGridLayout>
#include <QTreeWidgetItem>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHeaderView>

#include "geometry/transform.h"
#include "legacy/design.h"
#include "legacy/design_maker.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/prototype_maker/prototype.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/qtapplog.h"
#include "mosaic/mosaic.h"
#include "panels/controlpanel.h"
#include "panels/page_modelSettings.h"
#include "panels/panel_misc.h"
#include "settings/configuration.h"
#include "tile/tiling.h"
#include "viewers/view_controller.h"
#include "widgets/layout_sliderset.h"
#include "enums/eviewtype.h"

using std::string;
using std::make_shared;

page_modelSettings::page_modelSettings(ControlPanel * apanel)  : panel_page(apanel,PAGE_MODEL_SETTINGS,"Model Settings")
{
    QGridLayout * pgrid  = new QGridLayout();
    QGroupBox * box;

    int row = 0;
    box = createTilingSettings();
    pgrid->addWidget(box,row,0);

    frameBox = createCanvasStatus();
    pgrid->addWidget(frameBox,row,1);

    row++;
    box = createMosaicSettings();
    pgrid->addWidget(box,row,0);

    box = createViewStatus();
    pgrid->addWidget(box,row,1);


    vbox->addLayout(pgrid);
    vbox->addStretch();
    adjustSize();

    // connections
    connect(tilingMaker,   &TilingMaker::sig_tilingLoaded,      this,   &page_modelSettings::dummySetup);
    connect(mosaicMaker,   &MosaicMaker::sig_mosaicLoaded,      this,   &page_modelSettings::dummySetup);
    connect(designMaker,   &DesignMaker::sig_loadedDesign,      this,   &page_modelSettings::dummySetup);
}

QGroupBox *page_modelSettings::createTilingSettings()
{
    QLabel * l1 = new QLabel("View:     ");
    sizeW[TILING_SETTINGS] = new SpinSet("width",0,1,4096);
    sizeH[TILING_SETTINGS] = new SpinSet("height",0,1,2160);

    QGridLayout * fillGrid = createFillDataRow(TILING_SETTINGS);

    QGridLayout * grid = new QGridLayout();
    grid->addWidget(l1,0,0);
    grid->addLayout(sizeW[TILING_SETTINGS],0,1);
    grid->addLayout(sizeH[TILING_SETTINGS],0,2);

    QVBoxLayout * vb = new QVBoxLayout;
    vb->addLayout(grid);
    vb->addLayout(fillGrid);

    QGroupBox * box = new QGroupBox("Tiling Canvas Settings");
    box->setAlignment(Qt::AlignTop);
    box->setLayout(vb);

    connect(sizeW[TILING_SETTINGS], &SpinSet::valueChanged, this, &page_modelSettings::slot_tilingSizeChanged);
    connect(sizeH[TILING_SETTINGS], &SpinSet::valueChanged, this, &page_modelSettings::slot_tilingSizeChanged);

    return box;
}

QGroupBox *page_modelSettings::createMosaicSettings()
{
    QLabel * l1 = new QLabel("View:     ");
    sizeW[MOSAIC_SETTINGS]          = new SpinSet("width",0,1,4096);
    sizeH[MOSAIC_SETTINGS]          = new SpinSet("height",0,1,2160);
    startEditX[MOSAIC_SETTINGS]     = new DoubleSpinSet("start-X",0,-4096,4096);
    startEditY[MOSAIC_SETTINGS]     = new DoubleSpinSet("start-y",0,-2160,2160);
    bkColorEdit[MOSAIC_SETTINGS]    = new QLineEdit;
    bkgdColorPatch[MOSAIC_SETTINGS] = new ClickableLabel;

    QGridLayout * fillGrid = createFillDataRow(MOSAIC_SETTINGS);
    
    connect(sizeW[MOSAIC_SETTINGS],         &SpinSet::valueChanged,  this, &page_modelSettings::slot_canvasSizeChanged);
    connect(sizeH[MOSAIC_SETTINGS],         &SpinSet::valueChanged,  this, &page_modelSettings::slot_canvasSizeChanged);
    connect(bkgdColorPatch[MOSAIC_SETTINGS],&ClickableLabel::clicked,this, &page_modelSettings::backgroundColorDesignPick);
    connect(bkColorEdit[MOSAIC_SETTINGS],   &QLineEdit::textChanged, this, &page_modelSettings::backgroundColorDesignChanged);

    QGridLayout * grid = new QGridLayout();
    grid->addWidget(l1,0,0);
    grid->addLayout(sizeW[MOSAIC_SETTINGS],0,1);
    grid->addLayout(sizeH[MOSAIC_SETTINGS],0,2);

    QGridLayout * grid2 = new QGridLayout();
    grid2->addLayout(startEditX[MOSAIC_SETTINGS],0,0);
    grid2->addLayout(startEditY[MOSAIC_SETTINGS],0,1);
    grid2->addWidget(bkColorEdit[MOSAIC_SETTINGS],1,0);
    grid2->addWidget(bkgdColorPatch[MOSAIC_SETTINGS],1,1);
    grid2->addLayout(fillGrid,2,0,2,2);

    QVBoxLayout * vb = new QVBoxLayout;
    vb->addLayout(grid);
    vb->addLayout(grid2);

    QGroupBox * box = new QGroupBox("Mosaic Canvas Settings");
    box->setLayout(vb);
    return box;
}

QGroupBox *page_modelSettings::createCanvasStatus()
{
    sizeW[CANVAS] = new SpinSet("width",0,1,4096);
    sizeH[CANVAS] = new SpinSet("height",0,1,2160);

    ds_left  = new DoubleSpinSet("left",0,-3480,3480);
    ds_top   = new DoubleSpinSet("top",0,-2160,2160);
    ds_width = new DoubleSpinSet("width",1,1,100);

    QHBoxLayout * boundsbox = new QHBoxLayout;
    boundsbox->addLayout(ds_left);
    boundsbox->addLayout(ds_top);
    boundsbox->addLayout(ds_width);

    l_xform   = new QLabel();
    l_canvas  = new QLabel();
    l_layer  = new QLabel();

    connect(sizeW[CANVAS],         &SpinSet::valueChanged,       this, &page_modelSettings::slot_viewerSizeChanged);
    connect(sizeH[CANVAS],         &SpinSet::valueChanged,       this, &page_modelSettings::slot_viewerSizeChanged);
    connect(ds_left,               &DoubleSpinSet::valueChanged, this, &page_modelSettings::slot_boundsChanged);
    connect(ds_top,                &DoubleSpinSet::valueChanged, this, &page_modelSettings::slot_boundsChanged);
    connect(ds_width,              &DoubleSpinSet::valueChanged, this, &page_modelSettings::slot_boundsChanged);

    QLabel * l1 = new QLabel("Canvas:   ");

    int row = 0;
    QGridLayout * grid = new QGridLayout();
    grid->addWidget(l1,row++,0);
    grid->addLayout(sizeW[CANVAS],0,1);
    grid->addLayout(sizeH[CANVAS],0,2);

    grid->addLayout(boundsbox,row++,0,1,3);
    grid->addWidget(l_xform,row++,0,1,3);
    grid->addWidget(l_canvas,row++,0,1,3);
    grid->addWidget(l_layer,row++,0,1,3);

    QVBoxLayout * vb = new QVBoxLayout;
    vb->addLayout(grid);
    vb->addStretch();

    canvasBox = new QGroupBox("Canvas Status");
    canvasBox->setLayout(vb);

    return canvasBox;
}

QGroupBox * page_modelSettings::createViewStatus()
{
    sizeW[VIEW_STATUS]          = new SpinSet("width",0,1,4096);
    sizeH[VIEW_STATUS]          = new SpinSet("height",0,1,2160);
    startEditX[VIEW_STATUS]     = new DoubleSpinSet("start-X",0,-4096,4096);
    startEditY[VIEW_STATUS]     = new DoubleSpinSet("start-y",0,-2160,2160);
    bkColorEdit[VIEW_STATUS]    = new QLineEdit;
    bkgdColorPatch[VIEW_STATUS] = new ClickableLabel;
    
    connect(sizeW[VIEW_STATUS], &SpinSet::valueChanged, this, &page_modelSettings::slot_windowSizeChanged);
    connect(sizeH[VIEW_STATUS], &SpinSet::valueChanged, this, &page_modelSettings::slot_windowSizeChanged);

    //sizeW[VIEW_STATUS]->setReadOnly(true);
    //sizeH[VIEW_STATUS]->setReadOnly(true);
    startEditX[VIEW_STATUS]->setReadOnly(true);
    startEditY[VIEW_STATUS]->setReadOnly(true);
    bkColorEdit[VIEW_STATUS]->setReadOnly(true);

    QGridLayout * fillGrid = createFillDataRow(VIEW_STATUS);

    QGridLayout * grid = new QGridLayout();
    grid->addLayout(sizeW[VIEW_STATUS],0,0);
    grid->addLayout(sizeH[VIEW_STATUS],0,1);
    grid->addLayout(startEditX[VIEW_STATUS],1,0);
    grid->addLayout(startEditY[VIEW_STATUS],1,1);
    grid->addWidget(bkColorEdit[VIEW_STATUS],2,0);
    grid->addWidget(bkgdColorPatch[VIEW_STATUS],2,1);
    grid->addLayout(fillGrid,3,0,2,2);

    viewBox = new QGroupBox("View Status");
    viewBox->setLayout(grid);
    return viewBox;
}

QGridLayout * page_modelSettings::createFillDataRow(eSettingsGroup group)
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
        connect(chkSingle[group], &QCheckBox::clicked, this, &page_modelSettings::singleton_changed_des);

        connect(xRepMin[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsDesign(int)));
        connect(xRepMax[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsDesign(int)));
        connect(yRepMin[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsDesign(int)));
        connect(yRepMax[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsDesign(int)));
    }
    else if (group == TILING_SETTINGS)
    {
        connect(chkSingle[group], &QCheckBox::clicked, this, &page_modelSettings::singleton_changed_tile);
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


void  page_modelSettings::onRefresh()
{
    //if (!refresh) return;

    qtAppLog::getInstance()->suspend(true);

    // tiling settings
    TilingPtr tiling = tilingMaker->getSelected();
    if (tiling)
    {
        qDebug() << "Tiling" << tiling->getTitle();
        QSize size  = tiling->getData().getSettings().getViewSize();
        sizeW[TILING_SETTINGS]->setValue(size.width());
        sizeH[TILING_SETTINGS]->setValue(size.height());

        const FillData & fd = tiling->getCanvasSettings().getFillData();
        displayFillData(fd,TILING_SETTINGS);
    }

    // mosaic/design settings;
    const CanvasSettings & mosCanvasSettings = getMosaicOrDesignModelSettings();

    // size
    QSizeF sz = mosCanvasSettings.getViewSize();
    sizeW[MOSAIC_SETTINGS]->setValue(sz.width());
    sizeH[MOSAIC_SETTINGS]->setValue(sz.height());

    // background color
    QColor qc = mosCanvasSettings.getBackgroundColor();
    bkColorEdit[MOSAIC_SETTINGS]->setText(qc.name(QColor::HexArgb));
    QVariant variant = qc;
    QString colcode  = variant.toString();
    bkgdColorPatch[MOSAIC_SETTINGS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    // start tile
    QPointF  pt = mosCanvasSettings.getStartTile();
    startEditX[MOSAIC_SETTINGS]->setValue(pt.x());
    startEditY[MOSAIC_SETTINGS]->setValue(pt.y());

    // repeats
    const FillData & fd = mosCanvasSettings.getFillData();
    displayFillData(fd,MOSAIC_SETTINGS);

    // View Status
    // size
    QSize size  = view->size();
    sizeW[VIEW_STATUS]->setValue(size.width());
    sizeH[VIEW_STATUS]->setValue(size.height());

    // background color
    qc = view->getViewBackgroundColor();
    bkColorEdit[VIEW_STATUS]->setText(qc.name(QColor::HexArgb));
    QVariant qv = qc;
    colcode  = qv.toString();
    bkgdColorPatch[VIEW_STATUS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    // fill data
    const FillData & fd2 = viewControl->getCanvas().getFillData();
    displayFillData(fd2,VIEW_STATUS);

    // frame settings
    eViewType viewType = viewControl->getMostRecent();

    viewBox->setTitle(QString("View Status : %1").arg(sViewerType[viewType]));

    Canvas & canvas  = viewControl->getCanvas();

    QSizeF canvasSize  = canvas.getSize();

    sizeW[CANVAS]->setValue(canvasSize.width());
    sizeH[CANVAS]->setValue(canvasSize.height());

    auto bounds = canvas.getBounds();
    ds_left->setValue(bounds.left);
    ds_top->setValue(bounds.top);
    ds_width->setValue(bounds.width);
    auto lc = view->getActiveLayer(viewType);
    if (lc)
    {
        l_xform->setText( "canvas " + Transform::info(lc->getCanvasTransform()));
        l_canvas->setText("model  " + Transform::info(lc->getModelTransform()));
        l_layer->setText( "layer  " + Transform::info(lc->getLayerTransform()));
    }
    else
    {
        l_canvas->clear();
        l_xform->clear();
        l_layer->clear();
    }

    qtAppLog::getInstance()->suspend(false);
}

void page_modelSettings::onEnter()
{
    dummySetup();
}

void page_modelSettings::dummySetup()
{}

void page_modelSettings::slot_canvasSizeChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeW[MOSAIC_SETTINGS]->value(),sizeH[MOSAIC_SETTINGS]->value());
    
    CanvasSettings  & ms = mosaicMaker->getCanvasSettings();
    ms.setViewSize(sz);

    emit sig_refreshView();
}

void page_modelSettings::slot_viewerSizeChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeW[CANVAS]->value(),sizeH[CANVAS]->value());

    Canvas & canvas = viewControl->getCanvas();
    canvas.initCanvasSize(sz);
    emit sig_refreshView();
}

void page_modelSettings::slot_windowSizeChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeW[VIEW_STATUS]->value(),sizeH[VIEW_STATUS]->value());
    view->setSize(sz);   // direct
    emit sig_refreshView();
}

CanvasSettings & page_modelSettings::getMosaicOrDesignModelSettings()
{
    if (viewControl->isEnabled(VIEW_DESIGN))
    {
        QVector<DesignPtr> & designs = designMaker->getActiveDesigns();
        if (designs.count())
        {
            DesignPtr dp = designs.first();
            return dp->getDesignInfo();
        }
    }

    // drops thru
    return mosaicMaker->getCanvasSettings();
}

void page_modelSettings::setMosaicOrDesignModelSettings(CanvasSettings & ms)
{
    if (viewControl->isEnabled(VIEW_DESIGN))
    {
        QVector<DesignPtr> & designs = designMaker->getActiveDesigns();
        if (designs.count())
        {
            DesignPtr dp = designs.first();
            dp->setDesignInfo(ms);
        }
    }

    // drops thru
    mosaicMaker->setCanvasSettings(ms);
}

void page_modelSettings::slot_set_repsDesign(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    FillData fd;
    fd.set(chkSingle[MOSAIC_SETTINGS]->isChecked(),xRepMin[MOSAIC_SETTINGS]->value(), xRepMax[MOSAIC_SETTINGS]->value(), yRepMin[MOSAIC_SETTINGS]->value(), yRepMax[MOSAIC_SETTINGS]->value());

    auto mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        CanvasSettings & cs = mosaic->getCanvasSettings();
        cs.setFillData(fd);
    }

    viewControl->getCanvas().setFillData(fd);

    emit sig_render();
}

void page_modelSettings::singleton_changed_des(bool checked)
{
    FillData fd;
    if (!checked)
        fd.set(false,-3, 3, -3, 3);
    else
        fd.set(true,0,0,0,0);

    auto mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        CanvasSettings & cs = mosaic->getCanvasSettings();
        cs.setFillData(fd);
    }

    viewControl->getCanvas().setFillData(fd);

    emit sig_render();
}

void page_modelSettings::slot_set_repsTiling(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    FillData fd;
    fd.set(chkSingle[TILING_SETTINGS]->isChecked(),xRepMin[TILING_SETTINGS]->value(), xRepMax[TILING_SETTINGS]->value(), yRepMin[TILING_SETTINGS]->value(), yRepMax[TILING_SETTINGS]->value());

    TilingPtr tiling = tilingMaker->getSelected();
    if (!tiling) return;
    
    CanvasSettings cs = tiling->getCanvasSettings();
    cs.setFillData(fd);
    tiling->setCanvasSettings(cs);

    viewControl->getCanvas().setFillData(fd);

    if (viewControl->isEnabled(VIEW_TILING_MAKER)
     || viewControl->isEnabled(VIEW_TILING)
     || viewControl->isEnabled(VIEW_MAP_EDITOR))
        emit sig_refreshView();
    else
        emit sig_render();
}

void page_modelSettings::singleton_changed_tile(bool checked)
{
    FillData fd;
    if (!checked)
        fd.set(false,-3, 3, -3, 3);
    else
        fd.set(true,0,0,0,0);

    TilingPtr tiling = tilingMaker->getSelected();
    if (!tiling) return;
    
    CanvasSettings cs = tiling->getCanvasSettings();
    cs.setFillData(fd);
    tiling->setCanvasSettings(cs);

    viewControl->getCanvas().setFillData(fd);

    emit sig_render();

}
void page_modelSettings::backgroundColorDesignPick()
{
    const CanvasSettings & settings = getMosaicOrDesignModelSettings();
    QColor color = settings.getBackgroundColor();

    AQColorDialog dlg(color,this);
    dlg.setCurrentColor(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        bkColorEdit[MOSAIC_SETTINGS]->setText(color.name(QColor::HexArgb));

        QVariant variant = color;
        QString colcode  = variant.toString();
        bkgdColorPatch[MOSAIC_SETTINGS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    }
}

void page_modelSettings::backgroundColorDesignChanged(const QString & str)
{
    if (pageBlocked()) return;

    QColor color = QColor(str);
    CanvasSettings & ms = mosaicMaker->getCanvasSettings();
    ms.setBackgroundColor(color);

    emit sig_refreshView();
}

void page_modelSettings::slot_tilingSizeChanged(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    TilingPtr tiling  = tilingMaker->getSelected();
    CanvasSettings ms  = tiling->getCanvasSettings();
    ms.setViewSize(QSize(sizeW[TILING_SETTINGS]->value(),sizeH[TILING_SETTINGS]->value()));
    tiling->setCanvasSettings(ms);
    emit sig_refreshView();
}

void page_modelSettings::slot_boundsChanged()
{
    Bounds bounds(ds_left->value(), ds_top->value(), ds_width->value());
    viewControl->getCanvas().setBounds(bounds);

    QVector<Layer*> layers = view->getActiveLayers();
    for (auto layer : layers)
    {
        layer->forceLayerRecalc(true);
    }
}

void page_modelSettings::displayFillData(const FillData & fd, eSettingsGroup group)
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



