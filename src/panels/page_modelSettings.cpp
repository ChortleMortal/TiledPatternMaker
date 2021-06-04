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
#include "tile/tiling.h"
#include "tile/backgroundimage.h"
#include "tapp/prototype.h"
#include "base/border.h"
#include "designs/design.h"
#include "base/tiledpatternmaker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/decoration_maker/decoration_maker.h"
#include "designs/design_maker.h"
#include "geometry/transform.h"
#include "base/utilities.h"

using std::string;

page_modelSettings::page_modelSettings(ControlPanel * apanel)  : panel_page(apanel,"Model Settings")
{
    QPushButton  *pbRefresh       = new QPushButton("Refresh");
    QCheckBox    * chkShowBkgds   = new QCheckBox("Show Backgrounds");
    chkShowBkgds->setChecked(config->cs_showBkgds);

    QHBoxLayout * sourcebox = new QHBoxLayout();
    sourcebox->addStretch();
    sourcebox->addWidget(pbRefresh);
    sourcebox->addStretch();
    sourcebox->addWidget(chkShowBkgds);

    if (config->insightMode)
    {
        QCheckBox    * chkShowFset    = new QCheckBox("Show Frame Settings");
        chkShowFset->setChecked(config->cs_showFrameSettings);
        sourcebox->addWidget(chkShowFset);

        connect(chkShowFset, &QCheckBox::clicked, this, &page_modelSettings::slot_showFsetChanged);
    }
    else
    {
        config->cs_showFrameSettings = false;
    }

    QGridLayout * pgrid  = new QGridLayout();
    QGroupBox * box;

    int row = 0;
    box = createTilingSettings();
    pgrid->addWidget(box,row,0);

    box = createFrameSettings();
    pgrid->addWidget(box,row,1);

    row++;
    box = createDesignSettings();
    pgrid->addWidget(box,row,0);

    box = createViewStatus();
    pgrid->addWidget(box,row,1);

    //  background image
    mosaicBkgdBox = createBackgroundImageGroup(DESIGN_SETTINGS,"Design Background Image");
    tilingBkgdBox = createBackgroundImageGroup(TILING_SETTINGS,"Tiling Background Image");

    // putting it all together
    vbox->addLayout(sourcebox);
    vbox->addLayout(pgrid);
    vbox->addWidget(mosaicBkgdBox);
    vbox->addWidget(tilingBkgdBox);

    if (config->insightMode)
    {
        frameTable = new AQTableWidget();
        QStringList qslH;
        qslH << "" << "Default" << "Defined" << "Active";
        frameTable->setColumnCount(4);
        frameTable->setHorizontalHeaderLabels(qslH);
        frameTable->horizontalHeader()->setVisible(true);
        frameTable->verticalHeader()->setVisible(false);

        vbox->addWidget(frameTable);
    }
    else
    {
        frameTable = nullptr;
    }

    adjustSize();

    // connections
    connect(pbRefresh,        &QPushButton::clicked,            this,   &page_modelSettings::display);
    connect(chkShowBkgds,     &QCheckBox::clicked,              this,   &page_modelSettings::slot_showBkgdsChanged);

    connect(vcontrol,         &ViewControl::sig_viewUpdated,    this,   &page_modelSettings::slot_viewUpated);

    connect(theApp,  &TiledPatternMaker::sig_tilingLoaded,      this,   &page_modelSettings::onEnter);
    connect(theApp,  &TiledPatternMaker::sig_mosaicLoaded,      this,   &page_modelSettings::onEnter);
    connect(theApp,  &TiledPatternMaker::sig_loadedDesign,      this,   &page_modelSettings::onEnter);

    //connect(adjustBtn,        &QPushButton::clicked,          this,    &page_canvasSettings::slot_adjustBackground);
    //connect(saveAdjustedBtn,  &QPushButton::clicked,          this,    &page_canvasSettings::slot_saveAdjustedBackground);

    //connect(canvas, &Canvas::sig_deltaScale,    this, &page_canvasSettings::slot_bkgd_scale);
    //connect(canvas, &Canvas::sig_deltaRotate,   this, &page_canvasSettings::slot_bkgd_rotate);
    //connect(canvas, &Canvas::sig_deltaMoveY,    this, &page_canvasSettings::slot_bkgd_moveY);
    //connect(canvas, &Canvas::sig_deltaMoveX,    this, &page_canvasSettings::slot_bkgd_moveX);
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

    frameXform = new QLabel();

    connect(sizeW[FRAME_SETTINGS], &SpinSet::valueChanged, this, &page_modelSettings::frameSizeChanged);
    connect(sizeH[FRAME_SETTINGS], &SpinSet::valueChanged, this, &page_modelSettings::frameSizeChanged);

    QLabel * l1 = new QLabel("Defined: ");
    QLabel * l2 = new QLabel("Active: ");

    QGridLayout * grid = new QGridLayout();
    grid->addWidget(l1,0,0);
    grid->addLayout(sizeW[FRAME_SETTINGS],0,1);
    grid->addLayout(sizeH[FRAME_SETTINGS],0,2);
    grid->addWidget(l2,1,0);
    grid->addLayout(sizeW2,1,1);
    grid->addLayout(sizeH2,1,2);
    grid->addWidget(frameXform,2,0,1,3);

    viewBox = new QGroupBox("Frame Size");
    viewBox->setStyleSheet("QGroupBox:title {color: red;}");
    viewBox->setLayout(grid);

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

QGroupBox * page_modelSettings::createBackgroundImageGroup(eSettingsGroup group, QString title)
{
    QGroupBox * sbox       = new QGroupBox(title);
    QVBoxLayout * svlayout = new QVBoxLayout;
    sbox->setLayout(svlayout);

    QHBoxLayout * hbox    = new QHBoxLayout;
    bkgdImageBtn[group]   = new QPushButton("Load Background");
    bkgdImage[group]      = new QLineEdit();
    chk_adjustBkgd[group] = new QCheckBox("Use Perspective");
    hbox->addWidget(bkgdImage[group]);
    hbox->addWidget(bkgdImageBtn[group]);
    hbox->addWidget(chk_adjustBkgd[group]);
    //hbox->addStretch();
    svlayout->addLayout(hbox);

    bkgdLayout[group] = new LayoutTransform("Xform",5);
    svlayout->addLayout(bkgdLayout[group]);

    if (group == DESIGN_SETTINGS)
    {
        connect(bkgdImageBtn[group],  &QPushButton::clicked,            this,    &page_modelSettings::slot_loadMosaicBackground);
        connect(bkgdLayout[group],    &LayoutTransform::xformChanged,   this,    &page_modelSettings::slot_setBkgdMosaicXform);
        connect(chk_adjustBkgd[group],&QAbstractButton::clicked,        this,    &page_modelSettings::slot_setBkgdMosaic);
    }

    if (group == TILING_SETTINGS)
    {
        connect(bkgdImageBtn[group],  &QPushButton::clicked,            this,    &page_modelSettings::slot_loadTilingBackground);
        connect(bkgdLayout[group],    &LayoutTransform::xformChanged,   this,    &page_modelSettings::slot_setBkgdTilingXform);
        connect(chk_adjustBkgd[group],&QAbstractButton::clicked,        this,    &page_modelSettings::slot_setBkgdTiling);
    }

    return sbox;
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
    // frame settings
    viewBox->setTitle(QString("FrameSize : %1").arg(sViewerType[config->getViewerType()]));
    QSize isz = view->getActiveFrameSize(config->getViewerType());
    sizeW2->setValue(isz.width());
    sizeH2->setValue(isz.height());
    frameXform->setText(Transform::toInfoString(view->getDefinedFrameTransform(config->getViewerType())));

    // View Status
    // sizeW
    QSize size  = view->size();
    sizeW[VIEW_STATUS]->setValue(size.width());
    sizeH[VIEW_STATUS]->setValue(size.height());

    // background color
    QColor qc = view->getBackgroundColor();
    bkColorEdit[VIEW_STATUS]->setText(qc.name(QColor::HexArgb));
    QVariant variant = qc;
    QString colcode  = variant.toString();
    bkgdColorPatch[VIEW_STATUS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    // fill data
    FillData & fd = vcontrol->getFillData();
    int xMin,xMax,yMin,yMax;
    fd.get(xMin ,xMax,yMin,yMax);
    xRepMin[VIEW_STATUS]->setValue(xMin);
    xRepMax[VIEW_STATUS]->setValue(xMax);
    yRepMin[VIEW_STATUS]->setValue(yMin);
    yRepMax[VIEW_STATUS]->setValue(yMax);

    // frameTable
    if (frameTable && config->cs_showFrameSettings)
    {
        const QMap<eViewType,FrameSettings> & fset = view->getFrameSettings();
        if (fset.size() != frameTable->rowCount())
        {
            frameTable->clearContents();
            frameTable->setRowCount(fset.size());
        }

        int row = 0;
        QMap<eViewType,FrameSettings> ::const_iterator i = fset.constBegin();
        while (i != fset.constEnd())
        {
            eViewType type = i.key();
            const FrameSettings & s = i.value();
            ++i;

            QTableWidgetItem * item =  new QTableWidgetItem(sViewerType[type]);
            frameTable->setItem(row,0,item);

            item = new QTableWidgetItem(Utils::str(s.getDefaultFrameSize()));
            item->setTextAlignment(Qt::AlignRight);
            frameTable->setItem(row,1,item);

            item = new QTableWidgetItem(Utils::str(s.getDefinedFrameSize()));
            item->setTextAlignment(Qt::AlignRight);
            frameTable->setItem(row,2,item);

            item = new QTableWidgetItem(Utils::str(s.getActiveFrameSize()));
            item->setTextAlignment(Qt::AlignRight);
            frameTable->setItem(row,3,item);

            row++;
        }
        frameTable->resizeColumnsToContents();
        frameTable->adjustTableSize();
    }
}

void page_modelSettings::onEnter()
{
    display();
}

void page_modelSettings::slot_viewUpated()
{
    if (panel->isVisiblePage(this))
    {
        display();
    }
}

void page_modelSettings::display()
{
    blockPage(true);

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

    int xMin,xMax,yMin,yMax;
    mosaicSettings->getFillData().get(xMin ,xMax,yMin,yMax);
    xRepMin[DESIGN_SETTINGS]->setValue(xMin);
    xRepMax[DESIGN_SETTINGS]->setValue(xMax);
    yRepMin[DESIGN_SETTINGS]->setValue(yMin);
    yRepMax[DESIGN_SETTINGS]->setValue(yMax);

    // frame settings
    QSize isz = view->getDefinedFrameSize(config->getViewerType());
    sizeW[FRAME_SETTINGS]->setValue(isz.width());
    sizeH[FRAME_SETTINGS]->setValue(isz.height());
    isz = view->getActiveFrameSize(config->getViewerType());
    sizeW2->setValue(isz.width());
    sizeH2->setValue(isz.height());
    frameXform->setText(Transform::toInfoString(view->getDefinedFrameTransform(config->getViewerType())));

    // tiling settings
    TilingPtr tiling = tilingMaker->getSelected();

    if (tiling)
    {
        qDebug() << "Tiling" << tiling->getName();
        QSize size  = tiling->getSize();
        sizeW[TILING_SETTINGS]->setValue(size.width());
        sizeH[TILING_SETTINGS]->setValue(size.height());

        FillData fd = tiling->getFillData();
        fd.get(xMin,xMax,yMin,yMax);

        xRepMin[TILING_SETTINGS]->setValue(xMin);
        xRepMax[TILING_SETTINGS]->setValue(xMax);
        yRepMin[TILING_SETTINGS]->setValue(yMin);
        yRepMax[TILING_SETTINGS]->setValue(yMax);
    }

    if (config->cs_showBkgds)
    {
        BkgdImgPtr p1 = mosaicSettings->getBkgdImage();
        BkgdImgPtr p2;
        displayBkgdImgSettings(p1,DESIGN_SETTINGS);

        if (tiling)
        {
            p2 = tiling->getBackground();
            displayBkgdImgSettings(p2,TILING_SETTINGS);
        }
        qDebug() << "Mosaic settings" << p1.get() << "Tiling settings" << p2.get();

        mosaicBkgdBox->show();
        tilingBkgdBox->show();
    }
    else
    {
        mosaicBkgdBox->hide();
        tilingBkgdBox->hide();
    }

    if (config->cs_showFrameSettings)
    {
        frameTable->show();
    }
    else if (config->insightMode)
    {
        frameTable->hide();
    }

    blockPage(false);
}

void page_modelSettings::designSizeChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeW[DESIGN_SETTINGS]->value(),sizeH[DESIGN_SETTINGS]->value());

    ModelSettingsPtr settings = getMosaicOrDesignSettings();
    settings->setSize(sz);

    emit sig_refreshView();
}

