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
#include "makers/tilingmaker.h"
#include "panels/panel.h"
#include "viewers/workspaceviewer.h"

using std::string;

page_canvasSettings::page_canvasSettings(ControlPanel * apanel)  : panel_page(apanel,"Canvas Settings"), bkgdLayout("Bkgd Xform")
{
    QLabel * label            = new QLabel("Source");
    QRadioButton *radioStyle  = new QRadioButton("Style");
    QRadioButton *radioWS     = new QRadioButton("Workspace");
    QRadioButton *radioCanvas = new QRadioButton("Canvas");

    QPushButton * pbRefresh = new QPushButton("Refresh");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(label);
    hbox->addStretch(1);
    hbox ->addWidget(radioStyle);
    hbox->addStretch(1);
    hbox ->addWidget(radioWS);
    hbox->addStretch(1);
    hbox ->addWidget(radioCanvas);
    hbox->addStretch(2);
    hbox->addWidget(pbRefresh);
    hbox->addStretch(20);

    vbox->addLayout(hbox);

    QGridLayout * grid = new QGridLayout();
    int row = 0;

    // size
    label = new QLabel("width");
    grid->addWidget(label,row,0);
    sizeEditW = new QLineEdit();
    grid->addWidget(sizeEditW,row,1);
    row++;

    label = new QLabel("height");
    grid->addWidget(label,row,0);
    sizeEditH = new QLineEdit();
    grid->addWidget(sizeEditH,row,1);
    row++;

    label = new QLabel("startTile");
    grid->addWidget(label,row,0);
    startEditX = new QLineEdit();
    grid->addWidget(startEditX,row,1);
    startEditY = new QLineEdit();
    grid->addWidget(startEditY,row,2);
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

    //
    // backgrounds
    //

    QPushButton * adjustBtn       = new QPushButton("Adjust perspective");
    QPushButton * saveAdjustedBtn = new QPushButton("Save Adjusted");

    chk_showBkgd    = new QCheckBox("Show Background");
    chk_adjustBkgd  = new QCheckBox("Perspective");
    chk_xformBkgd   = new QCheckBox("Xform");

    // background color
    label = new QLabel("Background Color");
    bkColorEdit = new QLineEdit();
    bkgdColorPatch   = new ClickableLabel;
    bkgdColorPatch->setMinimumWidth(50);

    grid->addWidget(label,row,0);
    grid->addWidget(bkColorEdit,row,1);
    grid->addWidget(bkgdColorPatch,row,2);
    row++;

    // background image
    label = new QLabel("Background Image");
    QPushButton * imageBtn = new QPushButton("Select");
    imageEdit = new QLineEdit();

    grid->addWidget(label,row,0);
    grid->addWidget(imageBtn,row,1);
    grid->addWidget(imageEdit,row,2);
    row++;

    grid->addLayout(&bkgdLayout,row,0,1,4);
    grid->addWidget(adjustBtn,row,4);
    row++;

    grid->addWidget(chk_showBkgd,row,0);
    grid->addWidget(chk_adjustBkgd,row,1);
    grid->addWidget(chk_xformBkgd,row,2);
    grid->addWidget(saveAdjustedBtn,row,4);

    WorkspaceViewer * wviewer  = WorkspaceViewer::getInstance();

    connect(imageBtn,       &QPushButton::clicked,            this,    &page_canvasSettings::slot_loadBackground);
    connect(adjustBtn,      &QPushButton::clicked,            this,    &page_canvasSettings::slot_adjustBackground);
    connect(saveAdjustedBtn,&QPushButton::clicked,            this,    &page_canvasSettings::slot_saveAdjustedBackground);
    connect(pbRefresh,      &QPushButton::clicked,            this,    &page_canvasSettings::display);
    connect(wviewer,        &WorkspaceViewer::sig_viewUpdated,this,    &page_canvasSettings::display);

    connect(&bkgdLayout,    &LayoutTransform::xformChanged,   this,    &page_canvasSettings::slot_setBkgdXform);
    connect(chk_showBkgd,   &QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgd);
    connect(chk_adjustBkgd, &QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgd);
    connect(chk_xformBkgd,  &QAbstractButton::clicked,        this,    &page_canvasSettings::slot_setBkgd);

    hbox = new QHBoxLayout();
    hbox->addLayout(grid);
    hbox->addStretch(1);

    AQWidget * widget = new AQWidget();
    widget->setLayout(hbox);

    QPushButton * setInfoBtn = new QPushButton("Write Source");
    setInfoBtn->setMaximumWidth(131);

    hbox = new QHBoxLayout();
    hbox->addStretch();
    hbox ->addWidget(setInfoBtn);
    hbox->addStretch();

    line1 = new QLabel;
    line2 = new QLabel;

    vbox->addWidget(widget);
    vbox->addWidget(line1);
    vbox->addWidget(line2);
    vbox->addLayout(hbox);
    vbox->addStretch(1);
    adjustSize();

    bgroup.addButton(radioStyle, CS_STYLE);
    bgroup.addButton(radioWS,    CS_WS);
    bgroup.addButton(radioCanvas,CS_CANVAS);
    bgroup.button(config->canvasSettings)->setChecked(true);

    connect(setInfoBtn,      &QPushButton::clicked,            this, &page_canvasSettings::setInfo);
    connect(bkgdColorPatch,  &ClickableLabel::clicked,         this, &page_canvasSettings::pickBackgroundColor);
    connect(borderColorPatch[0],&ClickableLabel::clicked,      this, &page_canvasSettings::pickBorderColor);
    connect(borderColorPatch[1],&ClickableLabel::clicked,      this, &page_canvasSettings::pickBorderColor2);
    connect(&borderType,     SIGNAL(currentIndexChanged(int)), this, SLOT(borderChanged(int)));
    connect(&bgroup,         SIGNAL(buttonClicked(int)),       this, SLOT(settingsSelectionChanged(int)));

    connect(maker,  &TiledPatternMaker::sig_loadedTiling,      this, &page_canvasSettings::onEnter);
    connect(maker,  &TiledPatternMaker::sig_loadedXML,         this, &page_canvasSettings::onEnter);
    connect(maker,  &TiledPatternMaker::sig_loadedDesign,      this, &page_canvasSettings::onEnter);

    connect(canvas, &Canvas::sig_deltaScale,    this, &page_canvasSettings::slot_scale);
    connect(canvas, &Canvas::sig_deltaRotate,   this, &page_canvasSettings::slot_rotate);
    connect(canvas, &Canvas::sig_deltaMoveY,    this, &page_canvasSettings::slot_moveY);
    connect(canvas, &Canvas::sig_deltaMoveX,    this, &page_canvasSettings::slot_moveX);
}

