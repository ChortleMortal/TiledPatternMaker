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
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/qtapplog.h"
#include "misc/utilities.h"
#include "makers/prototype_maker/prototype.h"
#include "panels/panel_misc.h"
#include "panels/page_modelSettings.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"
#include "tile/tiling.h"
#include "viewers/viewcontrol.h"
#include "widgets/layout_sliderset.h"

using std::string;
using std::make_shared;

page_modelSettings::page_modelSettings(ControlPanel * apanel)  : panel_page(apanel,"Model Settings")
{
    QGridLayout * pgrid  = new QGridLayout();
    QGroupBox * box;

    int row = 0;
    box = createTilingSettings();
    pgrid->addWidget(box,row,0);

    if (config->insightMode)
    {
        QCheckBox* chkShowFrameInfo  = new QCheckBox("Show Frame Info");
        chkShowFrameInfo->setLayoutDirection(Qt::RightToLeft);
        chkShowFrameInfo->setChecked(config->cs_showFrameSettings);

        connect(chkShowFrameInfo, &QCheckBox::clicked, this, &page_modelSettings::slot_showFrameInfoChanged);

        QHBoxLayout * hbox = new QHBoxLayout;
        hbox->addWidget(chkShowFrameInfo);
        hbox->addStretch();
        QVBoxLayout * vbox = new QVBoxLayout;
        vbox->addLayout(hbox);
        vbox->addStretch();

        pgrid->addLayout(vbox,row,1);
    }
    else
    {
        config->cs_showFrameSettings = false;
    }

    row++;
    box = createDesignSettings();
    pgrid->addWidget(box,row,0);

    box = createViewStatus();
    pgrid->addWidget(box,row,1);

    row++;
    if (config->insightMode)
    {
        frameTable = new AQTableWidget();
        QStringList qslH;
        qslH << "Sizes" << "Crop" << "Zoom";
        frameTable->setColumnCount(3);
        frameTable->setHorizontalHeaderLabels(qslH);
        frameTable->horizontalHeader()->setVisible(true);
        frameTable->verticalHeader()->setVisible(false);

        pgrid->addWidget(frameTable,row,0);

        frameBox = createFrameSettings();
        pgrid->addWidget(frameBox,row,1);
    }
    else
    {
        frameTable = nullptr;
    }

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
    sizeW[TILING_SETTINGS] = new SpinSet("width",0,1,4096);
    sizeH[TILING_SETTINGS] = new SpinSet("height",0,1,2160);

    QGridLayout * fillGrid = createFillDataRow(TILING_SETTINGS);

    QGridLayout * grid = new QGridLayout();
    grid->addLayout(sizeW[TILING_SETTINGS],0,0);
    grid->addLayout(sizeH[TILING_SETTINGS],0,1);
    grid->addLayout(fillGrid,1,0,2,2);

    QGroupBox * box = new QGroupBox("Tiling Settings");
    box->setAlignment(Qt::AlignTop);
    box->setLayout(grid);

    connect(sizeW[TILING_SETTINGS], &SpinSet::valueChanged, this, &page_modelSettings::slot_tilingSizeChanged);
    connect(sizeH[TILING_SETTINGS], &SpinSet::valueChanged, this, &page_modelSettings::slot_tilingSizeChanged);

    return box;
}

QGroupBox *page_modelSettings::createDesignSettings()
{
    sizeW[DESIGN_SETTINGS]      = new SpinSet("width",0,1,4096);
    sizeH[DESIGN_SETTINGS]      = new SpinSet("height",0,1,2160);
    startEditX[DESIGN_SETTINGS]     = new DoubleSpinSet("start-X",0,-4096,4096);
    startEditY[DESIGN_SETTINGS]     = new DoubleSpinSet("start-y",0,-2160,2160);
    bkColorEdit[DESIGN_SETTINGS]    = new QLineEdit;
    bkgdColorPatch[DESIGN_SETTINGS] = new ClickableLabel;

    QGridLayout * fillGrid = createFillDataRow(DESIGN_SETTINGS);

    connect(sizeW[DESIGN_SETTINGS],      &SpinSet::valueChanged,  this, &page_modelSettings::designSizeChanged);
    connect(sizeH[DESIGN_SETTINGS],      &SpinSet::valueChanged,  this, &page_modelSettings::designSizeChanged);
    connect(bkgdColorPatch[DESIGN_SETTINGS], &ClickableLabel::clicked,this, &page_modelSettings::backgroundColorDesignPick);
    connect(bkColorEdit[DESIGN_SETTINGS],    &QLineEdit::textChanged, this, &page_modelSettings::backgroundColorDesignChanged);

    QGridLayout * grid = new QGridLayout();
    grid->addLayout(sizeW[DESIGN_SETTINGS],0,0);
    grid->addLayout(sizeH[DESIGN_SETTINGS],0,1);
    grid->addLayout(startEditX[DESIGN_SETTINGS],1,0);
    grid->addLayout(startEditY[DESIGN_SETTINGS],1,1);
    grid->addWidget(bkColorEdit[DESIGN_SETTINGS],2,0);
    grid->addWidget(bkgdColorPatch[DESIGN_SETTINGS],2,1);
    grid->addLayout(fillGrid,3,0,2,2);

    QGroupBox * box = new QGroupBox("Mosaic Settings");
    box->setLayout(grid);
    return box;
}