void page_modelSettings::frameSizeChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeW[FRAME_SETTINGS]->value(),sizeH[FRAME_SETTINGS]->value());

    view->setDefinedFrameSize(config->getViewerType(),sz);

    emit sig_refreshView();
}

void page_modelSettings::viewSizeChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeW[VIEW_STATUS]->value(),sizeH[VIEW_STATUS]->value());

    view->setActiveFrameSize(config->getViewerType(),sz);
    if (config->scaleToView)
    {
        view->setDefinedFrameSize(config->getViewerType(),sz);
    }
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

void page_modelSettings::displayBkgdImgSettings(BkgdImgPtr bi, eSettingsGroup group)
{
    // display background
    if (bi)
    {
        Xform xf = bi->getBkgdXform();
        bkgdLayout[group]->setTransform(xf);
        bkgdImage[group]->setText(bi->getName());
        //chk_showBkgd[group]->setChecked(bi->bShowBkgd);
        chk_adjustBkgd[group]->setChecked(bi->useAdjusted());
    }
    else
    {
        bkgdImage[group]->setText("none");
    }
}

void page_modelSettings::slot_loadMosaicBackground()
{
    QString bkgdDir = config->rootMediaDir + "bkgd_photos/";
    QString filename = QFileDialog::getOpenFileName(nullptr,"Select image file",bkgdDir, "Image Files (*.png *.jpg *.bmp *.heic)");
    if (filename.isEmpty()) return;

    bool rv = BackgroundImage::import(filename);
    if (rv)
    {
        // load
        QFileInfo info(filename);
        QString name = info.fileName();
        BkgdImgPtr bi = make_shared<BackgroundImage>(name);
        if (bi->isLoaded())
        {
            ModelSettingsPtr settings = getMosaicOrDesignSettings();
            settings->setBkgdImage(bi);

            bi->createPixmap();

            display();

            emit sig_refreshView();
        }
    }
}

