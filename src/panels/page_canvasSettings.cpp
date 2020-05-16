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
#include "tile/Tiling.h"
#include "tapp/Prototype.h"
#include "style/Sketch.h"
#include "base/canvas.h"
#include "base/border.h"
#include "base/tiledpatternmaker.h"
#include "base/utilities.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "panels/panel.h"
#include "viewers/workspaceviewer.h"

using std::string;

page_canvasSettings::page_canvasSettings(ControlPanel * apanel)  : panel_page(apanel,"Canvas Settings")
{
    QLabel * label            = new QLabel("Source:");
    QRadioButton *radioStyle  = new QRadioButton("Style");
    QRadioButton *radioWS     = new QRadioButton("Workspace");
    QPushButton * pbRefresh   = new QPushButton("Refresh");

    QHBoxLayout * sourcebox = new QHBoxLayout();
    sourcebox->addWidget(label);
    sourcebox ->addWidget(radioStyle);
    sourcebox ->addWidget(radioWS);
    sourcebox->addStretch();
    sourcebox->addWidget(pbRefresh);
    sourcebox->addStretch();

    QGridLayout * pgrid  = new QGridLayout();
    QGroupBox * box;

    box = createDesignSettings();
    pgrid->addWidget(box,0,0);

    box = createCanvasSettings();
    pgrid->addWidget(box,1,0);

    box = createSceneStatus();
    pgrid->addWidget(box,0,1);

    box = createViewSettings();
    pgrid->addWidget(box,2,0);

    box = createViewStatus();
    pgrid->addWidget(box,1,1);

#ifdef BORDER
    row++;
    // border
    label = new QLabel("Border");
    grid->addWidget(label,row,0);

    borderType.addItem("No border",BORDER_NONE);
    borderType.addItem("Solid border",BORDER_PLAIN);
    borderType.addItem("Two color border",BORDER_TWO_COLOR);
    borderType.addItem("Shaped border",BORDER_BLOCKS);
    grid->addWidget(&borderType,row,1);

    row++;

    label = new QLabel("Border Width");
    grid->addWidget(label,row,0);
    borderWidth = new QLineEdit();
    grid->addWidget(borderWidth,row,1);
    row++;

    borderColorLabel[0] = new QLabel("Border Color");
    grid->addWidget(borderColorLabel[0],row,0);
    borderColor[0] = new QLineEdit();
    grid->addWidget(borderColor[0],row,1);
    borderColorPatch[0] = new ClickableLabel;
    borderColorPatch[0]->setMinimumWidth(50);
    grid->addWidget(borderColorPatch[0],row,2);
    row++;

    borderColorLabel[1] = new QLabel("Border Color 2");
    grid->addWidget(borderColorLabel[1],row,0);
    borderColor[1] = new QLineEdit();
    grid->addWidget(borderColor[1],row,1);
    borderColorPatch[1] = new ClickableLabel;
    borderColorPatch[1]->setMinimumWidth(50);
    grid->addWidget(borderColorPatch[1],row,2);
    row++;
#endif
    //  source background image
    QGroupBox * sbox = new QGroupBox("Design Background Image");
    QVBoxLayout * svlayout = new QVBoxLayout;
    sbox->setLayout(svlayout);

    QHBoxLayout * hbox = new QHBoxLayout;
    bkgdImageBtn       = new QPushButton("Select");
    bkgdImage[SDESIGN_SETTINGS]       = new QLineEdit();
    chk_showBkgd[SDESIGN_SETTINGS]    = new QCheckBox("Show Background");
    chk_adjustBkgd[SDESIGN_SETTINGS]  = new QCheckBox("Perspective");
    chk_xformBkgd[SDESIGN_SETTINGS]   = new QCheckBox("Xform");
    hbox->addWidget(bkgdImage[SDESIGN_SETTINGS]);
    hbox->addWidget(bkgdImageBtn);
    hbox->addWidget(chk_showBkgd[SDESIGN_SETTINGS]);
    hbox->addWidget(chk_xformBkgd[SDESIGN_SETTINGS]);
    hbox->addWidget(chk_adjustBkgd[SDESIGN_SETTINGS]);
    hbox->addStretch();
    svlayout->addLayout(hbox);

    bkgdLayout[SDESIGN_SETTINGS] = new LayoutTransform("Xform",5);
    svlayout->addLayout(bkgdLayout[SDESIGN_SETTINGS]);

    connect(bkgdLayout[SDESIGN_SETTINGS],    &LayoutTransform::xformChanged,   this,    &page_canvasSettings::slot_setBkgdXform);
    connect(chk_showBkgd[SDESIGN_SETTINGS],  &QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgd);
    connect(chk_adjustBkgd[SDESIGN_SETTINGS],&QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgd);
    connect(chk_xformBkgd[SDESIGN_SETTINGS], &QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgd);

    hbox = new QHBoxLayout;
    QPushButton * adjustBtn       = new QPushButton("Adjust perspective");
    QPushButton * saveAdjustedBtn = new QPushButton("Save Adjusted");
    hbox->addWidget(adjustBtn);
    hbox->addStretch();
    hbox->addWidget(saveAdjustedBtn);
    svlayout->addLayout(hbox);

    // canvas background image
    QGroupBox * cbox = new QGroupBox("Canvas Background Image");
    QVBoxLayout * cvlayout = new QVBoxLayout();
    cbox->setLayout(cvlayout);

    hbox = new QHBoxLayout;
    bkgdImage[CANVAS_SETTINGS]      = new QLineEdit();
    bkgdImage[CANVAS_SETTINGS]->setReadOnly(true);
    chk_showBkgd[CANVAS_SETTINGS]   = new QCheckBox("Show Background");
    chk_adjustBkgd[CANVAS_SETTINGS] = new QCheckBox("Perspective");
    chk_xformBkgd[CANVAS_SETTINGS]  = new QCheckBox("Xform");
    hbox->addWidget(bkgdImage[CANVAS_SETTINGS]);
    hbox->addWidget(chk_showBkgd[CANVAS_SETTINGS]);
    hbox->addWidget(chk_xformBkgd[CANVAS_SETTINGS]);
    hbox->addWidget(chk_adjustBkgd[CANVAS_SETTINGS]);
    hbox->addStretch();
    cvlayout->addLayout(hbox);

    bkgdLayout[CANVAS_SETTINGS] = new LayoutTransform("Xform",5);
    cvlayout->addLayout(bkgdLayout[CANVAS_SETTINGS]);

    connect(bkgdLayout[CANVAS_SETTINGS],    &LayoutTransform::xformChanged,   this,    &page_canvasSettings::slot_setBkgdXformCanvas);
    connect(chk_showBkgd[CANVAS_SETTINGS],  &QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgdCanvas);
    connect(chk_adjustBkgd[CANVAS_SETTINGS],&QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgdCanvas);
    connect(chk_xformBkgd[CANVAS_SETTINGS], &QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgdCanvas);

    // putting it all together
    vbox->addLayout(sourcebox);
    vbox->addLayout(pgrid);
    vbox->addWidget(sbox);
    vbox->addWidget(cbox);
    adjustSize();

    bgroup.addButton(radioStyle, CS_STYLE);
    bgroup.addButton(radioWS,    CS_WS);
    bgroup.button(config->canvasSettings)->setChecked(true);


    // connections
    WorkspaceViewer * wviewer  = WorkspaceViewer::getInstance();

    connect(bkgdImageBtn,     &QPushButton::clicked,            this,   &page_canvasSettings::slot_loadBackground);
    connect(adjustBtn,        &QPushButton::clicked,            this,    &page_canvasSettings::slot_adjustBackground);
    connect(saveAdjustedBtn,  &QPushButton::clicked,            this,    &page_canvasSettings::slot_saveAdjustedBackground);
    connect(pbRefresh,        &QPushButton::clicked,            this,    &page_canvasSettings::display);
    connect(wviewer,          &WorkspaceViewer::sig_viewUpdated,this,    &page_canvasSettings::display);


#ifdef BORDER
    connect(borderColorPatch[0],&ClickableLabel::clicked,      this, &page_canvasSettings::pickBorderColor);
    connect(borderColorPatch[1],&ClickableLabel::clicked,      this, &page_canvasSettings::pickBorderColor2);
    connect(&borderType,     SIGNAL(currentIndexChanged(int)), this, SLOT(borderChanged(int)));
#endif
    connect(&bgroup,         SIGNAL(buttonClicked(int)),       this, SLOT(settingsSelectionChanged(int)));

    connect(maker,  &TiledPatternMaker::sig_loadedTiling,      this, &page_canvasSettings::onEnter);
    connect(maker,  &TiledPatternMaker::sig_loadedXML,         this, &page_canvasSettings::onEnter);
    connect(maker,  &TiledPatternMaker::sig_loadedDesign,      this, &page_canvasSettings::onEnter);

    connect(canvas, &Canvas::sig_deltaScale,    this, &page_canvasSettings::slot_scale);
    connect(canvas, &Canvas::sig_deltaRotate,   this, &page_canvasSettings::slot_rotate);
    connect(canvas, &Canvas::sig_deltaMoveY,    this, &page_canvasSettings::slot_moveY);
    connect(canvas, &Canvas::sig_deltaMoveX,    this, &page_canvasSettings::slot_moveX);
}

