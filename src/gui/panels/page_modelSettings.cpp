#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidgetItem>

#include "gui/panels/page_modelSettings.h"
#include "gui/panels/panel_misc.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view_controller.h"
#include "gui/widgets/layout_sliderset.h"
#include "legacy/design.h"
#include "legacy/design_maker.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/tilings/tiling.h"
#include "sys/enums/eviewtype.h"
#include "sys/geometry/transform.h"
#include "sys/qt/qtapplog.h"

using std::string;
using std::make_shared;

page_modelSettings::page_modelSettings(ControlPanel * apanel)  : panel_page(apanel,PAGE_MODEL_SETTINGS,"Model Settings")
{
    QBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(createMosaicSettings());
    hbox->addWidget(createTilingSettings());

    QVBoxLayout * abox = new QVBoxLayout;
    abox->addWidget(createCanvasStatus());
    abox->addSpacing(11);
    abox->addLayout(hbox);
    abox->addStretch();

    vbox->addLayout(abox);
    adjustSize();

    // connections
    connect(tilingMaker,   &TilingMaker::sig_tilingLoaded,      this,   &page_modelSettings::dummySetup);
    connect(mosaicMaker,   &MosaicMaker::sig_mosaicLoaded,      this,   &page_modelSettings::dummySetup);
    connect(designMaker,   &DesignMaker::sig_loadedDesign,      this,   &page_modelSettings::dummySetup);
}

QGroupBox *page_modelSettings::createTilingSettings()
{
    QLabel * l1         = new QLabel( "Canvas:  ");
    sizeTilingCanvasW   = new SpinSet("width  ",0,1,4096);
    sizeTilingCanvasH   = new SpinSet("height ",0,1,2160);

    QLabel * l2         = new QLabel( "View:    ");
    sizeTilingViewW     = new SpinSet("width  ",0,1,4096);
    sizeTilingViewH     = new SpinSet("height ",0,1,2160);

    QGridLayout * fillGrid = createFillDataRow(TILING_SETTINGS);

    QGridLayout * grid = new QGridLayout();

    grid->addWidget(l1,0,0);
    grid->addLayout(sizeTilingCanvasW,0,1);
    grid->addLayout(sizeTilingCanvasH,0,2);

    grid->addWidget(l2,1,0);
    grid->addLayout(sizeTilingViewW,1,1);
    grid->addLayout(sizeTilingViewH,1,2);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(grid);
    vbox->addStretch();
    vbox->addLayout(fillGrid);

    QGroupBox * box = new QGroupBox("Tiling Canvas Settings");
    box->setAlignment(Qt::AlignTop);
    box->setLayout(vbox);

    connect(sizeTilingCanvasW, &SpinSet::valueChanged, this, &page_modelSettings::slot_tilingCanvasChanged);
    connect(sizeTilingCanvasH, &SpinSet::valueChanged, this, &page_modelSettings::slot_tilingCanvasChanged);
    connect(sizeTilingViewW,   &SpinSet::valueChanged, this, &page_modelSettings::slot_tilingViewChanged);
    connect(sizeTilingViewH,   &SpinSet::valueChanged, this, &page_modelSettings::slot_tilingViewChanged);
    return box;
}

