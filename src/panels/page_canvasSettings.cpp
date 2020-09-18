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

#include "panels/page_canvasSettings.h"
#include "tile/tiling.h"
#include "tapp/prototype.h"
#include "style/sketch.h"
#include "base/border.h"
#include "base/tiledpatternmaker.h"
#include "base/utilities.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "panels/panel.h"
#include "viewers/workspace_viewer.h"

using std::string;

page_canvasSettings::page_canvasSettings(ControlPanel * apanel)  : panel_page(apanel,"Canvas Maker")
{
    QPushButton  *pbRefresh     = new QPushButton("Refresh");
    QPushButton  *pbMatchDesign = new QPushButton("Match Design to Workspace");
    QPushButton  *pbMatchTiling = new QPushButton("Match Tiling to Workspace");

    QHBoxLayout * sourcebox = new QHBoxLayout();
    sourcebox->addStretch();
    sourcebox->addWidget(pbMatchDesign);
    sourcebox->addWidget(pbMatchTiling);
    sourcebox->addWidget(pbRefresh);
    sourcebox->addStretch();

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

    //  mosaic background image
    QGroupBox * sbox = createBackgroundImageGroup(DESIGN_SETTINGS,"Design Background Image");

    // putting it all together
    vbox->addLayout(sourcebox);
    vbox->addLayout(pgrid);
    vbox->addWidget(sbox);
    if (config->insightMode)
    {
        //  tiling
        QGroupBox * tbox = createBackgroundImageGroup(TILING_SETTINGS,"Tiling Background Image");
#if 0
        box = createDesignBorderSettings();
        pgrid->addWidget(box,3,0);

        box = createCanvasBorderSettings();
        pgrid->addWidget(box,3,1);

#endif
        vbox->addWidget(tbox);
    }

    adjustSize();

    // connections

    connect(pbRefresh,        &QPushButton::clicked,            this,    &page_canvasSettings::display);
    connect(pbMatchDesign,    &QPushButton::clicked,            this,    &page_canvasSettings::slot_matchDesign);
    connect(pbMatchTiling,    &QPushButton::clicked,            this,    &page_canvasSettings::slot_matchTiling);

    connect(workspace,         &WorkspaceViewer::sig_viewUpdated,this,    &page_canvasSettings::display);

    connect(tpm,  &TiledPatternMaker::sig_loadedTiling,      this, &page_canvasSettings::onEnter);
    connect(tpm,  &TiledPatternMaker::sig_loadedXML,         this, &page_canvasSettings::onEnter);
    connect(tpm,  &TiledPatternMaker::sig_loadedDesign,      this, &page_canvasSettings::onEnter);

    //connect(adjustBtn,        &QPushButton::clicked,            this,    &page_canvasSettings::slot_adjustBackground);
    //connect(saveAdjustedBtn,  &QPushButton::clicked,            this,    &page_canvasSettings::slot_saveAdjustedBackground);

    //connect(canvas, &Canvas::sig_deltaScale,    this, &page_canvasSettings::slot_bkgd_scale);
    //connect(canvas, &Canvas::sig_deltaRotate,   this, &page_canvasSettings::slot_bkgd_rotate);
    //connect(canvas, &Canvas::sig_deltaMoveY,    this, &page_canvasSettings::slot_bkgd_moveY);
    //connect(canvas, &Canvas::sig_deltaMoveX,    this, &page_canvasSettings::slot_bkgd_moveX);
}

QGroupBox *page_canvasSettings::createTilingSettings()
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

    connect(sizeW[TILING_SETTINGS], &SpinSet::valueChanged, this, &page_canvasSettings::slot_tilingSizeChanged);
    connect(sizeH[TILING_SETTINGS], &SpinSet::valueChanged, this, &page_canvasSettings::slot_tilingSizeChanged);

    return box;
}