QGroupBox *page_canvasSettings::createDesignSettings()
{
    sizeEditW[SDESIGN_SETTINGS]      = new DoubleSpinSet("width",0,1,4096);
    sizeEditH[SDESIGN_SETTINGS]      = new DoubleSpinSet("height",0,1,2160);
    startEditX[SDESIGN_SETTINGS]     = new DoubleSpinSet("start-X",0,-4096,4096);
    startEditY[SDESIGN_SETTINGS]     = new DoubleSpinSet("start-y",0,-2160,2160);
    bkColorEdit[SDESIGN_SETTINGS]    = new QLineEdit;
    bkgdColorPatch[SDESIGN_SETTINGS] = new ClickableLabel;

    connect(sizeEditW[SDESIGN_SETTINGS],      &DoubleSpinSet::valueChanged, this, &page_canvasSettings::designSizeChanged);
    connect(sizeEditH[SDESIGN_SETTINGS],      &DoubleSpinSet::valueChanged, this, &page_canvasSettings::designSizeChanged);
    connect(bkgdColorPatch[SDESIGN_SETTINGS], &ClickableLabel::clicked,     this, &page_canvasSettings::pickBackgroundColor);

    QGridLayout * grid = new QGridLayout();
    grid->addLayout(sizeEditW[SDESIGN_SETTINGS],0,0);
    grid->addLayout(sizeEditH[SDESIGN_SETTINGS],0,1);
    grid->addLayout(startEditX[SDESIGN_SETTINGS],1,0);
    grid->addLayout(startEditY[SDESIGN_SETTINGS],1,1);
    grid->addWidget(bkColorEdit[SDESIGN_SETTINGS],2,0);
    grid->addWidget(bkgdColorPatch[SDESIGN_SETTINGS],2,1);

    QGroupBox * box = new QGroupBox("Design Settings");
    box->setLayout(grid);
    return box;
}

