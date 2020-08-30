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

#include "page_canvasSettings.h"
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
    view = View::getInstance();

    QPushButton  *pbRefresh     = new QPushButton("Refresh");
    QPushButton  *pbMatchDesign = new QPushButton("Match Design to View");
    QPushButton  *pbMatchTiling = new QPushButton("Match Tiling to View");

    QHBoxLayout * sourcebox = new QHBoxLayout();
    sourcebox->addStretch();
    sourcebox->addWidget(pbMatchDesign);
    sourcebox->addWidget(pbMatchTiling);
    sourcebox->addWidget(pbRefresh);
    sourcebox->addStretch();

    QGridLayout * pgrid  = new QGridLayout();
    QGroupBox * box;

    box = createDesignSettings();
    pgrid->addWidget(box,0,0);

    box = createTilingSettings();
    pgrid->addWidget(box,1,0);

    box = createViewSettings();
    pgrid->addWidget(box,2,0);

    box = createWorkspaceStatus();
    pgrid->addWidget(box,0,1);

    box = createViewStatus();
    pgrid->addWidget(box,2,1);

    //  mosaic background image
    QGroupBox * sbox = createBackgroundImageGroup(DESIGN_SETTINGS,"Design Background Image");

    // putting it all together
    vbox->addLayout(sourcebox);
    vbox->addLayout(pgrid);
    vbox->addWidget(sbox);
    if (config->nerdMode)
    {
        //  tiling
        QGroupBox * tbox = createBackgroundImageGroup(TILING_SETTINGS,"Tiling Background Image");

        // canvas background image
        QGroupBox * cbox = createBackgroundImageGroup(WSVIEWER_STATUS,"Viewer Background Image");
#if 0
        box = createDesignBorderSettings();
        pgrid->addWidget(box,3,0);

        box = createCanvasBorderSettings();
        pgrid->addWidget(box,3,1);

#endif
        vbox->addWidget(tbox);
        vbox->addWidget(cbox);
    }

    adjustSize();

    // connections

    connect(pbRefresh,        &QPushButton::clicked,            this,    &page_canvasSettings::display);
    connect(pbMatchDesign,    &QPushButton::clicked,            this,    &page_canvasSettings::slot_matchDesign);
    connect(pbMatchTiling,    &QPushButton::clicked,            this,    &page_canvasSettings::slot_matchTiling);

    connect(wsViewer,         &WorkspaceViewer::sig_viewUpdated,this,    &page_canvasSettings::display);

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
    viewW[TILING_SETTINGS] = new SpinSet("width",0,1,4096);
    viewH[TILING_SETTINGS] = new SpinSet("height",0,1,2160);


    QGridLayout * grid = new QGridLayout();
    grid->addLayout(viewW[TILING_SETTINGS],0,0);
    grid->addLayout(viewH[TILING_SETTINGS],0,1);

    QGroupBox * box = new QGroupBox("Tiling Settings");
    box->setAlignment(Qt::AlignTop);
    box->setLayout(grid);

    connect(viewW[TILING_SETTINGS], &SpinSet::valueChanged, this, &page_canvasSettings::slot_tilingSizeChanged);
    connect(viewH[TILING_SETTINGS], &SpinSet::valueChanged, this, &page_canvasSettings::slot_tilingSizeChanged);

    return box;
}