QGroupBox *page_modelSettings::createMosaicSettings()
{
    QLabel * l1         = new QLabel( "Canvas:  ");
    sizeMosaicCanvasW   = new SpinSet("width  ",0,1,4096);
    sizeMosaicCanvasH   = new SpinSet("height ",0,1,2160);

    QLabel * l2         = new QLabel( "View:    ");
    sizeMosaicViewW     = new SpinSet("width  ",0,1,4096);
    sizeMosaicViewH     = new SpinSet("height ",0,1,2160);

    bkColorEdit[MOSAIC_SETTINGS]    = new QLineEdit;
    bkgdColorPatch[MOSAIC_SETTINGS] = new ClickableLabel;
    bkgdColorPatch[MOSAIC_SETTINGS]->setMinimumWidth(145);

    QGridLayout * fillGrid = createFillDataRow(MOSAIC_SETTINGS);

    connect(sizeMosaicCanvasW,   &SpinSet::valueChanged,  this, &page_modelSettings::slot_mosaicCanvasChanged);
    connect(sizeMosaicCanvasH,   &SpinSet::valueChanged,  this, &page_modelSettings::slot_mosaicCanvasChanged);
    connect(sizeMosaicViewW,     &SpinSet::valueChanged,  this, &page_modelSettings::slot_mosaicViewChanged);
    connect(sizeMosaicViewH,     &SpinSet::valueChanged,  this, &page_modelSettings::slot_mosaicViewChanged);
    connect(bkgdColorPatch[MOSAIC_SETTINGS],&ClickableLabel::clicked,this, &page_modelSettings::backgroundColorDesignPick);
    connect(bkColorEdit[MOSAIC_SETTINGS],   &QLineEdit::textChanged, this, &page_modelSettings::backgroundColorDesignChanged);

    QGridLayout * grid = new QGridLayout();

    grid->addWidget(l1,0,0);
    grid->addLayout(sizeMosaicCanvasW,0,1);
    grid->addLayout(sizeMosaicCanvasH,0,2);

    grid->addWidget(l2,1,0);
    grid->addLayout(sizeMosaicViewW,1,1);
    grid->addLayout(sizeMosaicViewH,1,2);

    QGridLayout * grid2 = new QGridLayout();
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
    QLabel * actualLabel = new QLabel( "Actual:");
    actualSizeW          = new SpinSet("width  ",0,1,4096);
    actualSizeH          = new SpinSet("height ",0,1,2160);

    actualSizeW->setReadOnly(true);
    actualSizeH->setReadOnly(true);

    QLabel * canvasLabel = new QLabel( "Canvas:");
    sizeViewCanvasW      = new SpinSet("width  ",0,1,4096);
    sizeViewCanvasH      = new SpinSet("height ",0,1,2160);

    QLabel * viewLabel   = new QLabel( "View:  ");
    sizeViewViewW        = new SpinSet("width  ",0,1,4096);
    sizeViewViewH        = new SpinSet("height ",0,1,2160);

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
    l_layer  = new QLabel();

    bkColorEdit[CANVAS]    = new QLineEdit;
    bkgdColorPatch[CANVAS] = new ClickableLabel;
    bkgdColorPatch[CANVAS]->setMinimumWidth(145);

    //sizeW[VIEW_STATUS]->setReadOnly(true);
    //sizeH[VIEW_STATUS]->setReadOnly(true);
    bkColorEdit[CANVAS]->setReadOnly(true);

    connect(sizeViewCanvasW,    &SpinSet::valueChanged,       this, &page_modelSettings::slot_viewCanvasChanged);
    connect(sizeViewCanvasH,    &SpinSet::valueChanged,       this, &page_modelSettings::slot_viewCanvasChanged);
    connect(sizeViewViewW,      &SpinSet::valueChanged,       this, &page_modelSettings::slot_viewViewChanged);
    connect(sizeViewViewH,      &SpinSet::valueChanged,       this, &page_modelSettings::slot_viewViewChanged);

#ifdef VARIABLE_BOUNDS
    connect(ds_left,               &DoubleSpinSet::valueChanged, this, &page_modelSettings::slot_boundsChanged);
    connect(ds_top,                &DoubleSpinSet::valueChanged, this, &page_modelSettings::slot_boundsChanged);
    connect(ds_width,              &DoubleSpinSet::valueChanged, this, &page_modelSettings::slot_boundsChanged);
#endif

    QGridLayout * grid = new QGridLayout;

    grid->addWidget(actualLabel,0,0);
    grid->addLayout(actualSizeW,0,1);
    grid->addLayout(actualSizeH,0,2);

    grid->addWidget(canvasLabel,1,0);
    grid->addLayout(sizeViewCanvasW,1,1);
    grid->addLayout(sizeViewCanvasH,1,2);

    grid->addWidget(viewLabel,2,0);
    grid->addLayout(sizeViewViewW,2,1);
    grid->addLayout(sizeViewViewH,2,2);

    QHBoxLayout * hbox1 = new QHBoxLayout;
    hbox1->addLayout(grid);
    hbox1->addStretch();

    QHBoxLayout * hbox3 = new QHBoxLayout;
    hbox3->addWidget(bkColorEdit[CANVAS]);
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

    viewBox = new QGroupBox("View Status");
    viewBox->setLayout(vbox);

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
        const CanvasSettings & tset = tiling->hdr().getCanvasSettings();
        QSize csize  = tset.getCanvasSize();
        QSize vsize  = tset.getViewSize();

        sizeTilingCanvasW->setValue(csize.width());
        sizeTilingCanvasH->setValue(csize.height());
        sizeTilingViewW->setValue(vsize.width());
        sizeTilingViewH->setValue(vsize.height());

        const FillData & fd = tset.getFillData();
        displayFillData(fd,TILING_SETTINGS);
    }

    // mosaic/design settings;
    const CanvasSettings & mset = getMosaicOrDesignModelSettings();

    // size
    QSizeF vsize = mset.getViewSize();
    QSizeF csize = mset.getCanvasSize();

    sizeMosaicCanvasW->setValue(csize.width());
    sizeMosaicCanvasH->setValue(csize.height());
    sizeMosaicViewW->setValue(vsize.width());
    sizeMosaicViewH->setValue(vsize.height());

    // background color
    QColor qc = mset.getBackgroundColor();
    bkColorEdit[MOSAIC_SETTINGS]->setText(qc.name(QColor::HexArgb));
    QVariant variant = qc;
    QString colcode  = variant.toString();
    bkgdColorPatch[MOSAIC_SETTINGS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    // repeats
    const FillData & fd = mset.getFillData();
    displayFillData(fd,MOSAIC_SETTINGS);

    // Canvas status
    Canvas & canvas  = viewControl->getCanvas();
    csize            = canvas.getCanvasSize();
    vsize            = canvas.getViewSize();
    QSize size       = Sys::viewController->viewSize();

    sizeViewCanvasW->setValue(csize.width());
    sizeViewCanvasH->setValue(csize.height());
    sizeViewViewW->setValue(vsize.width());
    sizeViewViewH->setValue(vsize.height());
    actualSizeW->setValue(size.width());
    actualSizeH->setValue(size.height());

    // background color
    qc = Sys::viewController->getViewBackgroundColor();
    QVariant qv = qc;
    colcode  = qv.toString();
    bkColorEdit[CANVAS]->setText(qc.name(QColor::HexArgb));
    bkgdColorPatch[CANVAS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    // frame settings
    LayerPtr layer     = viewControl->getSelectedPrimaryLayer();
    if (layer)
    {
        eViewType viewType = layer->viewType();
        viewBox->setTitle(QString("View Status :  %1").arg(sViewerType[viewType]));

#ifdef VARIABLE_BOUNDS
        auto bounds = canvas.getBounds();
        ds_left->setValue(bounds.left);
        ds_top->setValue(bounds.top);
        ds_width->setValue(bounds.width);
#endif

        l_xform->setText( "<pre>Canvas " + Transform::info(layer->getCanvasTransform()) + "</pre>");
        l_canvas->setText("<pre>Model  " + Transform::info(layer->getModelTransform())  + "</pre>");
        l_layer->setText( "<pre>Layer  " + Transform::info(layer->getLayerTransform())  + "</pre>");
    }

    qtAppLog::getInstance()->suspend(false);
}

void page_modelSettings::onEnter()
{
    dummySetup();
}

void page_modelSettings::dummySetup()
{}

void page_modelSettings::slot_tilingCanvasChanged(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    QSize sz = QSize(sizeTilingCanvasW->value(),sizeTilingCanvasH->value());

    TilingPtr tiling  = tilingMaker->getSelected();

    CanvasSettings cs  = tiling->hdr().getCanvasSettings();
    cs.setCanvasSize(sz);
    tiling->hdr().setCanvasSettings(cs);

    emit sig_reconstructView();
}

void page_modelSettings::slot_tilingViewChanged(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    QSize sz = QSize(sizeTilingViewW->value(),sizeTilingViewH->value());

    TilingPtr tiling  = tilingMaker->getSelected();

    CanvasSettings cs  = tiling->hdr().getCanvasSettings();
    cs.setViewSize(sz);
    tiling->hdr().setCanvasSettings(cs);

    emit sig_reconstructView();
}

void page_modelSettings::slot_mosaicCanvasChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeMosaicCanvasW->value(),sizeMosaicCanvasH->value());
    
    CanvasSettings  & ms = mosaicMaker->getCanvasSettings();
    ms.setCanvasSize(sz);

    emit sig_reconstructView();
}