QGroupBox * page_canvasSettings::createCanvasSettings()
{
    sizeEditW[CANVAS_SETTINGS]      = new DoubleSpinSet("width",0,1,4096);
    sizeEditH[CANVAS_SETTINGS]      = new DoubleSpinSet("height",0,1,2160);
    startEditX[CANVAS_SETTINGS]     = new DoubleSpinSet("start-X",0,-4096,4096);
    startEditY[CANVAS_SETTINGS]     = new DoubleSpinSet("start-y",0,-2160,2160);
    bkColorEdit[CANVAS_SETTINGS]    = new QLineEdit;
    bkgdColorPatch[CANVAS_SETTINGS] = new ClickableLabel;


    connect(sizeEditW[CANVAS_SETTINGS],      &DoubleSpinSet::valueChanged, this, &page_canvasSettings::canvasSizeChanged);
    connect(sizeEditH[CANVAS_SETTINGS],      &DoubleSpinSet::valueChanged, this, &page_canvasSettings::canvasSizeChanged);
    connect(bkgdColorPatch[CANVAS_SETTINGS], &ClickableLabel::clicked,     this, &page_canvasSettings::pickBackgroundColorCanvas);

    QGridLayout * grid = new QGridLayout();
    grid->addLayout(sizeEditW[CANVAS_SETTINGS],0,0);
    grid->addLayout(sizeEditH[CANVAS_SETTINGS],0,1);
    grid->addLayout(startEditX[CANVAS_SETTINGS],1,0);
    grid->addLayout(startEditY[CANVAS_SETTINGS],1,1);
    grid->addWidget(bkColorEdit[CANVAS_SETTINGS],2,0);
    grid->addWidget(bkgdColorPatch[CANVAS_SETTINGS],2,1);

    QGroupBox * box = new QGroupBox("Canvas Settings");
    box->setLayout(grid);
    return box;
}