QGroupBox *page_canvasSettings::createDesignSettings()
{
    sizeW[DESIGN_SETTINGS]      = new SpinSet("width",0,1,4096);
    sizeH[DESIGN_SETTINGS]      = new SpinSet("height",0,1,2160);
    startEditX[DESIGN_SETTINGS]     = new DoubleSpinSet("start-X",0,-4096,4096);
    startEditY[DESIGN_SETTINGS]     = new DoubleSpinSet("start-y",0,-2160,2160);
    bkColorEdit[DESIGN_SETTINGS]    = new QLineEdit;
    bkgdColorPatch[DESIGN_SETTINGS] = new ClickableLabel;

    QGridLayout * fillGrid = createFillDataRow(DESIGN_SETTINGS);

    connect(sizeW[DESIGN_SETTINGS],      &SpinSet::valueChanged,  this, &page_canvasSettings::designSizeChanged);
    connect(sizeH[DESIGN_SETTINGS],      &SpinSet::valueChanged,  this, &page_canvasSettings::designSizeChanged);
    connect(bkgdColorPatch[DESIGN_SETTINGS], &ClickableLabel::clicked,this, &page_canvasSettings::backgroundColorDesignPick);
    connect(bkColorEdit[DESIGN_SETTINGS],    &QLineEdit::textChanged, this, &page_canvasSettings::backgroundColorDesignChanged);

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

QGroupBox *page_canvasSettings::createFrameSettings()
{
    sizeW[FRAME_SETTINGS] = new SpinSet("width",0,1,4096);
    sizeH[FRAME_SETTINGS] = new SpinSet("height",0,1,2160);

    connect(sizeW[FRAME_SETTINGS], &SpinSet::valueChanged, this, &page_canvasSettings::frameSizeChanged);
    connect(sizeH[FRAME_SETTINGS], &SpinSet::valueChanged, this, &page_canvasSettings::frameSizeChanged);

    QLabel * dummy = new QLabel;

    QGridLayout * grid = new QGridLayout();
    grid->addLayout(sizeW[FRAME_SETTINGS],0,0);
    grid->addLayout(sizeH[FRAME_SETTINGS],0,1);
    grid->addWidget(dummy,1,1);

    viewBox = new QGroupBox("Frame Size");
    viewBox->setStyleSheet("QGroupBox:title {color: red;}");
    viewBox->setLayout(grid);
    return viewBox;
}

QGroupBox * page_canvasSettings::createViewStatus()
{
    sizeW[VIEW_STATUS]      = new SpinSet("width",0,1,4096);
    sizeH[VIEW_STATUS]      = new SpinSet("height",0,1,2160);
    startEditX[VIEW_STATUS]     = new DoubleSpinSet("start-X",0,-4096,4096);
    startEditY[VIEW_STATUS]     = new DoubleSpinSet("start-y",0,-2160,2160);
    bkColorEdit[VIEW_STATUS]    = new QLineEdit;
    bkgdColorPatch[VIEW_STATUS] = new ClickableLabel;

    connect(sizeW[VIEW_STATUS], &SpinSet::valueChanged, this, &page_canvasSettings::viewSizeChanged);
    connect(sizeH[VIEW_STATUS], &SpinSet::valueChanged, this, &page_canvasSettings::viewSizeChanged);

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

    QGroupBox * box = new QGroupBox("Workspace Status");
    box->setLayout(grid);
    return box;
}

QGroupBox * page_canvasSettings::createBackgroundImageGroup(eSettingsGroup group, QString title)
{
    QGroupBox * sbox       = new QGroupBox(title);
    QVBoxLayout * svlayout = new QVBoxLayout;
    sbox->setLayout(svlayout);

    QHBoxLayout * hbox    = new QHBoxLayout;
    bkgdImageBtn[group]   = new QPushButton("Select");
    bkgdImage[group]      = new QLineEdit();
    chk_showBkgd[group]   = new QCheckBox("Show");
    chk_adjustBkgd[group] = new QCheckBox("Perspective");
    hbox->addWidget(bkgdImage[group]);
    hbox->addWidget(bkgdImageBtn[group]);
    hbox->addWidget(chk_showBkgd[group]);
    hbox->addWidget(chk_adjustBkgd[group]);
    //hbox->addStretch();
    svlayout->addLayout(hbox);

    bkgdLayout[group] = new LayoutTransform("Xform",5);
    svlayout->addLayout(bkgdLayout[group]);

    if (group == DESIGN_SETTINGS)
    {
        connect(bkgdImageBtn[group],  &QPushButton::clicked,            this,    &page_canvasSettings::slot_loadMosaicBackground);
        connect(bkgdLayout[group],    &LayoutTransform::xformChanged,   this,    &page_canvasSettings::slot_setBkgdMosaicXform);
        connect(chk_showBkgd[group],  &QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgdMosaic);
        connect(chk_adjustBkgd[group],&QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgdMosaic);
    }

    if (group == TILING_SETTINGS)
    {
        connect(bkgdImageBtn[group],  &QPushButton::clicked,            this,    &page_canvasSettings::slot_loadTilingBackground);
        connect(bkgdLayout[group],    &LayoutTransform::xformChanged,   this,    &page_canvasSettings::slot_setBkgdTilingXform);
        connect(chk_showBkgd[group],  &QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgdTiling);
        connect(chk_adjustBkgd[group],&QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgdTiling);
    }

    hbox = new QHBoxLayout;
    QPushButton * adjustBtn       = new QPushButton("Adjust perspective");
    QPushButton * saveAdjustedBtn = new QPushButton("Save Adjusted");
    hbox->addWidget(adjustBtn);
    hbox->addStretch();
    hbox->addWidget(saveAdjustedBtn);
    svlayout->addLayout(hbox);

    return sbox;
}

QGridLayout * page_canvasSettings::createFillDataRow(eSettingsGroup group)
{
    QGridLayout * grid = new QGridLayout;

    const int rmin = -99;
    const int rmax =  99;

    xRepMin[group] = new QSpinBox();
    xRepMax[group] = new QSpinBox();
    yRepMin[group] = new QSpinBox();
    yRepMax[group] = new QSpinBox();

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

QGroupBox * page_canvasSettings::createCanvasBorderSettings()
{
    QGroupBox * bbox = new QGroupBox("Canvas Border Settings");
#if 0
    QGridLayout * bgrid  = new QGridLayout();
    bbox->setLayout(bgrid);

    // border
    QLabel * label = new QLabel("Border");
    bgrid->addWidget(label,0,0);

    borderType[CANVAS_SETTINGS].addItem("No border",BORDER_NONE);
    borderType[CANVAS_SETTINGS].addItem("Solid border",BORDER_PLAIN);
    borderType[CANVAS_SETTINGS].addItem("Two color border",BORDER_TWO_COLOR);
    borderType[CANVAS_SETTINGS].addItem("Shaped border",BORDER_BLOCKS);
    bgrid->addWidget(&borderType[CANVAS_SETTINGS],0,1);

    label = new QLabel("Border Width");
    bgrid->addWidget(label,1,0);
    borderWidth[CANVAS_SETTINGS] = new QLineEdit();
    bgrid->addWidget(borderWidth[CANVAS_SETTINGS],1,1);

    borderColorLabel[CANVAS_SETTINGS][0] = new QLabel("Border Color");
    bgrid->addWidget(borderColorLabel[CANVAS_SETTINGS][0],2,0);
    borderColor[CANVAS_SETTINGS][0] = new QLineEdit();
    bgrid->addWidget(borderColor[CANVAS_SETTINGS][0],2,1);
    borderColorPatch[CANVAS_SETTINGS][0] = new ClickableLabel;
    borderColorPatch[CANVAS_SETTINGS][0]->setMinimumWidth(50);
    bgrid->addWidget(borderColorPatch[CANVAS_SETTINGS][0],2,2);

    borderColorLabel[CANVAS_SETTINGS][1] = new QLabel("Border Color 2");
    bgrid->addWidget(borderColorLabel[CANVAS_SETTINGS][1],3,0);
    borderColor[CANVAS_SETTINGS][1] = new QLineEdit();
    bgrid->addWidget(borderColor[CANVAS_SETTINGS][1],3,1);
    borderColorPatch[CANVAS_SETTINGS][1] = new ClickableLabel;
    borderColorPatch[CANVAS_SETTINGS][1]->setMinimumWidth(50);
    bgrid->addWidget(borderColorPatch[CANVAS_SETTINGS][1],3,2);

    connect(borderColorPatch[CANVAS_SETTINGS][0],&ClickableLabel::clicked,      this, &page_canvasSettings::pickBorderColor);
    connect(borderColorPatch[CANVAS_SETTINGS][1],&ClickableLabel::clicked,      this, &page_canvasSettings::pickBorderColor2);
    connect(&borderType[CANVAS_SETTINGS],     SIGNAL(currentIndexChanged(int)), this, SLOT(borderChanged(int)));
#endif
    return bbox;
}

QGroupBox *page_canvasSettings::createDesignBorderSettings()
{
    QGroupBox * bbox = new QGroupBox("Design Border Settings");
#if 0
    QGridLayout * bgrid  = new QGridLayout();
    bbox->setLayout(bgrid);

    // border
    QLabel * label = new QLabel("Border");
    bgrid->addWidget(label,0,0);

    borderType[DESIGN_SETTINGS].addItem("No border",BORDER_NONE);
    borderType[DESIGN_SETTINGS].addItem("Solid border",BORDER_PLAIN);
    borderType[DESIGN_SETTINGS].addItem("Two color border",BORDER_TWO_COLOR);
    borderType[DESIGN_SETTINGS].addItem("Shaped border",BORDER_BLOCKS);
    bgrid->addWidget(&borderType[DESIGN_SETTINGS],0,1);

    label = new QLabel("Border Width");
    bgrid->addWidget(label,1,0);
    borderWidth[DESIGN_SETTINGS] = new QLineEdit();
    bgrid->addWidget(borderWidth[DESIGN_SETTINGS],1,1);

    borderColorLabel[DESIGN_SETTINGS][0] = new QLabel("Border Color");
    bgrid->addWidget(borderColorLabel[DESIGN_SETTINGS][0],2,0);
    borderColor[DESIGN_SETTINGS][0] = new QLineEdit();
    bgrid->addWidget(borderColor[DESIGN_SETTINGS][0],2,1);
    borderColorPatch[DESIGN_SETTINGS][0] = new ClickableLabel;
    borderColorPatch[DESIGN_SETTINGS][0]->setMinimumWidth(50);
    bgrid->addWidget(borderColorPatch[DESIGN_SETTINGS][0],2,2);

    borderColorLabel[DESIGN_SETTINGS][1] = new QLabel("Border Color 2");
    bgrid->addWidget(borderColorLabel[DESIGN_SETTINGS][1],3,0);
    borderColor[DESIGN_SETTINGS][1] = new QLineEdit();
    bgrid->addWidget(borderColor[DESIGN_SETTINGS][1],3,1);
    borderColorPatch[DESIGN_SETTINGS][1] = new ClickableLabel;
    borderColorPatch[DESIGN_SETTINGS][1]->setMinimumWidth(50);
    bgrid->addWidget(borderColorPatch[DESIGN_SETTINGS][1],3,2);

    connect(borderColorPatch[DESIGN_SETTINGS][0],&ClickableLabel::clicked,      this, &page_canvasSettings::pickBorderColor);
    connect(borderColorPatch[DESIGN_SETTINGS][1],&ClickableLabel::clicked,      this, &page_canvasSettings::pickBorderColor2);
    connect(&borderType[DESIGN_SETTINGS],     SIGNAL(currentIndexChanged(int)), this, SLOT(borderChanged(int)));
#endif
    return bbox;
}

void  page_canvasSettings::refreshPage()
{
    // view settings
    viewBox->setTitle(QString("FrameSize : %1").arg(sViewerType[config->viewerType]));

    // View Status
    // sizeW
    QSize size  = workspace->size();
    sizeW[VIEW_STATUS]->setValue(size.width());
    sizeH[VIEW_STATUS]->setValue(size.height());

    // background color
    QColor qc = workspace->getBackgroundColor();
    bkColorEdit[VIEW_STATUS]->setText(qc.name(QColor::HexArgb));
    QVariant variant = qc;
    QString colcode  = variant.toString();
    bkgdColorPatch[VIEW_STATUS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    // fill data
    FillData & fd = workspace->getFillData();
    int xMin,xMax,yMin,yMax;
    fd.get(xMin ,xMax,yMin,yMax);
    xRepMin[VIEW_STATUS]->setValue(xMin);
    xRepMax[VIEW_STATUS]->setValue(xMax);
    yRepMin[VIEW_STATUS]->setValue(yMin);
    yRepMax[VIEW_STATUS]->setValue(yMax);
}

void page_canvasSettings::onEnter()
{
    display();
}

void page_canvasSettings::display()
{
    blockPage(true);

    // mosaic/design settings;
    WorkspaceSettings & cSettings = getMosaicOrDesignSettings();
    // size
    QSizeF sz = cSettings.getSize();
    sizeW[DESIGN_SETTINGS]->setValue(sz.width());
    sizeH[DESIGN_SETTINGS]->setValue(sz.height());

    // background color
    QColor qc = cSettings.getBackgroundColor();
    bkColorEdit[DESIGN_SETTINGS]->setText(qc.name(QColor::HexArgb));
    QVariant variant = qc;
    QString colcode  = variant.toString();
    bkgdColorPatch[DESIGN_SETTINGS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    // start tile
    QPointF  pt = cSettings.getStartTile();
    startEditX[DESIGN_SETTINGS]->setValue(pt.x());
    startEditY[DESIGN_SETTINGS]->setValue(pt.y());

    int xMin,xMax,yMin,yMax;
    cSettings.getFillData().get(xMin ,xMax,yMin,yMax);
    xRepMin[DESIGN_SETTINGS]->setValue(xMin);
    xRepMax[DESIGN_SETTINGS]->setValue(xMax);
    yRepMin[DESIGN_SETTINGS]->setValue(yMin);
    yRepMax[DESIGN_SETTINGS]->setValue(yMax);
    displayBkgdImgSettings(cSettings.getBkgdImage(),DESIGN_SETTINGS);

    // frame settings
    FrameSettings & vsettings = workspace->getFrameSettings(config->viewerType);
    QSize isz = vsettings.getFrameSize();
    sizeW[FRAME_SETTINGS]->setValue(isz.width());
    sizeH[FRAME_SETTINGS]->setValue(isz.height());

    // tiling settings
    TilingPtr tiling = workspace->getCurrentTiling();
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

    if (config->insightMode)
    {
        displayBkgdImgSettings(tiling->getBackground(),TILING_SETTINGS);
#if 0
        // border
        BorderPtr bp = cSettings.getBorder();
        displayBorder(bp,DESIGN_SETTINGS);

        bp = cSettingsv.getBorder();
        displayBorder(bp,CANVAS_SETTINGS);
#endif
    }

    blockPage(false);
}

void page_canvasSettings::designSizeChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeW[DESIGN_SETTINGS]->value(),sizeH[DESIGN_SETTINGS]->value());

    WorkspaceSettings & cSettings = getMosaicOrDesignSettings();
    cSettings.setSize(sz);

    emit sig_viewWS();
}

void page_canvasSettings::frameSizeChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeW[FRAME_SETTINGS]->value(),sizeH[FRAME_SETTINGS]->value());

    workspace->setFrameSize(config->viewerType,sz);

    emit sig_viewWS();
}