void  page_canvasSettings::refreshPage()
{
    View * view = View::getInstance();
    QRect  vr = view->contentsRect();
    QString str1 = QString("<pre>View   = %1 %2 %3 %4</pre>").arg(vr.x()).arg(vr.y()).arg(vr.width()).arg(vr.height());
    line1->setText(str1);

    if (canvas->scene)
    {
#if 0
        QRectF qr = canvas->scene->sceneRect();
        QString str2 = QString("<pre>Canvas = %1 %2 %3 %4</pre>").arg(qr.x()).arg(qr.y()).arg(qr.width()).arg(qr.height());
#else
        CanvasSettings cs = canvas->getCanvasSettings();
        QString str2 = QString("<pre>Canvas = %1 %2</pre>").arg(cs.getBackgroundColor().name()).arg(Utils::addr(cs.getBkgdImage().get()));
        line2->setText(str2);
#endif
    }
}

void page_canvasSettings::settingsSelectionChanged(int idx)
{
    Q_UNUSED(idx)
    config->canvasSettings = static_cast<eCSSelect>(idx);
    onEnter();
}

void page_canvasSettings::onEnter()
{
    switch (config->canvasSettings)
    {
    case CS_STYLE:
        cSettings = workspace->getLoadedStyles().getCanvasSettings();
        break;
    case CS_WS:
        cSettings = workspace->getWsStyles().getCanvasSettings();
        break;
    case CS_CANVAS:
        cSettings = canvas->getCanvasSettings();
        break;
    }

    display();
}

void page_canvasSettings::display()
{
    if (panel->getCurrentPage() != dynamic_cast<panel_page*>(this))
    {
        return;
    }

    QSizeF  sz = cSettings.getSizeF();
    sizeEditW->setText(QString::number(sz.width()));
    sizeEditH->setText(QString::number(sz.height()));

    QPointF  pt = cSettings.getStartTile();
    startEditX->setText(QString::number(pt.x()));
    startEditY->setText( QString::number(pt.y()));

    QColor qc = cSettings.getBackgroundColor();
    bkColorEdit->setText(qc.name(QColor::HexArgb));

    QVariant variant = qc;
    QString colcode  = variant.toString();
    bkgdColorPatch->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

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

    // display background
    displayBackgroundStatus();
}

