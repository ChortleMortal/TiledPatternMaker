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

#include "makers/decoration_maker/style_color_fill_set.h"
#include "base/utilities.h"
#include "viewers/view.h"
#include "style/filled.h"

StyleColorFillSet::StyleColorFillSet(FilledPtr style, QVBoxLayout * vbox)  : filled(style)
{
    QGridLayout * grid = new QGridLayout;

    QPushButton * modBtn = new QPushButton("Modify");
    QPushButton * upBtn  = new QPushButton("Up");
    QPushButton * dwnBtn = new QPushButton("Down");
    QPushButton * rptBtn = new QPushButton("Repeat Color");
    QPushButton * cpyBtn = new QPushButton("Copy");
    QPushButton * pstBtn = new QPushButton("Paste");

    grid->addWidget(modBtn,0,1);
    grid->addWidget(cpyBtn,0,2);
    grid->addWidget(upBtn, 0,0);
    grid->addWidget(dwnBtn,1,0);
    grid->addWidget(rptBtn,1,1);
    grid->addWidget(pstBtn,1,2);
    vbox->addLayout(grid);

    connect(modBtn, &QPushButton::clicked, this, &StyleColorFillSet::modify);
    connect(upBtn,  &QPushButton::clicked, this, &StyleColorFillSet::up);
    connect(dwnBtn, &QPushButton::clicked, this, &StyleColorFillSet::down);
    connect(rptBtn, &QPushButton::clicked, this, &StyleColorFillSet::rptColor);
    connect(cpyBtn, &QPushButton::clicked, this, &StyleColorFillSet::copyColor);
    connect(pstBtn, &QPushButton::clicked, this, &StyleColorFillSet::pasteColor);

    table = new AQTableWidget();
    table->verticalHeader()->setVisible(false);
    table->setRowCount(filled->getWhiteColorSet()->size());
    table->setColumnCount(8);
    table->setColumnWidth(COL_ROW,40);
    table->setColumnWidth(COL_FACES,40);
    table->setColumnWidth(COL_SIDES,40);
    table->setColumnWidth(COL_AREA,70);
    table->setColumnWidth(COL_HIDE,80);
    table->setColumnWidth(COL_SEL,40);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setMinimumHeight(501);

    QStringList qslH;
    qslH << "Row" <<  "Faces" << "Sides" << "Area" << "Hide" << "Select" << "Color" << "Color";
    table->setHorizontalHeaderLabels(qslH);

    connect(table,  &QTableWidget::cellClicked,       this, &StyleColorFillSet::slot_click);
    connect(table,  &QTableWidget::cellDoubleClicked, this, &StyleColorFillSet::slot_double_click);

    vbox->addWidget(table);
}

void StyleColorFillSet::display()
{
    QModelIndex selected = table->currentIndex();

    table->clearContents();
    filled->getWhiteColorSet()->resetIndex();

    FaceGroup * faceGroup = filled->getFaceGroup();

    table->setRowCount(faceGroup->size());

    int row = 0;
    for (FaceSetPtr & faceSet : *faceGroup)
    {
        QTableWidgetItem * item = new QTableWidgetItem(QString::number(row));
        table->setItem(row,COL_ROW,item);

        item = new QTableWidgetItem(QString::number(faceSet->size()));
        table->setItem(row,COL_FACES,item);

        item = new QTableWidgetItem(QString::number(faceSet->sides));
        table->setItem(row,COL_SIDES,item);

        item = new QTableWidgetItem(QString::number(faceSet->area));
        table->setItem(row,COL_AREA,item);

        // take color from whiteColorSet
        TPColor tpcolor  = filled->getWhiteColorSet()->getColor(row);
        QColor color     = tpcolor.color;
        QColor fullColor = color;
        fullColor.setAlpha(255);

        QLineEdit * le = new QLineEdit();
        table->setCellWidget(row,COL_COLOR_TEXT,le);
        le->setText(color.name());
        connect(le, &QLineEdit::textEdited, [this,row] { colorChanged(row); });

        QLabel * label   = new QLabel;
        QVariant variant = fullColor;
        QString colcode  = variant.toString();
        label->setStyleSheet("QLabel { background-color :"+colcode+" ;}");
        table->setCellWidget(row,COL_COLOR_PATCH,label);

        QCheckBox * cb = new QCheckBox("Hide");
        cb->setStyleSheet("padding-left:11px;");
        table->setCellWidget(row,COL_HIDE,cb);
        cb->setChecked(tpcolor.hidden);
        connect(cb, &QCheckBox::clicked, [this, row] { colorVisibilityChanged(row); });

        QString astring;
        if (faceSet->selected) astring = "X";
        item = new QTableWidgetItem(astring);
        table->setItem(row,COL_SEL,item);

        row++;
    }
    table->adjustTableSize();

    table->setCurrentIndex(selected);
}