void page_canvasSettings::viewSizeChanged(int)
{
    if (pageBlocked()) return;

    QSize sz = QSize(sizeW[VIEW_STATUS]->value(),sizeH[VIEW_STATUS]->value());

    workspace->setActiveSize(config->viewerType,sz);
    if (config->scaleToView)
    {
        workspace->setFrameSize(config->viewerType,sz);
    }
    emit sig_viewWS();
}

WorkspaceSettings & page_canvasSettings::getMosaicOrDesignSettings()
{
    static WorkspaceSettings dummy;
    if (config->viewerType == VIEW_DESIGN)
    {
        QVector<DesignPtr> & designs = workspace->getDesigns();
        if (designs.count())
        {
            DesignPtr dp = designs.first();
            return dp->getDesignInfo();
        }
        else
        {
            return dummy;
        }
    }
    else
    {
        return workspace->getMosaicSettings();
    }
}

// match design to view
void page_canvasSettings::slot_matchDesign()
{
    QSize size = workspace->size();
    WorkspaceSettings & cSettings = getMosaicOrDesignSettings();
    cSettings.setSize(size);

    workspace->setAllMosaicActiveSizes(size);
    if  (config->scaleToView)
    {
        workspace->setAllMosaicFrameSizes(size);
    }
    emit sig_viewWS();
}