QGroupBox * page_canvasSettings::createSceneStatus()
{
    sizeEditW[SCENE_STATUS]      = new DoubleSpinSet("width",0,1,4096);
    sizeEditH[SCENE_STATUS]      = new DoubleSpinSet("height",0,1,2160);
    bkColorEdit[SCENE_STATUS]    = new QLineEdit;
    bkgdColorPatch[SCENE_STATUS] = new ClickableLabel;

    sizeEditW[SCENE_STATUS]->setReadOnly(true);
    sizeEditH[SCENE_STATUS]->setReadOnly(true);
    bkColorEdit[SCENE_STATUS]->setReadOnly(true);

    QGridLayout * grid = new QGridLayout();
    grid->addLayout(sizeEditW[SCENE_STATUS],0,0);
    grid->addLayout(sizeEditH[SCENE_STATUS],0,1);
    grid->addWidget(bkColorEdit[SCENE_STATUS],1,0);
    grid->addWidget(bkgdColorPatch[SCENE_STATUS],1,1);

    QLabel * dummy = new QLabel;
    grid->addWidget(dummy,2,0);

    QGroupBox * box = new QGroupBox("Scene Status");
    box->setAlignment(Qt::AlignTop);
    box->setLayout(grid);
    return box;
}

QGroupBox *page_canvasSettings::createViewSettings()
{
    viewW[VIEW_SETTINGS] = new SpinSet("width",0,1,4096);
    viewH[VIEW_SETTINGS] = new SpinSet("height",0,1,2160);

    viewW[VIEW_SETTINGS]->setReadOnly(true);
    viewH[VIEW_SETTINGS]->setReadOnly(true);

    QGridLayout * grid = new QGridLayout();
    grid->addLayout(viewW[VIEW_SETTINGS],0,0);
    grid->addLayout(viewH[VIEW_SETTINGS],0,1);

    QGroupBox * box = new QGroupBox("View Settings");
    box->setLayout(grid);
    return box;
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

    QLabel * dummy = new QLabel;
    grid->addWidget(dummy,2,0);
    dummy = new QLabel;
    grid->addWidget(dummy,3,0);

    QGroupBox * box = new QGroupBox("View Status");
    box->setAlignment(Qt::AlignTop);
    box->setLayout(grid);
    return box;
}