QGroupBox *page_modelSettings::createFrameSettings()
{
    sizeW[FRAME_SETTINGS] = new SpinSet("width",0,1,4096);
    sizeH[FRAME_SETTINGS] = new SpinSet("height",0,1,2160);

    sizeW2 = new SpinSet("width",0,1,4096);
    sizeH2 = new SpinSet("height",0,1,2160);

    ds_left  = new DoubleSpinSet("left",0,-3480,3480);
    ds_top   = new DoubleSpinSet("top",0,-2160,2160);
    ds_width = new DoubleSpinSet("width",1,1,100);

    QHBoxLayout * boundsbox = new QHBoxLayout;
    boundsbox->addLayout(ds_left);
    boundsbox->addLayout(ds_top);
    boundsbox->addLayout(ds_width);

    l_xform   = new QLabel();

    connect(sizeW[FRAME_SETTINGS], &SpinSet::valueChanged,           this, &page_modelSettings::cropSizeChanged);
    connect(sizeH[FRAME_SETTINGS], &SpinSet::valueChanged,           this, &page_modelSettings::cropSizeChanged);
    connect(ds_left,               &DoubleSpinSet::valueChanged, this, &page_modelSettings::slot_boundsChanged);
    connect(ds_top,                &DoubleSpinSet::valueChanged, this, &page_modelSettings::slot_boundsChanged);
    connect(ds_width,              &DoubleSpinSet::valueChanged, this, &page_modelSettings::slot_boundsChanged);

    QLabel * l1 = new QLabel("Crop: ");
    QLabel * l2 = new QLabel("Zoom: ");

    int row = 0;
    QGridLayout * grid = new QGridLayout();
    grid->addWidget(l1,row++,0);
    grid->addLayout(sizeW[FRAME_SETTINGS],0,1);
    grid->addLayout(sizeH[FRAME_SETTINGS],0,2);

    grid->addWidget(l2,row,0);
    grid->addLayout(sizeW2,row,1);
    grid->addLayout(sizeH2,row++,2);
    grid->addLayout(boundsbox,row++,0,1,3);
    grid->addWidget(l_xform,row++,0,1,3);

    QVBoxLayout * vb = new QVBoxLayout;
    vb->addLayout(grid);
    vb->addStretch();

    viewBox = new QGroupBox("Frame Size");
    if (config->darkTheme)
        viewBox->setStyleSheet("QGroupBox:title {color: yellow;}");
    else
        viewBox->setStyleSheet("QGroupBox:title {color: red;}");
    viewBox->setLayout(vb);

    return viewBox;
}