void page_modelSettings::slot_mosaicViewChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeMosaicViewW->value(),sizeMosaicViewH->value());

    CanvasSettings  & ms = mosaicMaker->getCanvasSettings();
    ms.setViewSize(sz);

    emit sig_reconstructView();
}

void page_modelSettings::slot_viewCanvasChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeViewCanvasW->value(),sizeViewCanvasH->value());

    Canvas & canvas = viewControl->getCanvas();
    canvas.setCanvasSize(sz);

#if 0
    if (config->splitScreen)
        viewControl->setFixedSize(sz);
    else
        viewControl->setSize(sz);
#endif
    emit sig_reconstructView();
}

void page_modelSettings::slot_viewViewChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeViewViewW->value(),sizeViewViewH->value());
#if 0
    Canvas & canvas = viewControl->getCanvas();
    canvas.setViewSize(sz);
#endif
#if 1
    if (config->splitScreen)
        viewControl->setFixedSize(sz);
    else
        viewControl->setSize(sz);
#endif
    emit sig_reconstructView();
}

CanvasSettings & page_modelSettings::getMosaicOrDesignModelSettings()
{
    if (viewControl->isEnabled(VIEW_LEGACY))
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
    if (viewControl->isEnabled(VIEW_LEGACY))
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

    Sys::render(RENDER_RESET_PROTOTYPES);
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

    Sys::render(RENDER_RESET_PROTOTYPES);
}

