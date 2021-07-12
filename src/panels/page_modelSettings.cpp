/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "panels/page_modelSettings.h"
#include "panels/panel.h"
#include "base/border.h"
#include "base/shared.h"
#include "base/tiledpatternmaker.h"
#include "base/utilities.h"
#include "designs/design.h"
#include "designs/design_maker.h"
#include "geometry/transform.h"
#include "makers/decoration_maker/decoration_maker.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "panels/layout_sliderset.h"
#include "panels/layout_transform.h"
#include "settings/model_settings.h"
#include "tapp/prototype.h"
#include "tile/backgroundimage.h"
#include "tile/tiling.h"
#include "viewers/view.h"
#include "viewers/viewcontrol.h"

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

    adjustSize();

    // connections
    connect(theApp,  &TiledPatternMaker::sig_tilingLoaded,      this,   &page_modelSettings::onEnter);
    connect(theApp,  &TiledPatternMaker::sig_mosaicLoaded,      this,   &page_modelSettings::onEnter);
    connect(theApp,  &TiledPatternMaker::sig_loadedDesign,      this,   &page_modelSettings::onEnter);
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

    QGroupBox * box = new QGroupBox("Design Settings");
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
    connect(ds_left,               &DoubleSpinSet::sig_valueChanged, this, &page_modelSettings::slot_boundsChanged);
    connect(ds_top,                &DoubleSpinSet::sig_valueChanged, this, &page_modelSettings::slot_boundsChanged);
    connect(ds_width,              &DoubleSpinSet::sig_valueChanged, this, &page_modelSettings::slot_boundsChanged);

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

    xRepMin[group] = new AQSpinBox();
    xRepMax[group] = new AQSpinBox();
    yRepMin[group] = new AQSpinBox();
    yRepMax[group] = new AQSpinBox();

    xRepMin[group]->setRange(rmin,rmax);
    xRepMax[group]->setRange(rmin,rmax);
    yRepMin[group]->setRange(rmin,rmax);
    yRepMax[group]->setRange(rmin,rmax);

    grid->addWidget(new QLabel("xMin"),0,0);
    grid->addWidget(new QLabel("xMax"),0,1);
    grid->addWidget(new QLabel("yMin"),0,2);
    grid->addWidget(new QLabel("yMax"),0,3);

    grid->addWidget(xRepMin[group],1,0);
    grid->addWidget(xRepMax[group],1,1);
    grid->addWidget(yRepMin[group],1,2);
    grid->addWidget(yRepMax[group],1,3);

    if (group == DESIGN_SETTINGS)
    {
        connect(xRepMin[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsDesign(int)));
        connect(xRepMax[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsDesign(int)));
        connect(yRepMin[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsDesign(int)));
        connect(yRepMax[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsDesign(int)));
    }
    else if (group == TILING_SETTINGS)
    {
        connect(xRepMin[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsTiling(int)));
        connect(xRepMax[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsTiling(int)));
        connect(yRepMin[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsTiling(int)));
        connect(yRepMax[group], SIGNAL(valueChanged(int)), this,  SLOT(slot_set_repsTiling(int)));
    }
    else
    {
        xRepMin[group]->setReadOnly(true);
        xRepMax[group]->setReadOnly(true);
        yRepMin[group]->setReadOnly(true);
        yRepMax[group]->setReadOnly(true);
    }
    return grid;
}


