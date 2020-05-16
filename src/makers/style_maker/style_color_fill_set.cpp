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

StyleColorFillSet::StyleColorFillSet(FilledEditor * editor, ColorSet & cset, QVBoxLayout * vbox)  : colorSet(cset)
{
    ed = editor;
    Q_ASSERT(ed);

    connect(this, &StyleColorFillSet::sig_colorsChanged, editor, &FilledEditor::slot_colorsChanged);

    connect(&mapper, SIGNAL(mapped(int)), this, SLOT(slot_colorVisibilityChanged(int)));

    QGridLayout * grid = new QGridLayout;

    QPushButton * addBtn = new QPushButton("Add");
    QPushButton * modBtn = new QPushButton("Modify");
    QPushButton * delBtn = new QPushButton("Delete");
    QPushButton * upBtn  = new QPushButton("Up");
    QPushButton * dwnBtn = new QPushButton("Down");
    QPushButton * rptBtn = new QPushButton("Repeat Color");
    QPushButton * cpyBtn = new QPushButton("Copy");
    QPushButton * pstBtn = new QPushButton("Paste");

    grid->addWidget(addBtn,0,0);
    grid->addWidget(modBtn,0,1);
    grid->addWidget(delBtn,0,2);
    grid->addWidget(cpyBtn,0,3);
    grid->addWidget(upBtn, 1,0);
    grid->addWidget(dwnBtn,1,1);
    grid->addWidget(rptBtn,1,2);
    grid->addWidget(pstBtn,1,3);
    vbox->addLayout(grid);

    table = new QTableWidget();
    table->horizontalHeader()->setVisible(false);
    table->verticalHeader()->setVisible(false);
    table->setRowCount(colorSet.size());
    table->setColumnCount(7);
    table->setColumnWidth(0,40);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setMinimumHeight(501);

    table->horizontalHeader()->setVisible(true);
    QStringList qslH;
    qslH << "" << "Color" << "Color" << "Hide/Select" << "Number" << "Sides" << "Area";
    table->setHorizontalHeaderLabels(qslH);

    vbox->addWidget(table);

    connect(addBtn, &QPushButton::clicked, this, &StyleColorFillSet::add);
    connect(modBtn, &QPushButton::clicked, this, &StyleColorFillSet::modify);
    connect(delBtn, &QPushButton::clicked, this, &StyleColorFillSet::del);
    connect(upBtn,  &QPushButton::clicked, this, &StyleColorFillSet::up);
    connect(dwnBtn, &QPushButton::clicked, this, &StyleColorFillSet::down);
    connect(rptBtn, &QPushButton::clicked, this, &StyleColorFillSet::rptColor);
    connect(cpyBtn, &QPushButton::clicked, this, &StyleColorFillSet::copyColor);
    connect(pstBtn, &QPushButton::clicked, this, &StyleColorFillSet::pasteColor);
    connect(table,  &QTableWidget::cellClicked,       this, &StyleColorFillSet::slot_click);
    connect(table,  &QTableWidget::cellDoubleClicked, this, &StyleColorFillSet::slot_double_click);

    createTable();
}

void StyleColorFillSet::createTable()
{
    Filled * filled = ed->getFilled();
    FaceGroup & faceGroup = filled->getFaceGroup();

    int rowCount = qMax(faceGroup.size(),colorSet.size());
    table->setRowCount(rowCount);
    int row = 0;

    for (auto it = faceGroup.begin(); it != faceGroup.end(); it++)
    {
        FaceSetPtr fsp = *it;

        TPColor tpcolor = fsp->tpcolor;

        qDebug() << "color="  << tpcolor.color << "sides=" << fsp->sides;

        QTableWidgetItem * item = new QTableWidgetItem(QString::number(row));
        table->setItem(row,0,item);

        item = new QTableWidgetItem(QString::number(fsp->size()));
        table->setItem(row,4,item);

        item = new QTableWidgetItem(QString::number(fsp->sides));
        table->setItem(row,5,item);

        item = new QTableWidgetItem(QString::number(fsp->area));
        table->setItem(row,6,item);

        row++;
    }
}

void StyleColorFillSet::displayColors(ColorSet & cset)
{
    Filled * filled = ed->getFilled();
    FaceGroup & faceGroup = filled->getFaceGroup();
    int rowCount = qMax(faceGroup.size(),colorSet.size());
    table->setRowCount(rowCount);

    cset.resetIndex();
    for (int i=0; i < cset.size(); i++)
    {
        int row = i;
        TPColor tpcolor = cset.getNextColor();
        QColor color    = tpcolor.color;

        QTableWidgetItem * item = new QTableWidgetItem(color.name());
        table->setItem(row,1,item);

        QLabel * label = new QLabel;
        QVariant variant= color;
        QString colcode = variant.toString();
        label->setStyleSheet("QLabel { background-color :"+colcode+" ;}");
        table->setCellWidget(row,2,label);

        QCheckBox * cb = new QCheckBox("Hide");
        cb->setStyleSheet("padding-left:11px;");
        table->setCellWidget(row,3,cb);
        cb->setChecked(tpcolor.hidden);
        mapper.setMapping(cb,row);
        QObject::connect(cb, SIGNAL(toggled(bool)), &mapper, SLOT(map()), Qt::UniqueConnection);
    }
}

void StyleColorFillSet::add()
{
    AQColorDialog dlg;
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    QColor newColor = dlg.selectedColor();
    if (newColor.isValid())
    {
        colorSet.addColor(newColor);
        if (ed)
        {
            emit sig_colorsChanged();
        }
        displayColors(colorSet);
    }
}

void StyleColorFillSet::modify()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= colorSet.size())
        return;

    TPColor tpcolor = colorSet.getColor(currentRow);
    QColor color    = tpcolor.color;

    AQColorDialog dlg(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        colorSet.setColor(currentRow, color);

        emit sig_colorsChanged();
        displayColors(colorSet);
    }
}

void StyleColorFillSet::del()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= colorSet.size())
        return;

    colorSet.removeColor(currentRow);

    emit sig_colorsChanged();

    displayColors(colorSet);
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

    displayColors(colorSet);
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

    displayColors(colorSet);
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

    displayColors(colorSet);
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

    displayColors(colorSet);
}

void StyleColorFillSet::slot_colorVisibilityChanged(int row)
{
    qDebug() << "slot_colorVisibilityChanged row=" << row;

    if (row >= colorSet.size())
    {
        qDebug() << "ignored - out of range of color set";
    }

    TPColor tpcolor = colorSet.getColor(row);
    QCheckBox * cb  = dynamic_cast<QCheckBox*>(table->cellWidget(row,3));
    bool hide       = cb->isChecked();
    tpcolor.hidden  = hide;
    qDebug() << "hide state="  << hide;

    colorSet.setColor(row, tpcolor);

    emit sig_colorsChanged();

    displayColors(colorSet);

    qDebug() << "slot_colorVisibilityChanged: done";
}

void StyleColorFillSet::slot_click(int row, int col)
{
    FaceGroup & fg = ed->getFilled()->getFaceGroup();
    if (col == 3)
    {
        if (!fg.isSelected(row))
        {
            fg.select(row);
        }
        else
        {
            fg.deselect(row);
        }
    }
    else
    {
        fg.deselect();
    }

    emit sig_colorsChanged();
}

void StyleColorFillSet::slot_double_click(int row, int col)
{
    Q_UNUSED(row);
    Q_UNUSED(col);

    modify();
}