void page_modelSettings::slot_set_repsTiling(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    FillData fd;
    fd.set(chkSingle[TILING_SETTINGS]->isChecked(),xRepMin[TILING_SETTINGS]->value(), xRepMax[TILING_SETTINGS]->value(), yRepMin[TILING_SETTINGS]->value(), yRepMax[TILING_SETTINGS]->value());

    TilingPtr tiling = tilingMaker->getSelected();
    if (!tiling) return;
    
    CanvasSettings cs = tiling->hdr().getCanvasSettings();
    cs.setFillData(fd);
    tiling->hdr().setCanvasSettings(cs);

    if (viewControl->isEnabled(VIEW_TILING_MAKER)
     || viewControl->isEnabled(VIEW_TILING)
     || viewControl->isEnabled(VIEW_MAP_EDITOR))
        emit sig_reconstructView();
    else
        Sys::render(RENDER_RESET_PROTOTYPES);
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
    
    CanvasSettings cs = tiling->hdr().getCanvasSettings();
    cs.setFillData(fd);
    tiling->hdr().setCanvasSettings(cs);

    Sys::render(RENDER_RESET_PROTOTYPES);

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

    emit sig_reconstructView();
}

#ifdef VARIABLE_BOUNDS
void page_modelSettings::slot_boundsChanged()
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