QGroupBox *page_canvasSettings::createDesignSettings()
{
    sizeEditW[DESIGN_SETTINGS]      = new SpinSet("width",0,1,4096);
    sizeEditH[DESIGN_SETTINGS]      = new SpinSet("height",0,1,2160);
    startEditX[DESIGN_SETTINGS]     = new DoubleSpinSet("start-X",0,-4096,4096);
    startEditY[DESIGN_SETTINGS]     = new DoubleSpinSet("start-y",0,-2160,2160);
    bkColorEdit[DESIGN_SETTINGS]    = new QLineEdit;
    bkgdColorPatch[DESIGN_SETTINGS] = new ClickableLabel;

    QHBoxLayout * hbox1 = new QHBoxLayout;
    QHBoxLayout * hbox2 = new QHBoxLayout;

    const int rmin = -1000;
    const int rmax =  1000;

    xRepMin[DESIGN_SETTINGS] = new SpinSet("xMin",0,rmin,rmax);
    xRepMax[DESIGN_SETTINGS] = new SpinSet("xMax",0,rmin,rmax);
    yRepMin[DESIGN_SETTINGS] = new SpinSet("yMin",0,rmin,rmax);
    yRepMax[DESIGN_SETTINGS] = new SpinSet("yMax",0,rmin,rmax);
    QPushButton * pb =  new QPushButton("Set");

    hbox1->addLayout(xRepMin[DESIGN_SETTINGS]);
    hbox1->addLayout(xRepMax[DESIGN_SETTINGS]);
    hbox1->addStretch();

    hbox2->addLayout(yRepMin[DESIGN_SETTINGS]);
    hbox2->addLayout(yRepMax[DESIGN_SETTINGS]);
    hbox2->addWidget(pb);
    hbox2->addStretch();

    connect(sizeEditW[DESIGN_SETTINGS],      &SpinSet::valueChanged,  this, &page_canvasSettings::designSizeChanged);
    connect(sizeEditH[DESIGN_SETTINGS],      &SpinSet::valueChanged,  this, &page_canvasSettings::designSizeChanged);
    connect(bkgdColorPatch[DESIGN_SETTINGS], &ClickableLabel::clicked,this, &page_canvasSettings::backgroundColorDesignPick);
    connect(bkColorEdit[DESIGN_SETTINGS],    &QLineEdit::textChanged, this, &page_canvasSettings::backgroundColorDesignChanged);
    connect(pb,                              &QPushButton::clicked,   this, &page_canvasSettings::slot_set_repsDesign);

    QGridLayout * grid = new QGridLayout();
    grid->addLayout(sizeEditW[DESIGN_SETTINGS],0,0);
    grid->addLayout(sizeEditH[DESIGN_SETTINGS],0,1);
    grid->addLayout(startEditX[DESIGN_SETTINGS],1,0);
    grid->addLayout(startEditY[DESIGN_SETTINGS],1,1);
    grid->addWidget(bkColorEdit[DESIGN_SETTINGS],2,0);
    grid->addWidget(bkgdColorPatch[DESIGN_SETTINGS],2,1);
    grid->addLayout(hbox1,3,0,1,2);
    grid->addLayout(hbox2,4,0,1,2);

    QGroupBox * box = new QGroupBox("Design Settings");
    box->setLayout(grid);
    return box;
}

QGroupBox * page_canvasSettings::createWorkspaceStatus()
{
    sizeEditW[WSVIEWER_STATUS]      = new SpinSet("width",0,1,4096);
    sizeEditH[WSVIEWER_STATUS]      = new SpinSet("height",0,1,2160);
    startEditX[WSVIEWER_STATUS]     = new DoubleSpinSet("start-X",0,-4096,4096);
    startEditY[WSVIEWER_STATUS]     = new DoubleSpinSet("start-y",0,-2160,2160);
    bkColorEdit[WSVIEWER_STATUS]    = new QLineEdit;
    bkgdColorPatch[WSVIEWER_STATUS] = new ClickableLabel;
    QLabel * l1 = new QLabel("");
    QLabel * l2 = new QLabel("");

    sizeEditW[WSVIEWER_STATUS]->setReadOnly(true);
    sizeEditH[WSVIEWER_STATUS]->setReadOnly(true);
    startEditX[WSVIEWER_STATUS]->setReadOnly(true);
    startEditY[WSVIEWER_STATUS]->setReadOnly(true);
    bkColorEdit[WSVIEWER_STATUS]->setReadOnly(true);

    QGridLayout * grid = new QGridLayout();
    grid->addLayout(sizeEditW[WSVIEWER_STATUS],0,0);
    grid->addLayout(sizeEditH[WSVIEWER_STATUS],0,1);
    grid->addLayout(startEditX[WSVIEWER_STATUS],1,0);
    grid->addLayout(startEditY[WSVIEWER_STATUS],1,1);
    grid->addWidget(bkColorEdit[WSVIEWER_STATUS],2,0);
    grid->addWidget(bkgdColorPatch[WSVIEWER_STATUS],2,1);
    grid->addWidget(l1,3,0);
    grid->addWidget(l2,4,0);

    QGroupBox * box = new QGroupBox("Workspace Status");
    box->setLayout(grid);
    return box;
}