void page_modelSettings::slot_loadTilingBackground()
{
    QString bkgdDir = config->rootMediaDir + "bkgd_photos/";
    QString filename = QFileDialog::getOpenFileName(nullptr,"Select image file",bkgdDir, "Image Files (*.png *.jpg *.bmp *.heic)");
    if (filename.isEmpty()) return;

    bool rv = BackgroundImage::import(filename);
    if (rv)
    {
        // load
        QFileInfo info(filename);
        QString name = info.fileName();
        BkgdImgPtr bi = make_shared<BackgroundImage>(name);
        if (bi->isLoaded())
        {
            TilingPtr tp = tilingMaker->getSelected();
            tp->setBackground(bi);

            bi->createPixmap();

            display();

            emit sig_refreshView();
        }
    }
}

void page_modelSettings::slot_setBkgdMosaic()
{
    ModelSettingsPtr settings = decorationMaker->getMosaicSettings();

    BkgdImgPtr bi = settings->getBkgdImage();
    if (bi)
    {
        Xform xf = bi->getCanvasXform();
        xf.setTransform(bkgdLayout[DESIGN_SETTINGS]->getQTransform());
        bi->updateBkgdXform(xf);
        bi->createPixmap();
    }
}

void page_modelSettings::slot_setBkgdTiling()
{
    BkgdImgPtr bi = tilingMaker->getSelected()->getBackground();
    if (bi)
    {
        Xform xf = bi->getCanvasXform();
        xf.setTransform(bkgdLayout[TILING_SETTINGS]->getQTransform());
        bi->updateBkgdXform(xf);
        bi->createPixmap();
    }
}