void  page_canvasSettings::refreshPage()
{
    // background Images
    BkgdImgPtr bi = viewer->getBkgdImage();
    if (bi)
    {
        Xform xf = bi->getXform();
        bkgdLayout[VIEW_STATUS]->setX(xf.getTranslateX());
        bkgdLayout[VIEW_STATUS]->setY(xf.getTranslateY());
        bkgdLayout[VIEW_STATUS]->setScale(xf.getScale());
        bkgdLayout[VIEW_STATUS]->setRot(xf.getRotateDegrees());
        bkgdImage[VIEW_STATUS]->setText(bi->bkgdName);

        chk_showBkgd[VIEW_STATUS]->setChecked(bi->bShowBkgd);
        chk_xformBkgd[VIEW_STATUS]->setChecked(bi->bTransformBkgd);
        chk_adjustBkgd[VIEW_STATUS]->setChecked(bi->bAdjustPerspective);
    }
    else
    {
        bkgdImage[1]->setText("none");
    }

    // view
    View * view = View::getInstance();
    QSize size  = view->size();
    viewW[VIEW_STATUS]->setValue(size.width());
    viewH[VIEW_STATUS]->setValue(size.height());

    // Scene
    Scene * scene = canvas->currentScene();
    if (scene)
    {
        QSizeF sizeF  = scene->sceneRect().size();
        sizeEditW[SCENE_STATUS]->setValue(sizeF.width());
        sizeEditH[SCENE_STATUS]->setValue(sizeF.height());

        // background color
        QBrush br = scene->backgroundBrush();
        QColor qc = br.color();
        bkColorEdit[SCENE_STATUS]->setText(qc.name(QColor::HexArgb));

        QVariant variant = qc;
        QString colcode  = variant.toString();
        bkgdColorPatch[SCENE_STATUS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    }
}

void page_canvasSettings::settingsSelectionChanged(int idx)
{
    config->canvasSettings = static_cast<eCSSelect>(idx);
    onEnter();
}

void page_canvasSettings::onEnter()
{
    display();
}

void page_canvasSettings::display()
{
    eWsData wsdata  = (config->canvasSettings == CS_STYLE) ? WS_LOADED : WS_TILING;
    CanvasSettings & cSettings = workspace->getStyledDesign(wsdata).getCanvasSettings();
    displaySettings(cSettings, SDESIGN_SETTINGS);
    displayBkgdImgSettings(cSettings,SDESIGN_SETTINGS);

    CanvasSettings & cSettingsv = viewer->GetCanvasSettings();
    displaySettings(cSettingsv, CANVAS_SETTINGS);
    displayBkgdImgSettings(cSettingsv,CANVAS_SETTINGS);

    ViewSettings & vsettings = viewer->getViewSettings(config->viewerType);
    QSize sz = vsettings.getViewSize();
    viewW[VIEW_SETTINGS]->setValue(sz.width());
    viewH[VIEW_SETTINGS]->setValue(sz.height());

    // display background
    BkgdImgPtr bi = cSettings.getBkgdImage();
    if (bi)
    {
        Xform xf = bi->getXform();
        bkgdLayout[SDESIGN_SETTINGS]->setX(xf.getTranslateX());
        bkgdLayout[SDESIGN_SETTINGS]->setY(xf.getTranslateY());
        bkgdLayout[SDESIGN_SETTINGS]->setScale(xf.getScale());
        bkgdLayout[SDESIGN_SETTINGS]->setRot(xf.getRotateDegrees());
        bkgdImage[SDESIGN_SETTINGS]->setText(bi->bkgdName);

        chk_showBkgd[SDESIGN_SETTINGS]->setChecked(bi->bShowBkgd);
        chk_xformBkgd[SDESIGN_SETTINGS]->setChecked(bi->bTransformBkgd);
        chk_adjustBkgd[SDESIGN_SETTINGS]->setChecked(bi->bAdjustPerspective);

        bkgdImageBtn->setText(bi->bkgdName);
    }
    else
    {
        bkgdImage[SDESIGN_SETTINGS]->setText("none");
    }

#ifdef BORDER
    // border
    BorderPtr bp = cSettings.getBorder();
    if (bp)
    {
        // common
        int index = borderType.findData(bp->getType());
        borderType.blockSignals(true);
        borderType.setCurrentIndex(index);
        borderType.blockSignals(false);

        qreal w = bp->getWidth();
        borderWidth->setText(QString::number(w));
        borderWidth->show();

        borderColorLabel[0]->show();

        qc = bp->getColor();
        borderColor[0]->setText(qc.name(QColor::HexArgb));
        borderColor[0]->show();

        variant = qc;
        colcode  = variant.toString();
        borderColorPatch[0]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
        borderColorPatch[0]->show();

        if (bp->getType() == BORDER_TWO_COLOR)
        {
            borderColorLabel[1]->show();

            BorderTwoColor * bp2 = dynamic_cast<BorderTwoColor*>(bp.get());
            qc = bp2->getColor2();
            borderColor[1]->setText(qc.name(QColor::HexArgb));
            borderColor[1]->show();

            variant = qc;
            colcode  = variant.toString();
            borderColorPatch[1]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
            borderColorPatch[1]->show();
        }
        else
        {
            borderColorLabel[1]->hide();
            borderColor[1]->hide();
            borderColorPatch[1]->hide();
        }
    }
    else
    {
        int index = borderType.findData(BORDER_NONE);
        borderType.blockSignals(true);
        borderType.setCurrentIndex(index);
        borderType.blockSignals(false);

        borderColorLabel[0]->hide();
        borderColor[0]->hide();
        borderColorPatch[0]->hide();

        borderColorLabel[1]->hide();
        borderColor[1]->hide();
        borderColorPatch[1]->hide();
    }
#endif
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
}

void page_canvasSettings::displayBkgdImgSettings(CanvasSettings & cSettings, eSettingsGroup group)
{
    // display background
    BkgdImgPtr bi = cSettings.getBkgdImage();
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

        bkgdImageBtn->setText(bi->bkgdName);
    }
    else
    {
        bkgdImage[SDESIGN_SETTINGS]->setText("none");
    }

}