QGroupBox *page_canvasSettings::createViewSettings()
{
    viewW[VIEW_SETTINGS] = new SpinSet("width",0,1,4096);
    viewH[VIEW_SETTINGS] = new SpinSet("height",0,1,2160);

    connect(viewW[VIEW_SETTINGS], &SpinSet::valueChanged, this, &page_canvasSettings::viewSizeChanged);
    connect(viewH[VIEW_SETTINGS], &SpinSet::valueChanged, this, &page_canvasSettings::viewSizeChanged);

    QGridLayout * grid = new QGridLayout();
    grid->addLayout(viewW[VIEW_SETTINGS],1,0);
    grid->addLayout(viewH[VIEW_SETTINGS],1,1);

    viewBox = new QGroupBox("View Settings");
    viewBox->setLayout(grid);
    return viewBox;
}

QGroupBox *page_canvasSettings::createViewStatus()
{
    viewW[VIEW_STATUS] = new SpinSet("width",0,1,4096);
    viewH[VIEW_STATUS] = new SpinSet("height",0,1,2160);

    viewW[VIEW_STATUS]->setReadOnly(true);
    viewH[VIEW_STATUS]->setReadOnly(true);

    QGridLayout * grid = new QGridLayout();
    grid->addLayout(viewW[VIEW_STATUS],0,0);
    grid->addLayout(viewH[VIEW_STATUS],0,1);

    QGroupBox * box = new QGroupBox("View Status");
    box->setAlignment(Qt::AlignTop);
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
    chk_xformBkgd[group]  = new QCheckBox("Xform");
    hbox->addWidget(bkgdImage[group]);
    hbox->addWidget(bkgdImageBtn[group]);
    hbox->addWidget(chk_showBkgd[group]);
    hbox->addWidget(chk_xformBkgd[group]);
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
        connect(chk_xformBkgd[group], &QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgdMosaic);
    }

    if (group == TILING_SETTINGS)
    {
        connect(bkgdImageBtn[group],  &QPushButton::clicked,            this,    &page_canvasSettings::slot_loadTilingBackground);
        connect(bkgdLayout[group],    &LayoutTransform::xformChanged,   this,    &page_canvasSettings::slot_setBkgdTilingXform);
        connect(chk_showBkgd[group],  &QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgdTiling);
        connect(chk_adjustBkgd[group],&QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgdTiling);
        connect(chk_xformBkgd[group], &QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgdTiling);
    }

    if (group == WSVIEWER_STATUS)
    {
        bkgdImageBtn[group]->hide();
        connect(bkgdLayout[group],    &LayoutTransform::xformChanged,   this,    &page_canvasSettings::slot_setBkdViewerXform);
        connect(chk_showBkgd[group],  &QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgdViewer);
        connect(chk_adjustBkgd[group],&QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgdViewer);
        connect(chk_xformBkgd[group], &QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgdViewer);
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

#if 0
QGroupBox *page_canvasSettings::createCanvasBorderSettings()
{
    QGroupBox * bbox = new QGroupBox("Canvas Border Settings");
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

    return bbox;
}

QGroupBox *page_canvasSettings::createDesignBorderSettings()
{
    QGroupBox * bbox = new QGroupBox("Design Border Settings");
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

    return bbox;
}
#endif

void  page_canvasSettings::refreshPage()
{
    // Workspace viewer status
    CanvasSettings & cSettingsv = wsViewer->getCurrentCanvasSettings();
    displaySettings(cSettingsv, WSVIEWER_STATUS);
    if (config->nerdMode)
    {
        displayBkgdImgSettings(cSettingsv.getBkgdImage(),WSVIEWER_STATUS);
    }

    // View Status
    QSize size  = view->size();
    viewW[VIEW_STATUS]->setValue(size.width());
    viewH[VIEW_STATUS]->setValue(size.height());
    viewBox->setTitle(QString("View Settings : %1").arg(sViewerType[config->viewerType]));
}

void page_canvasSettings::onEnter()
{
    display();
}

void page_canvasSettings::display()
{
    // mosaic/design settings;
    CanvasSettings & cSettings = getMosaicOrDesignSettings();
    displaySettings(cSettings, DESIGN_SETTINGS);
    displayBkgdImgSettings(cSettings.getBkgdImage(),DESIGN_SETTINGS);

    // view settings
    ViewSettings & vsettings = wsViewer->getViewSettings(config->viewerType);
    QSize sz = vsettings.getViewSize();
    viewW[VIEW_SETTINGS]->setValue(sz.width());
    viewH[VIEW_SETTINGS]->setValue(sz.height());

    // tiling settings
    TilingPtr tiling = workspace->getCurrentTiling();
    QSize size  = tiling->getCanvasSize();
    viewW[TILING_SETTINGS]->setValue(size.width());
    viewH[TILING_SETTINGS]->setValue(size.height());

    if (config->nerdMode)
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
}

void page_canvasSettings::displaySettings(CanvasSettings & cSettings, eSettingsGroup group)
{
    // size
    QSizeF sz = cSettings.getCanvasSize();
    sizeEditW[group]->setValue(sz.width());
    sizeEditH[group]->setValue(sz.height());

    // background color
    QColor qc = cSettings.getBackgroundColor();
    bkColorEdit[group]->setText(qc.name(QColor::HexArgb));
    QVariant variant = qc;
    QString colcode  = variant.toString();
    bkgdColorPatch[group]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    // start tile
    QPointF  pt = cSettings.getStartTile();
    startEditX[group]->setValue(pt.x());
    startEditY[group]->setValue(pt.y());

    if (group == DESIGN_SETTINGS)
    {
        int xMin,xMax,yMin,yMax;
        cSettings.getFillData().get(xMin ,xMax,yMin,yMax);
        xRepMin[group]->setValue(xMin);
        xRepMax[group]->setValue(xMax);
        yRepMin[group]->setValue(yMin);
        yRepMax[group]->setValue(yMax);
    }
}

void page_canvasSettings::designSizeChanged(int)
{
    CanvasSettings & cSettings = getMosaicOrDesignSettings();
    QSize sz = QSize(sizeEditW[DESIGN_SETTINGS]->value(),sizeEditH[DESIGN_SETTINGS]->value());
    cSettings.setCanvasSize(sz);
    emit sig_viewWS();
}

void page_canvasSettings::viewSizeChanged(int)
{
    ViewSettings & vSettings = wsViewer->getViewSettings(config->viewerType);
    QSize sz = QSize(sizeEditW[WSVIEWER_STATUS]->value(),sizeEditH[WSVIEWER_STATUS]->value());
    vSettings.setViewSize(sz);
    emit sig_viewWS();
}

CanvasSettings & page_canvasSettings::getMosaicOrDesignSettings()
{
    static CanvasSettings dummy;
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
    QSize size = view->size();
    CanvasSettings & cSettings = getMosaicOrDesignSettings();
    cSettings.setCanvasSize(size);
    emit sig_viewWS();
}


void page_canvasSettings::slot_matchTiling()
{
    QSize size = view->size();
    TilingPtr tiling = workspace->getCurrentTiling();
    tiling->setCanvasSize(size);
    emit sig_viewWS();
}

void page_canvasSettings::slot_set_repsDesign()
{
    FillData fd;
    fd.set(xRepMin[DESIGN_SETTINGS]->value(), xRepMax[DESIGN_SETTINGS]->value(), yRepMin[DESIGN_SETTINGS]->value(), yRepMax[DESIGN_SETTINGS]->value());

    CanvasSettings & cSettings = getMosaicOrDesignSettings();
    cSettings.setFillData(fd);

    emit sig_render();
}

void page_canvasSettings::backgroundColorDesignPick()
{
    CanvasSettings & cSettings = getMosaicOrDesignSettings();
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
    QColor color = QColor(str);
    CanvasSettings & cSettings = getMosaicOrDesignSettings();
    cSettings.setBackgroundColor(color);
    emit sig_viewWS();
}

void page_canvasSettings::displayBkgdImgSettings(BkgdImgPtr bi, eSettingsGroup group)
{
    // display background
    if (bi)
    {
        Xform xf = bi->getXform();
        bkgdLayout[group]->setX(xf.getTranslateX());
        bkgdLayout[group]->setY(xf.getTranslateY());
        bkgdLayout[group]->setScale(xf.getScale());
        bkgdLayout[group]->setRot(xf.getRotateDegrees());

        bkgdImage[group]->setText(bi->bkgdName);

        chk_showBkgd[group]->setChecked(bi->bShowBkgd);
        chk_xformBkgd[group]->setChecked(bi->bTransformBkgd);
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
            CanvasSettings & cSettings = getMosaicOrDesignSettings();
            cSettings.setBkgdImage(bi);

            bi->bkgdImageChanged(true,false,true);  // TODO use checkboxes

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

            bi->bkgdImageChanged(true,false,true);  // TODO use checkboxes

            display();
        }
    }
}