void page_canvasSettings::slot_matchTiling()
{
    QSize size = workspace->size();
    TilingPtr tiling = workspace->getCurrentTiling();
    tiling->setSize(size);

    workspace->setAllTilingActiveSizes(size);
    if  (config->scaleToView)
    {
        workspace->setAllTilingFrameSizes(size);
    }

    emit sig_viewWS();
}

void page_canvasSettings::slot_set_repsDesign(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    FillData fd;
    fd.set(xRepMin[DESIGN_SETTINGS]->value(), xRepMax[DESIGN_SETTINGS]->value(), yRepMin[DESIGN_SETTINGS]->value(), yRepMax[DESIGN_SETTINGS]->value());

    WorkspaceSettings & cSettings = getMosaicOrDesignSettings();
    cSettings.setFillData(fd);

    workspace->setFillData(fd);

    emit sig_render();
}

void page_canvasSettings::slot_set_repsTiling(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    FillData fd;
    fd.set(xRepMin[TILING_SETTINGS]->value(), xRepMax[TILING_SETTINGS]->value(), yRepMin[TILING_SETTINGS]->value(), yRepMax[TILING_SETTINGS]->value());

    TilingPtr tiling = workspace->getCurrentTiling();
    Q_ASSERT(tiling);

    tiling->setFillData(fd);
    workspace->setFillData(fd);

    emit sig_render();
}