void  page_modelSettings::refreshPage()
{
    qtAppLog::suspend(true);

    // tiling settings
    TilingPtr tiling = tilingMaker->getSelected();
    if (tiling)
    {
        qDebug() << "Tiling" << tiling->getName();
        QSize size  = tiling->getSize();
        sizeW[TILING_SETTINGS]->setValue(size.width());
        sizeH[TILING_SETTINGS]->setValue(size.height());

        FillData fd = tiling->getFillData();
        int xMin,xMax,yMin,yMax;
        fd.get(xMin,xMax,yMin,yMax);

        xRepMin[TILING_SETTINGS]->setValue(xMin);
        xRepMax[TILING_SETTINGS]->setValue(xMax);
        yRepMin[TILING_SETTINGS]->setValue(yMin);
        yRepMax[TILING_SETTINGS]->setValue(yMax);
    }

    // mosaic/design settings;
    ModelSettingsPtr mosaicSettings = getMosaicOrDesignSettings();

    // size
    QSizeF sz = mosaicSettings->getSize();
    sizeW[DESIGN_SETTINGS]->setValue(sz.width());
    sizeH[DESIGN_SETTINGS]->setValue(sz.height());

    // background color
    QColor qc = mosaicSettings->getBackgroundColor();
    bkColorEdit[DESIGN_SETTINGS]->setText(qc.name(QColor::HexArgb));
    QVariant variant = qc;
    QString colcode  = variant.toString();
    bkgdColorPatch[DESIGN_SETTINGS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    // start tile
    QPointF  pt = mosaicSettings->getStartTile();
    startEditX[DESIGN_SETTINGS]->setValue(pt.x());
    startEditY[DESIGN_SETTINGS]->setValue(pt.y());

    // repeats
    int xMin,xMax,yMin,yMax;
    mosaicSettings->getFillData().get(xMin ,xMax,yMin,yMax);
    xRepMin[DESIGN_SETTINGS]->setValue(xMin);
    xRepMax[DESIGN_SETTINGS]->setValue(xMax);
    yRepMin[DESIGN_SETTINGS]->setValue(yMin);
    yRepMax[DESIGN_SETTINGS]->setValue(yMax);

    // View Status
    // size
    QSize size  = view->size();
    sizeW[VIEW_STATUS]->setValue(size.width());
    sizeH[VIEW_STATUS]->setValue(size.height());

    // background color
    qc = view->getBackgroundColor();
    bkColorEdit[VIEW_STATUS]->setText(qc.name(QColor::HexArgb));
    variant = qc;
    colcode  = variant.toString();
    bkgdColorPatch[VIEW_STATUS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    // fill data
    FillData & fd = vcontrol->getFillData();
    fd.get(xMin ,xMax,yMin,yMax);
    xRepMin[VIEW_STATUS]->setValue(xMin);
    xRepMax[VIEW_STATUS]->setValue(xMax);
    yRepMin[VIEW_STATUS]->setValue(yMin);
    yRepMax[VIEW_STATUS]->setValue(yMax);

    // frameTable
    if (config->insightMode && config->cs_showFrameSettings)
    {
        // frame settings
        viewBox->setTitle(QString("Frame Settings : %1").arg(sViewerType[config->getViewerType()]));
        FrameData * fs  = view->frameSettings.getFrameData(config->getViewerType());

        QSize csize = fs->getCropSize();
        QSize zsize  = fs->getZoomSize();
        qDebug() << csize << zsize;

        sizeW[FRAME_SETTINGS]->setValue(csize.width());
        sizeH[FRAME_SETTINGS]->setValue(csize.height());
        sizeW2->setValue(zsize.width());
        sizeH2->setValue(zsize.height());

        ds_left->setValue(fs->getBounds().left);
        ds_top->setValue(fs->getBounds().top);
        ds_width->setValue(fs->getBounds().width);
        l_xform->setText(Transform::toInfoString(fs->getTransform()));

        frameBox->show();

        const QMap<eViewType,FrameData*> & fset = view->frameSettings.getFrameSettings();
        if (fset.size() != frameTable->rowCount())
        {
            frameTable->clearContents();
            frameTable->setRowCount(fset.size());
        }

        int row = 0;
        QMap<eViewType,FrameData*> ::const_iterator i = fset.constBegin();
        while (i != fset.constEnd())
        {
            eViewType type = i.key();
            const FrameData * s = i.value();
            ++i;

            QTableWidgetItem * item =  new QTableWidgetItem(sViewerType[type]);
            if (type == config->getViewerType()) item->setBackground(QBrush("yellow"));
            frameTable->setItem(row,0,item);

            item = new QTableWidgetItem(Utils::str(s->getCropSize()));
            if (type == config->getViewerType()) item->setBackground(QBrush("yellow"));
            item->setTextAlignment(Qt::AlignRight);
            frameTable->setItem(row,1,item);

            item = new QTableWidgetItem(Utils::str(s->getZoomSize()));
            if (type == config->getViewerType()) item->setBackground(QBrush("yellow"));
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

    qtAppLog::suspend(false);
}

void page_modelSettings::onEnter()
{
}

void page_modelSettings::designSizeChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeW[DESIGN_SETTINGS]->value(),sizeH[DESIGN_SETTINGS]->value());

    ModelSettingsPtr settings = getMosaicOrDesignSettings();
    settings->setSize(sz);

    emit sig_refreshView();
}

void page_modelSettings::cropSizeChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeW[FRAME_SETTINGS]->value(),sizeH[FRAME_SETTINGS]->value());
    FrameData * data = view->frameSettings.getFrameData(config->getViewerType());
    data->setCropSize(sz);
    emit sig_refreshView();
}