void page_modelSettings::slot_setBkgdMosaicXform()
{
    ModelSettingsPtr settings = decorationMaker->getMosaicSettings();

    BkgdImgPtr bi = settings->getBkgdImage();
    if (bi)
    {
        Xform xf = bi->getBkgdXform();
        xf.setTransform(bkgdLayout[DESIGN_SETTINGS]->getQTransform());
        bi->updateBkgdXform(xf);
    }
}

void page_modelSettings::slot_setBkgdTilingXform()
{
    BkgdImgPtr bi = tilingMaker->getSelected()->getBackground();
    if (bi)
    {
        Xform xf = bi->getBkgdXform();
        xf.setTransform(bkgdLayout[TILING_SETTINGS]->getQTransform());
        bi->updateBkgdXform(xf);
    }
}

void page_modelSettings::slot_tilingSizeChanged(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    TilingPtr tiling = tilingMaker->getSelected();
    tiling->setSize(QSize(sizeW[TILING_SETTINGS]->value(),sizeH[TILING_SETTINGS]->value()));

    emit sig_refreshView();
}

void page_modelSettings::slot_adjustBackground()
{
#if 0
    if (tilingMaker->getMouseMode() != BKGD_SKEW_MODE)
        return;

    EdgePoly & waccum = tilingMaker->getAccumW();
    if (waccum.size() != 4)
        return;

    TilingPtr tiling = tilingMaker->getTiling();
    if (!tiling) return;


    BkgdImgPtr bi = tiling->getBackground();
    bi->adjustBackground(
        tilingMaker->worldToScreen(waccum[0]->getV1()->getPosition()),
        tilingMaker->worldToScreen(waccum[1]->getV1()->getPosition()),
        tilingMaker->worldToScreen(waccum[2]->getV1()->getPosition()),
        tilingMaker->worldToScreen(waccum[3]->getV1()->getPosition()));


    displayBackgroundStatus(tiling);
    tilingMaker->setMouseMode(NO_MOUSE_MODE);

    if (config->getViewerType() == VIEW_TILING_MAKER)
    {
        emit sig_viewWS();
    }
#else
    QMessageBox box;
    box.setIcon(QMessageBox::Information);
    box.setText("Not Implemented");
    box.exec();
#endif
}