QGroupBox * page_modelSettings::createViewStatus()
{
    sizeW[VIEW_STATUS]          = new SpinSet("width",0,1,4096);
    sizeH[VIEW_STATUS]          = new SpinSet("height",0,1,2160);
    startEditX[VIEW_STATUS]     = new DoubleSpinSet("start-X",0,-4096,4096);
    startEditY[VIEW_STATUS]     = new DoubleSpinSet("start-y",0,-2160,2160);
    bkColorEdit[VIEW_STATUS]    = new QLineEdit;
    bkgdColorPatch[VIEW_STATUS] = new ClickableLabel;

    connect(sizeW[VIEW_STATUS], &SpinSet::valueChanged, this, &page_modelSettings::viewSizeChanged);
    connect(sizeH[VIEW_STATUS], &SpinSet::valueChanged, this, &page_modelSettings::viewSizeChanged);

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

    QGroupBox * box = new QGroupBox("View Status");
    box->setLayout(grid);
    return box;
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

    if (group == DESIGN_SETTINGS)
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
    qtAppLog::getInstance()->suspend(true);

    // tiling settings
    TilingPtr tiling = tilingMaker->getSelected();
    if (tiling)
    {
        qDebug() << "Tiling" << tiling->getName();
        QSize size  = tiling->getData().getSettings().getSize();
        sizeW[TILING_SETTINGS]->setValue(size.width());
        sizeH[TILING_SETTINGS]->setValue(size.height());

        const FillData & fd = tiling->getData().getFillData();
        displayFillData(fd,TILING_SETTINGS);
    }

    // mosaic/design settings;
    ModelSettings & mosaicSettings = getMosaicOrDesignSettings();

    // size
    QSizeF sz = mosaicSettings.getSize();
    sizeW[DESIGN_SETTINGS]->setValue(sz.width());
    sizeH[DESIGN_SETTINGS]->setValue(sz.height());

    // background color
    QColor qc = mosaicSettings.getBackgroundColor();
    bkColorEdit[DESIGN_SETTINGS]->setText(qc.name(QColor::HexArgb));
    QVariant variant = qc;
    QString colcode  = variant.toString();
    bkgdColorPatch[DESIGN_SETTINGS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    // start tile
    QPointF  pt = mosaicSettings.getStartTile();
    startEditX[DESIGN_SETTINGS]->setValue(pt.x());
    startEditY[DESIGN_SETTINGS]->setValue(pt.y());

    // repeats
    const FillData & fd = mosaicSettings.getFillData();
    displayFillData(fd,DESIGN_SETTINGS);

    // View Status
    // size
    QSize size  = view->size();
    sizeW[VIEW_STATUS]->setValue(size.width());
    sizeH[VIEW_STATUS]->setValue(size.height());

    // background color
    qc = view->getViewBackgroundColor();
    bkColorEdit[VIEW_STATUS]->setText(qc.name(QColor::HexArgb));
    QVariant qv = qc;  // FIXME for qt6
    colcode  = qv.toString();
    bkgdColorPatch[VIEW_STATUS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    // fill data
    const FillData & fd2 = view->getFillData();
    displayFillData(fd2,VIEW_STATUS);

    // frameTable
    if (config->insightMode && config->cs_showFrameSettings)
    {
        // frame settings
        auto viewType = view->getMostRecent();

        viewBox->setTitle(QString("Frame Settings : %1").arg(sViewerType[viewType]));

        auto & settings  = view->getViewSettings();

        QSize csize = settings.getCropSize(viewType);
        QSize zsize  = settings.getZoomSize(viewType);
        qDebug() << csize << zsize;

        sizeW[FRAME_SETTINGS]->setValue(csize.width());
        sizeH[FRAME_SETTINGS]->setValue(csize.height());
        sizeW2->setValue(zsize.width());
        sizeH2->setValue(zsize.height());

        auto bounds = settings.getBounds(viewType);
        ds_left->setValue(bounds.left);
        ds_top->setValue(bounds.top);
        ds_width->setValue(bounds.width);
        l_xform->setText(Transform::toInfoString(settings.getTransform(viewType)));

        frameBox->show();

        const QMap<eViewType,ViewData*> & fset = settings.getSettingsMap();
        if (fset.size() != frameTable->rowCount())
        {
            frameTable->clearContents();
            frameTable->setRowCount(fset.size());
        }

        int row = 0;
        QBrush brush;
        if (config->darkTheme)
            brush = QBrush(QColor(0x777777));
        else
            brush  = QBrush(Qt::yellow);

        auto recentType = view->getMostRecent();
        QMap<eViewType,ViewData*>::const_iterator i = fset.constBegin();
        while (i != fset.constEnd())
        {
            eViewType type = i.key();
            const ViewData * s = i.value();
            ++i;

            QTableWidgetItem * item =  new QTableWidgetItem(s2ViewerType[type]);
            if (type == recentType)
                item->setBackground(brush);
            frameTable->setItem(row,0,item);

            item = new QTableWidgetItem(Utils::str(s->getCropSize()));
            if (type == recentType)
                item->setBackground(brush);
            item->setTextAlignment(Qt::AlignRight);
            frameTable->setItem(row,1,item);

            item = new QTableWidgetItem(Utils::str(s->getZoomSize()));
            if (type == recentType)
                item->setBackground(brush);
            item->setTextAlignment(Qt::AlignRight);
            frameTable->setItem(row,2,item);

            row++;
        }
        frameTable->resizeColumnsToContents();
        frameTable->adjustTableSize();
        frameTable->show();
    }
    else if (config->insightMode)
    {
        frameBox->hide();
        frameTable->hide();
    }

    qtAppLog::getInstance()->suspend(false);
}

void page_modelSettings::onEnter()
{
    dummySetup();
}

void page_modelSettings::dummySetup()
{}

void page_modelSettings::designSizeChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeW[DESIGN_SETTINGS]->value(),sizeH[DESIGN_SETTINGS]->value());

    ModelSettings & settings = getMosaicOrDesignSettings();
    settings.setSize(sz);

    emit sig_refreshView();
}

