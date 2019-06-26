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

using std::string;

page_canvasSettings::page_canvasSettings(ControlPanel *panel)  : panel_page(panel,"Canvas Settings")
{
    QRadioButton *radioStyle  = new QRadioButton("Style");
    QRadioButton *radioWS     = new QRadioButton("Workspace");
    QRadioButton *radioCanvas = new QRadioButton("Canvas");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addStretch();
    hbox ->addWidget(radioStyle);
    hbox->addStretch();
    hbox ->addWidget(radioWS);
    hbox->addStretch();
    hbox ->addWidget(radioCanvas);
    hbox->addStretch();

    vbox->addLayout(hbox);

    CanvasSettings  di = canvas->getCanvasSettings();

    QGridLayout * grid = new QGridLayout();
    QLabel * label;
    int row = 0;

    QSizeF  sz = di.getSizeF();
    label = new QLabel("size");
    sizeEditW = new QLineEdit();
    sizeEditW->setText(QString::number(sz.width()));
    sizeEditH = new QLineEdit();
    sizeEditH->setText(QString::number(sz.height()));
    grid->addWidget(label,row,0);
    grid->addWidget(sizeEditW,row,1);
    grid->addWidget(sizeEditH,row,2);
    row++;

    QPointF  pt = di.getStartTile();
    label = new QLabel("startTile");
    startEditX = new QLineEdit();
    startEditX->setText(QString::number(pt.x()));
    startEditY = new QLineEdit();
    startEditY->setText( QString::number(pt.y()));
    grid->addWidget(label,row,0);
    grid->addWidget(startEditX,row,1);
    grid->addWidget(startEditY,row,2);
    row++;

    qreal d  = di.getDiameter();
    label = new QLabel("diameter");
    diamEdit = new QLineEdit();
    diamEdit->setText(QString::number(d));
    grid->addWidget(label,row,0);
    grid->addWidget(diamEdit,row,1);
    row++;

    qreal sc = di.getScale();
    label = new QLabel("scale");
    scaleEdit = new QLineEdit();
    scaleEdit->setText(QString::number(sc));
    grid->addWidget(label,row,0);
    grid->addWidget(scaleEdit,row,1);
    row++;

    QColor qc = di.getBackgroundColor();
    label = new QLabel("bkgdColor");
    bkColorEdit = new QLineEdit();
    bkColorEdit->setText(qc.name(QColor::HexArgb));
    clabel   = new ClickableLabel;
    label->setMinimumWidth(50);
    QVariant variant = qc;
    QString colcode  = variant.toString();
    clabel->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black; }");
    grid->addWidget(label,row,0);
    grid->addWidget(bkColorEdit,row,1);
    grid->addWidget(clabel,row,2);
    row++;

    label = new QLabel("border");
    borderEdit = new QLineEdit();
    borderEdit->setReadOnly(true);
    borderEdit->setText(addr(di.getBorder().get()));
    grid->addWidget(label,row,0);
    grid->addWidget(borderEdit,row,1);
    row++;

    hbox = new QHBoxLayout();
    hbox->addLayout(grid);
    hbox->addStretch(1);

    AQWidget * widget = new AQWidget();
    widget->setLayout(hbox);

    QPushButton * setInfoBtn = new QPushButton("Set Source");
    setInfoBtn->setMaximumWidth(131);

    QPushButton * refreshBtn = new QPushButton("Refresh");
    refreshBtn->setMaximumWidth(131);

    QPushButton * pushBtn = new QPushButton("Push Canvas");
    pushBtn->setMaximumWidth(131);

    hbox = new QHBoxLayout();
    hbox->addStretch();
    hbox ->addWidget(setInfoBtn);
    hbox->addStretch();
    hbox ->addWidget(refreshBtn);
    hbox->addStretch();
    hbox ->addWidget(pushBtn);
    hbox->addStretch();

    line1 = new QLabel;
    line2 = new QLabel;

    vbox->addWidget(widget);
    vbox->addWidget(line1);
    vbox->addWidget(line2);
    vbox->addLayout(hbox);
    vbox->addStretch(1);
    adjustSize();

    bgroup.addButton(radioStyle, DIS_STYLE);
    bgroup.addButton(radioWS,    DIS_WS);
    bgroup.addButton(radioCanvas,DIS_CANVAS);
    bgroup.button(DIS_CANVAS)->setChecked(true);

    connect(setInfoBtn, &QPushButton::clicked,    this, &page_canvasSettings::setInfo);
    connect(refreshBtn, &QPushButton::clicked,    this, &page_canvasSettings::onEnter);
    connect(pushBtn,    &QPushButton::clicked,    this, &page_canvasSettings::push);
    connect(clabel,     &ClickableLabel::clicked, this, &page_canvasSettings::pickColor);
    connect(&bgroup,    SIGNAL(buttonClicked(int)), this, SLOT(slot_callNewlyEnter(int)));
}

void  page_canvasSettings::refreshPage()
{
    View * view = View::getInstance();
    QRect vr = view->contentsRect();

    QRectF qr = canvas->sceneRect();

    QString str1 = QString("View   = %1 %2 %3 %4").arg(vr.x()).arg(vr.y()).arg(vr.width()).arg(vr.height());
    QString str2 = QString("Canvas = %1 %2 %3 %4").arg(qr.x()).arg(qr.y()).arg(qr.width()).arg(qr.height());
    line1->setText(str1);
    line2->setText(str2);
}

void page_canvasSettings::slot_callNewlyEnter(int idx)
{
    Q_UNUSED(idx);
    onEnter();
}

void page_canvasSettings::onEnter()
{
    switch (bgroup.checkedId())
    {
    case DIS_STYLE:
        cSettings = workspace->getLoadedStyles().getCanvasSettings();
        break;
    case DIS_WS:
        cSettings = workspace->getWsStyles().getCanvasSettings();
        break;
    case DIS_CANVAS:
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
    clabel->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    borderEdit->setText(addr(cSettings.getBorder().get()));
}

void page_canvasSettings::push()
{
    canvas->writeCanvasSettings(cSettings);
    canvas->invalidate();
}

void page_canvasSettings::setInfo(bool)
{
    setFromForm();
    switch (bgroup.checkedId())
    {
    case DIS_STYLE:
        workspace->getLoadedStyles().setCanvasSettings(cSettings);
        break;
    case DIS_WS:
        workspace->getWsStyles().setCanvasSettings(cSettings);
        break;
    case DIS_CANVAS:
        canvas->writeCanvasSettings(cSettings);
        break;
    }
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
    if (ok)
    {
        cSettings.setBackgroundColor(color);
    }

    sig_viewWS();
}

void page_canvasSettings::pickColor()
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
        clabel->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    }
}