void StyleColorFillSet::modify()
{
    qDebug().noquote() << "before" << filled->getWhiteColorSet()->colorsString();

    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= filled->getWhiteColorSet()->size())
        return;

    TPColor tpcolor = filled->getWhiteColorSet()->getColor(currentRow);
    QColor color    = tpcolor.color;

    AQColorDialog dlg(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        filled->getWhiteColorSet()->setColor(currentRow, color);
        qDebug().noquote() << "after" << filled->getWhiteColorSet()->colorsString();

        display();
        emit sig_colorsChanged();
    }
}

void StyleColorFillSet::up()
{
    int currentRow = table->currentRow();
    if (currentRow < 1)
        return;

    TPColor a = filled->getWhiteColorSet()->getColor(currentRow);
    TPColor b = filled->getWhiteColorSet()->getColor(currentRow-1);

    filled->getWhiteColorSet()->setColor(currentRow-1, a);
    filled->getWhiteColorSet()->setColor(currentRow  , b);

    currentRow--;

    emit sig_colorsChanged();

    display();
}

void StyleColorFillSet::down()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= (filled->getWhiteColorSet()->size()-1))
        return;

    TPColor a = filled->getWhiteColorSet()->getColor(currentRow);
    TPColor b = filled->getWhiteColorSet()->getColor(currentRow+1);

    filled->getWhiteColorSet()->setColor(currentRow+1, a);
    filled->getWhiteColorSet()->setColor(currentRow,   b);

    currentRow++;

    emit sig_colorsChanged();

    display();
}

void StyleColorFillSet::rptColor()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= filled->getWhiteColorSet()->size())
        return;

    TPColor a = filled->getWhiteColorSet()->getColor(currentRow);
    filled->getWhiteColorSet()->resize(table->rowCount());
    for (int i = currentRow + 1; i < table->rowCount(); i++)
    {
        filled->getWhiteColorSet()->setColor(i,a);
    }

    emit sig_colorsChanged();

    display();
}

void StyleColorFillSet::copyColor()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= filled->getWhiteColorSet()->size())
        return;

    copyPasteColor = filled->getWhiteColorSet()->getColor(currentRow);
}

void StyleColorFillSet::pasteColor()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= filled->getWhiteColorSet()->size())
        return;

    filled->getWhiteColorSet()->setColor(currentRow,copyPasteColor);

    emit sig_colorsChanged();

    display();
}

void StyleColorFillSet::colorVisibilityChanged(int row)
{
    qDebug() << "colorVisibilityChanged row=" << row;

    TPColor tpcolor = filled->getWhiteColorSet()->getColor(row);
    QCheckBox * cb  = dynamic_cast<QCheckBox*>(table->cellWidget(row,COL_HIDE));
    bool hide       = cb->isChecked();
    tpcolor.hidden  = hide;
    filled->getWhiteColorSet()->setColor(row, tpcolor);
    qDebug() << "hide state="  << hide;

    View * view = View::getInstance();
    view->update();

    qDebug() << "colorVisibilityChanged: done";
}

void StyleColorFillSet::colorChanged(int row)
{
    qDebug() << "colorChanged row=" << row;

    QLineEdit *  le = dynamic_cast<QLineEdit*>(table->cellWidget(row,COL_COLOR_TEXT));

    QString  sColor = le->text();
    if (sColor.size() != 7)
    {
        return;     // wait for complete color
    }

    QColor color(sColor);
    if (color.isValid())
    {
        filled->getWhiteColorSet()->setColor(row, color);
        display();
        emit sig_colorsChanged();
    }
}

void StyleColorFillSet::slot_click(int row, int col)
{
    if (col == COL_SEL)
    {
        if (!filled->getFaceGroup()->isSelected(row))
        {
            filled->getFaceGroup()->select(row);
        }
        else
        {
            filled->getFaceGroup()->deselect(row);
        }
    }
    else
    {
        filled->getFaceGroup()->deselect();
    }

    View * view = View::getInstance();
    view->update();

    display();
}

void StyleColorFillSet::slot_double_click(int row, int col)
{
    Q_UNUSED(row);
    Q_UNUSED(col);

    modify();
}