void page_canvasSettings::setInfo()
{
    qDebug() << "page_canvasSettings::setInfo()";

    setFromForm();

    switch (bgroup.checkedId())
    {
    case CS_STYLE:
        workspace->getLoadedStyles().setCanvasSettings(cSettings);
        break;
    case CS_WS:
        workspace->getWsStyles().setCanvasSettings(cSettings);
        break;
    case CS_CANVAS:
        canvas->useCanvasSettings(cSettings);
        break;
    }

    emit sig_viewWS();
    onEnter();
}

void page_canvasSettings::setFromForm()
{
    bool ok,ok2;
    qreal W = sizeEditW->text().toDouble(&ok);
    qreal H = sizeEditH->text().toDouble(&ok2);
    if (ok && ok2)
    {
        QSizeF asize(W,H);
        cSettings.setSizeF(asize);
    }

    qreal X = startEditX->text().toDouble(&ok);
    qreal Y = startEditY->text().toDouble(&ok2);
    if (ok && ok2)
    {
        QPointF apoint(X,Y);
        cSettings.setStartTile(apoint);
    }

    QColor color;
    color.setNamedColor(bkColorEdit->text());
    if (color.isValid())
    {
        cSettings.setBackgroundColor(color);
    }

    setBorderFromForm();

    sig_viewWS();
}

void page_canvasSettings::setBorderFromForm()
{
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
}

void page_canvasSettings::pickBackgroundColor()
{
    QColor color = cSettings.getBackgroundColor();

    AQColorDialog dlg(color,this);
    dlg.setCurrentColor(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        bkColorEdit->setText(color.name(QColor::HexArgb));

        QVariant variant = color;
        QString colcode  = variant.toString();
        bkgdColorPatch->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    }
}

void page_canvasSettings::pickBorderColor()
{
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
}

void page_canvasSettings::pickBorderColor2()
{
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
}

void page_canvasSettings::borderChanged(int row)
{
    Q_UNUSED(row)

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
}

void page_canvasSettings::displayBackgroundStatus()
{
    BkgdImgPtr bi = cSettings.getBkgdImage();
    if (bi)
    {
        Xform xf = bi->getXform();
        bkgdLayout.setX(xf.getTranslateX());
        bkgdLayout.setY(xf.getTranslateY());
        bkgdLayout.setScale(xf.getScale());
        bkgdLayout.setRot(xf.getRotateDegrees());

        chk_showBkgd->setChecked(bi->bShowBkgd);
        chk_xformBkgd->setChecked(bi->bTransformBkgd);
        chk_adjustBkgd->setChecked(bi->bAdjustPerspective);

        imageEdit->setText(bi->bkgdName);
    }
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
            cSettings.setBkgdImage(bi);
            bi->bkgdImageChanged(true,false,true);

            displayBackgroundStatus();
        }
    }
}

void page_canvasSettings::slot_setBkgd()
{
    BkgdImgPtr bi = cSettings.getBkgdImage();
    if (bi)
    {
        Xform xf = bi->getXform();
        xf.setTransform(bkgdLayout.getQTransform());
        bi->setXform(xf);
        bi->bkgdImageChanged(chk_showBkgd->isChecked(),
                         chk_adjustBkgd->isChecked(),
                         chk_xformBkgd->isChecked());
        emit sig_viewWS();
    }
}

void page_canvasSettings::slot_setBkgdXform()
{
    BkgdImgPtr bi = cSettings.getBkgdImage();
    if (bi)
    {
        Xform xf = bi->getXform();
        xf.setTransform(bkgdLayout.getQTransform());
        bi->setXform(xf);
        bi->bkgdTransformChanged(chk_xformBkgd->isChecked());
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
        bkgdLayout.bumpX(amount);
    }
}

void page_canvasSettings::slot_moveY(int amount)
{
    if (panel->getCurrentPage() == dynamic_cast<panel_page*>(this) && config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        qDebug() << "page_canvasSettings::slot_moveY" << amount;
        bkgdLayout.bumpY(amount);
    }
}

void page_canvasSettings::slot_rotate(int amount)
{
    if (panel->getCurrentPage() == dynamic_cast<panel_page*>(this) && config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        qDebug() << "page_canvasSettings::slot_rotate" << amount;
        bkgdLayout.bumpRot(amount);
    }
}

void page_canvasSettings::slot_scale(int amount)
{
    if (panel->getCurrentPage() == dynamic_cast<panel_page*>(this) && config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        qDebug() << "page_canvasSettings::slot_scale" << amount;
        bkgdLayout.bumpScale(-amount);
    }
}