void page_canvasSettings::slot_setBkgdMosaic()
{
    CanvasSettings & cSettings = workspace->getMosaicSettings();

    BkgdImgPtr bi = cSettings.getBkgdImage();
    if (bi)
    {
        Xform xf = bi->getXform();
        xf.setTransform(bkgdLayout[DESIGN_SETTINGS]->getQTransform());
        bi->setXform(xf);
        bi->bkgdImageChanged(chk_showBkgd[DESIGN_SETTINGS]->isChecked(),
                         chk_adjustBkgd[DESIGN_SETTINGS]->isChecked(),
                         chk_xformBkgd[DESIGN_SETTINGS]->isChecked());
        emit sig_viewWS();
    }
}

void page_canvasSettings::slot_setBkgdTiling()
{
    BkgdImgPtr bi = workspace->getCurrentTiling()->getBackground();
    if (bi)
    {
        Xform xf = bi->getXform();
        xf.setTransform(bkgdLayout[TILING_SETTINGS]->getQTransform());
        bi->setXform(xf);
        bi->bkgdImageChanged(chk_showBkgd[TILING_SETTINGS]->isChecked(),
                         chk_adjustBkgd[TILING_SETTINGS]->isChecked(),
                         chk_xformBkgd[TILING_SETTINGS]->isChecked());
        emit sig_viewWS();
    }
}

