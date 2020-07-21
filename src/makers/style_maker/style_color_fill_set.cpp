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

#include "makers/style_maker/style_color_fill_set.h"
#include "base/utilities.h"

StyleColorFillSet::StyleColorFillSet(FaceGroup & fgroup, ColorSet & cset, QVBoxLayout * vbox)  : faceGroup(fgroup),colorSet(cset)
{
    connect(&mapper, SIGNAL(mapped(int)), this, SLOT(slot_colorVisibilityChanged(int)));

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
    table->horizontalHeader()->setVisible(false);
    table->verticalHeader()->setVisible(false);
    table->setRowCount(colorSet.size());
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

    table->horizontalHeader()->setVisible(true);
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
    colorSet.resetIndex();

    table->setRowCount(faceGroup.size());

    int row = 0;
    for (auto face : faceGroup)
    {
        QTableWidgetItem * item = new QTableWidgetItem(QString::number(row));
        table->setItem(row,COL_ROW,item);

        item = new QTableWidgetItem(QString::number(face->size()));
        table->setItem(row,COL_FACES,item);

        item = new QTableWidgetItem(QString::number(face->sides));
        table->setItem(row,COL_SIDES,item);

        item = new QTableWidgetItem(QString::number(face->area));
        table->setItem(row,COL_AREA,item);

        TPColor tpcolor = colorSet.getColor(row);
        QColor color    = tpcolor.color;

        item = new QTableWidgetItem(color.name());
        table->setItem(row,COL_COLOR_TEXT,item);

        QLabel * label = new QLabel;
        QVariant variant= color;
        QString colcode = variant.toString();
        label->setStyleSheet("QLabel { background-color :"+colcode+" ;}");
        table->setCellWidget(row,COL_COLOR_PATCH,label);

        QCheckBox * cb = new QCheckBox("Hide");
        cb->setStyleSheet("padding-left:11px;");
        table->setCellWidget(row,COL_HIDE,cb);
        cb->setChecked(tpcolor.hidden);
        mapper.setMapping(cb,row);
        QObject::connect(cb, SIGNAL(toggled(bool)), &mapper, SLOT(map()));

        QString astring;
        if (face->selected) astring = "X";
        item = new QTableWidgetItem(astring);
        table->setItem(row,COL_SEL,item);

        row++;
    }
    table->adjustTableSize();

    table->setCurrentIndex(selected);

}

void StyleColorFillSet::modify()
{
    qDebug().noquote() << "before" << colorSet.colorsString();

    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= colorSet.size())
        return;

    TPColor tpcolor = colorSet.getColor(currentRow);
    QColor color    = tpcolor.color;

    AQColorDialog dlg(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        colorSet.setColor(currentRow, color);
        qDebug().noquote() << "after" << colorSet.colorsString();

        display();
        emit sig_colorsChanged();
    }
}

void StyleColorFillSet::up()
{
    int currentRow = table->currentRow();
    if (currentRow < 1)
        return;

    TPColor a = colorSet.getColor(currentRow);
    TPColor b = colorSet.getColor(currentRow-1);

    colorSet.setColor(currentRow-1, a);
    colorSet.setColor(currentRow  , b);

    currentRow--;

    emit sig_colorsChanged();

    display();
}

void StyleColorFillSet::down()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= (colorSet.size()-1))
        return;

    TPColor a = colorSet.getColor(currentRow);
    TPColor b = colorSet.getColor(currentRow+1);

    colorSet.setColor(currentRow+1, a);
    colorSet.setColor(currentRow,   b);

    currentRow++;

    emit sig_colorsChanged();

    display();
}

void StyleColorFillSet::rptColor()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= colorSet.size())
        return;

    TPColor a = colorSet.getColor(currentRow);
    colorSet.resize(table->rowCount());
    for (int i = currentRow + 1; i < table->rowCount(); i++)
    {
        colorSet.setColor(i,a);
    }

    emit sig_colorsChanged();

    display();
}

void StyleColorFillSet::copyColor()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= colorSet.size())
        return;

    copyPasteColor = colorSet.getColor(currentRow);
}

void StyleColorFillSet::pasteColor()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= colorSet.size())
        return;

    colorSet.setColor(currentRow,copyPasteColor);

    emit sig_colorsChanged();

    display();
}

void StyleColorFillSet::slot_colorVisibilityChanged(int row)
{
    qDebug() << "slot_colorVisibilityChanged row=" << row;

    TPColor tpcolor = colorSet.getColor(row);
    QCheckBox * cb  = dynamic_cast<QCheckBox*>(table->cellWidget(row,COL_HIDE));
    bool hide       = cb->isChecked();
    tpcolor.hidden  = hide;
    colorSet.setColor(row, tpcolor);
    qDebug() << "hide state="  << hide;

    emit sig_colorsChanged();

    display();

    qDebug() << "slot_colorVisibilityChanged: done";
}

void StyleColorFillSet::slot_click(int row, int col)
{
    if (col == COL_SEL)
    {
        if (!faceGroup.isSelected(row))
        {
            faceGroup.select(row);
        }
        else
        {
            faceGroup.deselect(row);
        }
    }
    else
    {
        faceGroup.deselect();
    }

    emit sig_colorsChanged();

    display();
}

void StyleColorFillSet::slot_double_click(int row, int col)
{
    Q_UNUSED(row);
    Q_UNUSED(col);

    modify();
}