void page_canvasSettings::backgroundColorDesignPick()
{
    WorkspaceSettings & cSettings = getMosaicOrDesignSettings();
    QColor color = cSettings.getBackgroundColor();

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

void page_canvasSettings::backgroundColorDesignChanged(const QString & str)
{
    if (pageBlocked()) return;

    QColor color = QColor(str);
    WorkspaceSettings & cSettings = getMosaicOrDesignSettings();
    cSettings.setBackgroundColor(color);
    emit sig_viewWS();
}

void page_canvasSettings::displayBkgdImgSettings(BkgdImgPtr bi, eSettingsGroup group)
{
    // display background
    if (bi)
    {
        Xform xf = bi->getCanvasXform();
        bkgdLayout[group]->setX(xf.getTranslateX());
        bkgdLayout[group]->setY(xf.getTranslateY());
        bkgdLayout[group]->setScale(xf.getScale());
        bkgdLayout[group]->setRot(xf.getRotateDegrees());

        bkgdImage[group]->setText(bi->bkgdName);

        chk_showBkgd[group]->setChecked(bi->bShowBkgd);
        chk_adjustBkgd[group]->setChecked(bi->bAdjustPerspective);
    }
    else
    {
        bkgdImage[group]->setText("none");
    }

}

void page_canvasSettings::slot_loadMosaicBackground()
{
    QString bkgdDir = config->rootMediaDir + "bkgd_photos/";
    QString filename = QFileDialog::getOpenFileName(nullptr,"Select image file",bkgdDir, "Image Files (*.png *.jpg *.bmp *.heic)");
    if (filename.isEmpty()) return;

    // load
    BkgdImgPtr bi = make_shared<BackgroundImage>();
    if (bi)
    {
        if ( bi->loadAndCopy(filename))
        {
            WorkspaceSettings & cSettings = getMosaicOrDesignSettings();
            cSettings.setBkgdImage(bi);
            bi->bkgdImageChanged(true,false);  // TODO use checkboxes

            display();
        }
    }
}

void page_canvasSettings::slot_loadTilingBackground()
{
    QString bkgdDir = config->rootMediaDir + "bkgd_photos/";
    QString filename = QFileDialog::getOpenFileName(nullptr,"Select image file",bkgdDir, "Image Files (*.png *.jpg *.bmp *.heic)");
    if (filename.isEmpty()) return;

    // load
    BkgdImgPtr bi = make_shared<BackgroundImage>();
    if (bi)
    {
        if ( bi->loadAndCopy(filename))
        {
            TilingPtr tp = workspace->getCurrentTiling();
            tp->setBackground(bi);

            bi->bkgdImageChanged(true,false);  // TODO use checkboxes

            display();
        }
    }
}

void page_canvasSettings::slot_setBkgdMosaic()
{
    WorkspaceSettings & cSettings = workspace->getMosaicSettings();

    BkgdImgPtr bi = cSettings.getBkgdImage();
    if (bi)
    {
        Xform xf = bi->getCanvasXform();
        xf.setTransform(bkgdLayout[DESIGN_SETTINGS]->getQTransform());
        bi->updateCanvasXform(xf);
        bi->bkgdImageChanged(chk_showBkgd[DESIGN_SETTINGS]->isChecked(),
                         chk_adjustBkgd[DESIGN_SETTINGS]->isChecked());
        emit sig_viewWS();
    }
}

void page_canvasSettings::slot_setBkgdTiling()
{
    BkgdImgPtr bi = workspace->getCurrentTiling()->getBackground();
    if (bi)
    {
        Xform xf = bi->getCanvasXform();
        xf.setTransform(bkgdLayout[TILING_SETTINGS]->getQTransform());
        bi->updateCanvasXform(xf);
        bi->bkgdImageChanged(chk_showBkgd[TILING_SETTINGS]->isChecked(),
                         chk_adjustBkgd[TILING_SETTINGS]->isChecked());
        emit sig_viewWS();
    }
}

void page_canvasSettings::slot_setBkgdMosaicXform()
{
    WorkspaceSettings & cSettings = workspace->getMosaicSettings();

    BkgdImgPtr bi = cSettings.getBkgdImage();
    if (bi)
    {
        Xform xf = bi->getCanvasXform();
        xf.setTransform(bkgdLayout[DESIGN_SETTINGS]->getQTransform());
        bi->updateCanvasXform(xf);
    }
}

void page_canvasSettings::slot_setBkgdTilingXform()
{
    BkgdImgPtr bi = workspace->getCurrentTiling()->getBackground();
    if (bi)
    {
        Xform xf = bi->getCanvasXform();
        xf.setTransform(bkgdLayout[TILING_SETTINGS]->getQTransform());
        bi->updateCanvasXform(xf);
    }
}

void page_canvasSettings::slot_tilingSizeChanged(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    TilingPtr tp = workspace->getCurrentTiling();
    tp->setSize(QSize(sizeW[TILING_SETTINGS]->value(),sizeH[TILING_SETTINGS]->value()));

    emit sig_viewWS();
}

void page_canvasSettings::slot_adjustBackground()
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

    if (config->viewerType == VIEW_TILING_MAKER)
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


void page_canvasSettings::slot_saveAdjustedBackground()
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

void page_canvasSettings::displayBorder(BorderPtr bp, eSettingsGroup group)
{
    if (bp)
    {
        // common
        int index = borderType[group].findData(bp->getType());
        borderType[group].blockSignals(true);
        borderType[group].setCurrentIndex(index);
        borderType[group].blockSignals(false);

        qreal w = bp->getWidth();
        borderWidth[group]->setText(QString::number(w));
        borderWidth[group]->show();

        borderColorLabel[group][0]->show();

        QColor qc = bp->getColor();
        borderColor[group][0]->setText(qc.name(QColor::HexArgb));
        borderColor[group][0]->show();

        QVariant variant = qc;
        QString colcode  = variant.toString();
        borderColorPatch[group][0]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
        borderColorPatch[group][0]->show();

        if (bp->getType() == BORDER_TWO_COLOR)
        {
            borderColorLabel[group][1]->show();

            BorderTwoColor * bp2 = dynamic_cast<BorderTwoColor*>(bp.get());
            qc = bp2->getColor2();
            borderColor[group][1]->setText(qc.name(QColor::HexArgb));
            borderColor[group][1]->show();

            variant = qc;
            colcode  = variant.toString();
            borderColorPatch[group][1]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
            borderColorPatch[group][1]->show();
        }
        else
        {
            borderColorLabel[group][1]->hide();
            borderColor[group][1]->hide();
            borderColorPatch[group][1]->hide();
        }
    }
    else
    {
        int index = borderType[group].findData(BORDER_NONE);
        borderType[group].blockSignals(true);
        borderType[group].setCurrentIndex(index);
        borderType[group].blockSignals(false);

        borderColorLabel[group][0]->hide();
        borderColor[group][0]->hide();
        borderColorPatch[group][0]->hide();

        borderColorLabel[group][1]->hide();
        borderColor[group][1]->hide();
        borderColorPatch[group][1]->hide();
    }
}


void page_canvasSettings::setBorderFromForm()
{
    bool ok;
    QColor color;

    CanvasSettings & cSettings = workspace->getCurrentCanvasSettings();
    BorderPtr bp = cSettings.getBorder();
    if (bp)
    {
        qreal width = borderWidth[CANVAS_SETTINGS]->text().toDouble(&ok);
        if (ok)
        {
            bp->setWidth(width);
        }

        color.setNamedColor(borderColor[CANVAS_SETTINGS][0]->text());
        if (color.isValid())
        {
            bp->setColor(color);
        }

        BorderTwoColor * bp2 = dynamic_cast<BorderTwoColor*>(bp.get());
        if (bp2)
        {
            color.setNamedColor(borderColor[CANVAS_SETTINGS][1]->text());
            if (color.isValid())
            {
                bp2->setColor2(color);
            }
        }
    }
}

void page_canvasSettings::pickBorderColor()
{
    CanvasSettings & cSettings = workspace->getCurrentCanvasSettings();
    BorderPtr bp = cSettings.getBorder();
    if (!bp) return;
    QColor color = bp->getColor();

    AQColorDialog dlg(color,this);
    dlg.setCurrentColor(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        borderColor[CANVAS_SETTINGS][0]->setText(color.name(QColor::HexArgb));

        QVariant variant = color;
        QString colcode  = variant.toString();
        borderColorPatch[CANVAS_SETTINGS][0]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    }
}

void page_canvasSettings::pickBorderColor2()
{
    CanvasSettings & cSettings = workspace->getCurrentCanvasSettings();
    BorderPtr bp = cSettings.getBorder();
    if (!bp) return;

    BorderTwoColor * bp2 = dynamic_cast<BorderTwoColor*>(bp.get());
    if (!bp2) return;

    QColor color = bp2->getColor2();

    AQColorDialog dlg(color,this);
    dlg.setCurrentColor(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        borderColor[CANVAS_SETTINGS][1]->setText(color.name(QColor::HexArgb));

        QVariant variant = color;
        QString colcode  = variant.toString();
        borderColorPatch[CANVAS_SETTINGS][1]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    }
}

void page_canvasSettings::borderChanged(int row)
{
    Q_UNUSED(row)

    eBorderType type = static_cast<eBorderType>(borderType[CANVAS_SETTINGS].currentData().toInt());

    BorderPtr bp;
    switch(type)
    {
    case BORDER_NONE:
        break;
    case BORDER_PLAIN:
        bp = make_shared<BorderPlain>(20,Qt::blue);
        break;
    case BORDER_TWO_COLOR:
        bp = make_shared<BorderTwoColor>(QColor(0xa2,0x79,0x67),QColor(TileWhite),20);
        break;
    case BORDER_BLOCKS:
        bp = make_shared<BorderBlocks>(QColor(0xa2,0x79,0x67),150,11,6);
        break;
    }

    CanvasSettings & cSettings = workspace->getCurrentCanvasSettings();
    cSettings.setBorder(bp);
}
#endif