void page_canvasSettings::setBorderFromForm()
{
#ifdef BORDER
    bool ok;
    QColor color;

    BorderPtr bp = cSettings.getBorder();
    if (bp)
    {
        qreal width = borderWidth->text().toDouble(&ok);
        if (ok)
        {
            bp->setWidth(width);
        }

        color.setNamedColor(borderColor[0]->text());
        if (color.isValid())
        {
            bp->setColor(color);
        }

        BorderTwoColor * bp2 = dynamic_cast<BorderTwoColor*>(bp.get());
        if (bp2)
        {
            color.setNamedColor(borderColor[1]->text());
            if (color.isValid())
            {
                bp2->setColor2(color);
            }
        }
    }
#endif
}



void page_canvasSettings::pickBorderColor()
{
#ifdef BORDER
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
        borderColor[0]->setText(color.name(QColor::HexArgb));

        QVariant variant = color;
        QString colcode  = variant.toString();
        borderColorPatch[0]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    }
#endif
}

void page_canvasSettings::pickBorderColor2()
{
#ifdef BORDER
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
        borderColor[1]->setText(color.name(QColor::HexArgb));

        QVariant variant = color;
        QString colcode  = variant.toString();
        borderColorPatch[1]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    }
#endif
}

void page_canvasSettings::borderChanged(int row)
{
    Q_UNUSED(row)
#ifdef BORDER

    eBorderType type = static_cast<eBorderType>(borderType.currentData().toInt());

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

    cSettings.setBorder(bp);

    setInfo();
#endif
}

void page_canvasSettings::slot_loadBackground()
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
            eWsData wsdata  = (config->canvasSettings == CS_STYLE) ? WS_LOADED : WS_TILING;
            CanvasSettings & cSettings = workspace->getStyledDesign(wsdata).getCanvasSettings();
            cSettings.setBkgdImage(bi);
            bi->bkgdImageChanged(true,false,true);

            display();
        }
    }
}