void page_modelSettings::slot_saveAdjustedBackground()
{
#if 0
    TilingPtr tiling = tilingMaker->getTiling();
    if (!tiling) return;

    BkgdImgPtr bi   = tiling->getBackground();
    QString oldname = bi->bkgdName;

    DlgName dlg;
    dlg.newEdit->setText(oldname);
    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }

    Q_ASSERT(retval == QDialog::Accepted);
    QString newName = dlg.newEdit->text();

    // save
    bool rv = bi->saveAdjusted(newName);

    QMessageBox box;
    if (rv)
    {
        box.setIcon(QMessageBox::Information);
        box.setText("OK");
    }
    else
    {
        box.setIcon(QMessageBox::Warning);
        box.setText("FAILED");
    }
    box.exec();
#else
    QMessageBox box;
    box.setIcon(QMessageBox::Information);
    box.setText("Not Implemented");
    box.exec();
#endif
}

#if 0
void page_canvasSettings::slot_bkgd_moveX(int amount)
{
    if (panel->getCurrentPage() == dynamic_cast<panel_page*>(this) && config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        qDebug() << "page_canvasSettings::slot_moveX - background" << amount;
        bkgdLayout[DESIGN_SETTINGS]->bumpX(amount);
    }
}

void page_canvasSettings::slot_bkgd_moveY(int amount)
{
    if (panel->getCurrentPage() == dynamic_cast<panel_page*>(this) && config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        qDebug() << "page_canvasSettings::slot_moveY" << amount;
        bkgdLayout[DESIGN_SETTINGS]->bumpY(amount);
    }
}

void page_canvasSettings::slot_bkgd_rotate(int amount)
{
    if (panel->getCurrentPage() == dynamic_cast<panel_page*>(this) && config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        qDebug() << "page_canvasSettings::slot_rotate" << amount;
        bkgdLayout[DESIGN_SETTINGS]->bumpRot(amount);
    }
}

void page_canvasSettings::slot_bkgd_scale(int amount)
{
    if (panel->getCurrentPage() == dynamic_cast<panel_page*>(this) && config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        qDebug() << "page_canvasSettings::slot_scale" << amount;
        bkgdLayout[DESIGN_SETTINGS]->bumpScale(-amount);
    }
}

#endif

void page_modelSettings::slot_showBkgdsChanged(bool checked)
{
    config->cs_showBkgds = checked;
    display();
}

void page_modelSettings::slot_showFsetChanged(bool checked)
{
    config->cs_showFrameSettings = checked;
    display();
}
