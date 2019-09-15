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

using std::string;

page_canvasSettings::page_canvasSettings(ControlPanel *panel)  : panel_page(panel,"Canvas Settings")
{
    QLabel * label            = new QLabel("Source");
    QRadioButton *radioStyle  = new QRadioButton("Style");
    QRadioButton *radioWS     = new QRadioButton("Workspace");
    QRadioButton *radioCanvas = new QRadioButton("Canvas");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addStretch();
    hbox->addWidget(label);
    hbox->addStretch();
    hbox ->addWidget(radioStyle);
    hbox->addStretch();
    hbox ->addWidget(radioWS);
    hbox->addStretch();
    hbox ->addWidget(radioCanvas);
    hbox->addStretch();

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

    label = new QLabel("diameter");
    grid->addWidget(label,row,0);
    diamEdit = new QLineEdit();
    grid->addWidget(diamEdit,row,1);
    row++;

    label = new QLabel("scale");
    grid->addWidget(label,row,0);
    scaleEdit = new QLineEdit();
    grid->addWidget(scaleEdit,row,1);
    row++;

    // background color
    label = new QLabel("bkgdColor");
    grid->addWidget(label,row,0);
    bkColorEdit = new QLineEdit();
    grid->addWidget(bkColorEdit,row,1);
    bkgdColorPatch   = new ClickableLabel;
    bkgdColorPatch->setMinimumWidth(50);
    grid->addWidget(bkgdColorPatch,row,2);
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
}

void  page_canvasSettings::refreshPage()
{
    View * view = View::getInstance();
    QRect  vr = view->contentsRect();
    QString str1 = QString("<pre>View   = %1 %2 %3 %4</pre>").arg(vr.x()).arg(vr.y()).arg(vr.width()).arg(vr.height());
    line1->setText(str1);

    if (canvas->scene)
    {
        QRectF qr = canvas->scene->sceneRect();
        QString str2 = QString("<pre>Canvas = %1 %2 %3 %4</pre>").arg(qr.x()).arg(qr.y()).arg(qr.width()).arg(qr.height());
        line2->setText(str2);
    }
}

void page_canvasSettings::settingsSelectionChanged(int idx)
{
    Q_UNUSED(idx);
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
    QSizeF  sz = cSettings.getSizeF();
    sizeEditW->setText(QString::number(sz.width()));
    sizeEditH->setText(QString::number(sz.height()));

    QPointF  pt = cSettings.getStartTile();
    startEditX->setText(QString::number(pt.x()));
    startEditY->setText( QString::number(pt.y()));

    qreal d  = cSettings.getDiameter();
    diamEdit->setText(QString::number(d));

    qreal sc = cSettings.getScale();
    scaleEdit->setText(QString::number(sc));

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
}

void page_canvasSettings::setInfo()
{
    setFromForm();

    switch (bgroup.checkedId())
    {
    case CS_STYLE:
        workspace->getLoadedStyles().setupCanvas(cSettings);
        break;
    case CS_WS:
        workspace->getWsStyles().setupCanvas(cSettings);
        break;
    case CS_CANVAS:
        canvas->setCanvasSettings(cSettings);
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

    qreal d = diamEdit->text().toDouble(&ok);
    if (ok)
    {
        cSettings.setDiameter(d);
    }

    qreal s = scaleEdit->text().toDouble(&ok);
    if (ok)
    {
        cSettings.setScale(s);
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
    Q_UNUSED(row);

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