void page_canvasSettings::pickBackgroundColor()
{
    eWsData wsdata  = (config->canvasSettings == CS_STYLE) ? WS_LOADED : WS_TILING;
    CanvasSettings & cSettings = workspace->getStyledDesign(wsdata).getCanvasSettings();
    QColor color = cSettings.getBackgroundColor();

    AQColorDialog dlg(color,this);
    dlg.setCurrentColor(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        bkColorEdit[SDESIGN_SETTINGS]->setText(color.name(QColor::HexArgb));

        QVariant variant = color;
        QString colcode  = variant.toString();
        bkgdColorPatch[SDESIGN_SETTINGS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    }
}

void page_canvasSettings::slot_setBkgd()
{
    eWsData wsdata  = (config->canvasSettings == CS_STYLE) ? WS_LOADED : WS_TILING;
    CanvasSettings & cSettings = workspace->getStyledDesign(wsdata).getCanvasSettings();
    BkgdImgPtr bi = cSettings.getBkgdImage();
    if (bi)
    {
        Xform xf = bi->getXform();
        xf.setTransform(bkgdLayout[SDESIGN_SETTINGS]->getQTransform());
        bi->setXform(xf);
        bi->bkgdImageChanged(chk_showBkgd[SDESIGN_SETTINGS]->isChecked(),
                         chk_adjustBkgd[SDESIGN_SETTINGS]->isChecked(),
                         chk_xformBkgd[SDESIGN_SETTINGS]->isChecked());
        emit sig_viewWS();
    }
}

void page_canvasSettings::slot_setBkgdXform()
{
    eWsData wsdata  = (config->canvasSettings == CS_STYLE) ? WS_LOADED : WS_TILING;
    CanvasSettings & cSettings = workspace->getStyledDesign(wsdata).getCanvasSettings();
    BkgdImgPtr bi = cSettings.getBkgdImage();
    if (bi)
    {
        Xform xf = bi->getXform();
        xf.setTransform(bkgdLayout[SDESIGN_SETTINGS]->getQTransform());
        bi->setXform(xf);
        bi->bkgdTransformChanged(chk_xformBkgd[SDESIGN_SETTINGS]->isChecked());
    }
}


void page_canvasSettings::slot_setBkgdXformCanvas()
{
    CanvasSettings & cSettings = viewer->GetCanvasSettings();
    BkgdImgPtr bi = cSettings.getBkgdImage();
    if (bi)
    {
        Xform xf = bi->getXform();
        xf.setTransform(bkgdLayout[CANVAS_SETTINGS]->getQTransform());
        bi->setXform(xf);
        bi->bkgdTransformChanged(chk_xformBkgd[CANVAS_SETTINGS]->isChecked());
    }
}

void page_canvasSettings::slot_setBkgdCanvas()
{
    CanvasSettings & cSettings = viewer->GetCanvasSettings();
    BkgdImgPtr bi = cSettings.getBkgdImage();
    if (bi)
    {
        Xform xf = bi->getXform();
        xf.setTransform(bkgdLayout[CANVAS_SETTINGS]->getQTransform());
        bi->setXform(xf);
        bi->bkgdImageChanged(chk_showBkgd[CANVAS_SETTINGS]->isChecked(),
                         chk_adjustBkgd[CANVAS_SETTINGS]->isChecked(),
                         chk_xformBkgd[CANVAS_SETTINGS]->isChecked());
        emit sig_viewWS();
    }
}

void page_canvasSettings::pickBackgroundColorCanvas()
{
    CanvasSettings & cSettings = viewer->GetCanvasSettings();
    QColor color = cSettings.getBackgroundColor();

    AQColorDialog dlg(color,this);
    dlg.setCurrentColor(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        bkColorEdit[CANVAS_SETTINGS]->setText(color.name(QColor::HexArgb));

        QVariant variant = color;
        QString colcode  = variant.toString();
        bkgdColorPatch[CANVAS_SETTINGS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    }
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

void page_canvasSettings::slot_moveX(int amount)
{
    if (panel->getCurrentPage() == dynamic_cast<panel_page*>(this) && config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        qDebug() << "page_canvasSettings::slot_moveX - background" << amount;
        bkgdLayout[SDESIGN_SETTINGS]->bumpX(amount);
    }
}

void page_canvasSettings::slot_moveY(int amount)
{
    if (panel->getCurrentPage() == dynamic_cast<panel_page*>(this) && config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        qDebug() << "page_canvasSettings::slot_moveY" << amount;
        bkgdLayout[SDESIGN_SETTINGS]->bumpY(amount);
    }
}

void page_canvasSettings::slot_rotate(int amount)
{
    if (panel->getCurrentPage() == dynamic_cast<panel_page*>(this) && config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        qDebug() << "page_canvasSettings::slot_rotate" << amount;
        bkgdLayout[SDESIGN_SETTINGS]->bumpRot(amount);
    }
}

void page_canvasSettings::slot_scale(int amount)
{
    if (panel->getCurrentPage() == dynamic_cast<panel_page*>(this) && config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        qDebug() << "page_canvasSettings::slot_scale" << amount;
        bkgdLayout[SDESIGN_SETTINGS]->bumpScale(-amount);
    }
}

void page_canvasSettings::designSizeChanged(qreal)
{
    eWsData wsdata  = (config->canvasSettings == CS_STYLE) ? WS_LOADED : WS_TILING;
    CanvasSettings & cSettings = workspace->getStyledDesign(wsdata).getCanvasSettings();
    QSizeF sz = QSizeF(sizeEditW[SDESIGN_SETTINGS]->value(),sizeEditH[SDESIGN_SETTINGS]->value());
    cSettings.setCanvasSize(sz);
    emit sig_viewWS();

}

void page_canvasSettings::canvasSizeChanged(qreal)
{
      CanvasSettings & cSettings = viewer->GetCanvasSettings();
      QSizeF sz = QSizeF(sizeEditW[CANVAS_SETTINGS]->value(),sizeEditH[CANVAS_SETTINGS]->value());
      cSettings.setCanvasSize(sz);
      emit sig_viewWS();

}