void page_canvasSettings::slot_setBkgdViewer()
{
    CanvasSettings & cSettings = wsViewer->getCurrentCanvasSettings();
    BkgdImgPtr bi = cSettings.getBkgdImage();
    if (bi)
    {
        Xform xf = bi->getXform();
        xf.setTransform(bkgdLayout[WSVIEWER_STATUS]->getQTransform());
        bi->setXform(xf);
        bi->bkgdImageChanged(chk_showBkgd[WSVIEWER_STATUS]->isChecked(),
                         chk_adjustBkgd[WSVIEWER_STATUS]->isChecked(),
                         chk_xformBkgd[WSVIEWER_STATUS]->isChecked());
        emit sig_viewWS();
    }
}

void page_canvasSettings::slot_setBkgdMosaicXform()
{
    CanvasSettings & cSettings = workspace->getMosaicSettings();

    BkgdImgPtr bi = cSettings.getBkgdImage();
    if (bi)
    {
        Xform xf = bi->getXform();
        xf.setTransform(bkgdLayout[DESIGN_SETTINGS]->getQTransform());
        bi->setXform(xf);
        bi->bkgdTransformChanged(chk_xformBkgd[DESIGN_SETTINGS]->isChecked());
    }
}

void page_canvasSettings::slot_setBkgdTilingXform()
{
    BkgdImgPtr bi = workspace->getCurrentTiling()->getBackground();
    if (bi)
    {
        Xform xf = bi->getXform();
        xf.setTransform(bkgdLayout[TILING_SETTINGS]->getQTransform());
        bi->setXform(xf);
        bi->bkgdTransformChanged(chk_xformBkgd[TILING_SETTINGS]->isChecked());
    }
}


void page_canvasSettings::slot_setBkdViewerXform()
{
    CanvasSettings & cSettings = wsViewer->getCurrentCanvasSettings();
    BkgdImgPtr bi = cSettings.getBkgdImage();
    if (bi)
    {
        Xform xf = bi->getXform();
        xf.setTransform(bkgdLayout[WSVIEWER_STATUS]->getQTransform());
        bi->setXform(xf);
        bi->bkgdTransformChanged(chk_xformBkgd[WSVIEWER_STATUS]->isChecked());
    }
}


void page_canvasSettings::slot_tilingSizeChanged(int val)
{
    Q_UNUSED(val);

    TilingPtr tp = workspace->getCurrentTiling();
    tp->setCanvasSize(QSize(viewW[TILING_SETTINGS]->value(),viewH[TILING_SETTINGS]->value()));

    emit sig_viewWS();
}

#if 0


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
#endif

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


#if 0
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

    CanvasSettings & cSettings = wsViewer->getCurrentCanvasSettings();
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
    CanvasSettings & cSettings = wsViewer->getCurrentCanvasSettings();
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
    CanvasSettings & cSettings = wsViewer->getCurrentCanvasSettings();
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

    CanvasSettings & cSettings = wsViewer->getCurrentCanvasSettings();
    cSettings.setBorder(bp);
}
#endif