void page_modelSettings::viewSizeChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeW[VIEW_STATUS]->value(),sizeH[VIEW_STATUS]->value());
    view->resize(sz);   // direct
    emit sig_refreshView();
}

ModelSettingsPtr page_modelSettings::getMosaicOrDesignSettings()
{
    if (config->getViewerType() == VIEW_DESIGN)
    {
        DesignMaker * designMaker = DesignMaker::getInstance();
        QVector<DesignPtr> & designs = designMaker->getDesigns();
        if (designs.count())
        {
            DesignPtr dp = designs.first();
            return dp->getDesignInfo();
        }
        else
        {
            ModelSettingsPtr settings = make_shared<ModelSettings>();
            return settings;
        }
    }
    else
    {
        return decorationMaker->getMosaicSettings();
    }
}

void page_modelSettings::slot_set_repsDesign(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    FillData fd;
    fd.set(xRepMin[DESIGN_SETTINGS]->value(), xRepMax[DESIGN_SETTINGS]->value(), yRepMin[DESIGN_SETTINGS]->value(), yRepMax[DESIGN_SETTINGS]->value());

    ModelSettingsPtr settings = getMosaicOrDesignSettings();
    settings->setFillData(fd);

    vcontrol->setFillData(fd);

    emit sig_render();
}

void page_modelSettings::slot_set_repsTiling(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    FillData fd;
    fd.set(xRepMin[TILING_SETTINGS]->value(), xRepMax[TILING_SETTINGS]->value(), yRepMin[TILING_SETTINGS]->value(), yRepMax[TILING_SETTINGS]->value());

    TilingPtr tiling = tilingMaker->getSelected();
    Q_ASSERT(tiling);

    tiling->setFillData(fd);
    vcontrol->setFillData(fd);

    emit sig_render();
}

void page_modelSettings::backgroundColorDesignPick()
{
    ModelSettingsPtr settings = getMosaicOrDesignSettings();
    QColor color = settings->getBackgroundColor();

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
    ModelSettingsPtr settings = getMosaicOrDesignSettings();
    settings->setBackgroundColor(color);
    emit sig_refreshView();
}



void page_modelSettings::slot_tilingSizeChanged(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    TilingPtr tiling = tilingMaker->getSelected();
    tiling->setSize(QSize(sizeW[TILING_SETTINGS]->value(),sizeH[TILING_SETTINGS]->value()));

    emit sig_refreshView();
}

void page_modelSettings::slot_showFrameInfoChanged(bool checked)
{
    config->cs_showFrameSettings = checked;
}

void page_modelSettings::slot_boundsChanged()
{
    FrameData * frd = view->frameSettings.getFrameData(config->getViewerType());
    Bounds & bounds = frd->getBounds();
    bounds.left  = ds_left->value();
    bounds.top   = ds_top->value();
    bounds.width = ds_width->value();
    frd->calculateTransform();

    QVector<LayerPtr> layers = view->getActiveLayers();
    for (LayerPtr layer : layers)
    {
        layer->forceLayerRecalc(true);
    }
}