void page_modelSettings::cropSizeChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeW[FRAME_SETTINGS]->value(),sizeH[FRAME_SETTINGS]->value());
    auto & settings = view->getViewSettings();
    settings.setCropSize(view->getMostRecent(),sz);
    emit sig_refreshView();
}

void page_modelSettings::viewSizeChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeW[VIEW_STATUS]->value(),sizeH[VIEW_STATUS]->value());
    view->resize(sz);   // direct
    emit sig_refreshView();
}

ModelSettings & page_modelSettings::getMosaicOrDesignSettings()
{
    if (view->isEnabled(VIEW_DESIGN))
    {
        DesignMaker * designMaker = DesignMaker::getInstance();
        QVector<DesignPtr> & designs = designMaker->getActiveDesigns();
        if (designs.count())
        {
            DesignPtr dp = designs.first();
            return dp->getDesignInfo();
        }
    }

    // drops thru
    return mosaicMaker->getMosaicSettings();
}

void page_modelSettings::slot_set_repsDesign(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    FillData fd;
    fd.set(chkSingle[DESIGN_SETTINGS]->isChecked(),xRepMin[DESIGN_SETTINGS]->value(), xRepMax[DESIGN_SETTINGS]->value(), yRepMin[DESIGN_SETTINGS]->value(), yRepMax[DESIGN_SETTINGS]->value());

    ModelSettings & settings = getMosaicOrDesignSettings();
    settings.setFillData(fd);

    view->setFillData(fd);

    emit sig_render();
}

void page_modelSettings::singleton_changed_des(bool checked)
{
    FillData fd;
    if (!checked)
        fd.set(false,-3, 3, -3, 3);
    else
        fd.set(true,0,0,0,0);

    ModelSettings & settings = getMosaicOrDesignSettings();
    settings.setFillData(fd);

    view->setFillData(fd);

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
    
    FillData & fdata = tiling->getRWData(true).getFillDataAccess();
    fdata = fd;

    view->setFillData(fd);

    if (view->isEnabled(VIEW_TILING_MAKER)
     || view->isEnabled(VIEW_TILING)
     || view->isEnabled(VIEW_MAP_EDITOR))
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
    
    FillData & fdata = tiling->getRWData(true).getFillDataAccess();
    fdata = fd;

    view->setFillData(fd);

    emit sig_render();

}
void page_modelSettings::backgroundColorDesignPick()
{
    ModelSettings & settings = getMosaicOrDesignSettings();
    QColor color = settings.getBackgroundColor();

    AQColorDialog dlg(color,this);
    dlg.setCurrentColor(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        bkColorEdit[DESIGN_SETTINGS]->setText(color.name(QColor::HexArgb));

        QVariant variant = color;
        QString colcode  = variant.toString();
        bkgdColorPatch[DESIGN_SETTINGS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    }
}

void page_modelSettings::backgroundColorDesignChanged(const QString & str)
{
    if (pageBlocked()) return;

    QColor color = QColor(str);
    ModelSettings & settings = getMosaicOrDesignSettings();
    settings.setBackgroundColor(color);
    emit sig_refreshView();
}

void page_modelSettings::slot_tilingSizeChanged(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    TilingPtr tiling = tilingMaker->getSelected();
    ModelSettings & ms  = tiling->getRWData(true).getSettingsAccess();
    ms.setSize(QSize(sizeW[TILING_SETTINGS]->value(),sizeH[TILING_SETTINGS]->value()));

    emit sig_refreshView();
}

void page_modelSettings::slot_showFrameInfoChanged(bool checked)
{
    config->cs_showFrameSettings = checked;
}

void page_modelSettings::slot_boundsChanged()
{
    Bounds bounds(ds_left->value(), ds_top->value(), ds_width->value());
    view->getViewSettings().setBounds(view->getMostRecent(),bounds);

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



