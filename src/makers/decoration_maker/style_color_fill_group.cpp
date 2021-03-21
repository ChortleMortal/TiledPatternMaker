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

#include "makers/decoration_maker/style_color_fill_group.h"
#include "panels/dlg_colorSet.h"
#include "base/utilities.h"

StyleColorFillGroup::StyleColorFillGroup(FaceGroup & fGroup, ColorGroup & cGroup, QVBoxLayout *vbox) : faceGroup(fGroup), colorGroup(cGroup)
{
    colorGroup = cGroup;
    faceGroup  = fGroup;

    QGridLayout * grid = new QGridLayout;

    QPushButton * modBtn = new QPushButton("Modify");
    QPushButton * upBtn  = new QPushButton("Up");
    QPushButton * dwnBtn = new QPushButton("Down");
    QPushButton * rptBtn = new QPushButton("Repeat Set");
    QPushButton * cpyBtn = new QPushButton("Copy Set");
    QPushButton * pstBtn = new QPushButton("Paste Set");

    grid->addWidget(modBtn,0,1);
    grid->addWidget(cpyBtn,0,3);
    grid->addWidget(upBtn, 1,0);
    grid->addWidget(dwnBtn,1,1);
    grid->addWidget(rptBtn,1,2);
    grid->addWidget(pstBtn,1,3);
    vbox->addLayout(grid);

    connect(modBtn, &QPushButton::clicked, this, &StyleColorFillGroup::modify);
    connect(upBtn,  &QPushButton::clicked, this, &StyleColorFillGroup::up);
    connect(dwnBtn, &QPushButton::clicked, this, &StyleColorFillGroup::down);
    connect(rptBtn, &QPushButton::clicked, this, &StyleColorFillGroup::rptSet);
    connect(cpyBtn, &QPushButton::clicked, this, &StyleColorFillGroup::copySet);
    connect(pstBtn, &QPushButton::clicked, this, &StyleColorFillGroup::pasteSet);

    table = new AQTableWidget();
    table->horizontalHeader()->setVisible(false);
    table->verticalHeader()->setVisible(false);
    table->setColumnCount(8);
    table->setColumnWidth(COL_INDEX,35);
    table->setColumnWidth(COL_COUNT,40);
    table->setColumnWidth(COL_SIDES,40);
    table->setColumnWidth(COL_AREA,70);
    table->setColumnWidth(COL_HIDE,80);
    table->setColumnWidth(COL_SEL,40);
    table->setColumnWidth(COL_BTN,70);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setRowCount(1);

    table->horizontalHeader()->setVisible(true);
    QStringList qslH;
    qslH << "" << "Count" << "Sides" << "Area" << "Hide" << "Sel" << "Edit" << "Colors";
    table->setHorizontalHeaderLabels(qslH);
    table->setMinimumHeight(501);

    connect(table,  &QTableWidget::cellClicked,       this, &StyleColorFillGroup::slot_click);
    connect(table,  &QTableWidget::cellDoubleClicked, this, &StyleColorFillGroup::slot_double_click);

    vbox->addWidget(table);
}

void StyleColorFillGroup::display()
{
    QModelIndex selected = table->currentIndex();

    table->clearContents();

    table->setRowCount(faceGroup.size());

    int row = 0;
    for (auto& face : faceGroup)
    {
        QTableWidgetItem * item = new QTableWidgetItem(QString::number(row));
        table->setItem(row,COL_INDEX,item);

        item = new QTableWidgetItem(QString::number(face->size()));
        table->setItem(row,COL_COUNT,item);

        item = new QTableWidgetItem(QString::number(face->sides));
        table->setItem(row,COL_SIDES,item);

        item = new QTableWidgetItem(QString::number(face->area));
        table->setItem(row,COL_AREA,item);

        QCheckBox * cb = new QCheckBox("Hide");
        cb->setStyleSheet("padding-left:11px;");
        table->setCellWidget(row,COL_HIDE,cb);
        cb->setChecked(colorGroup.isHidden(row));
        connect(cb, &QCheckBox::toggled, [this,row] { colorSetVisibilityChanged(row); });

        QPushButton * btn = new QPushButton("Edit");
        btn->setFixedWidth(40);
        table->setCellWidget(row,COL_BTN,btn);
        connect(btn, &QPushButton::clicked, [this,row] { edit(row); });

        ColorSet & cset = colorGroup.getColorSet(row);
        AQWidget * widget = cset.createWidget();
        table->setCellWidget(row,COL_COLORS,widget);

        QString astring;
        if (face->selected) astring = "X";
        item = new QTableWidgetItem(astring);
        table->setItem(row,COL_SEL,item);
        row++;
    }

    table->setColumnWidth(COL_BTN,45);
    table->resizeColumnToContents(5);
    table->resizeColumnToContents(6);
    table->adjustTableSize();

    table->setCurrentIndex(selected);

}

void StyleColorFillGroup::edit(int row)
{
    if (row < 0 || row >= colorGroup.size())
    {
        return;
    }

    ColorSet & colorSet = colorGroup.getColorSet(row);

    DlgColorSet dlg(colorSet);

    connect(&dlg, &DlgColorSet::sig_colorsChanged, this, &StyleColorFillGroup::sig_colorsChanged);

    dlg.exec();

    display();
}

void StyleColorFillGroup::modify()
{
    int row = table->currentRow();
    edit(row);
}

void StyleColorFillGroup::up()
{
    int currentRow = table->currentRow();
    if (currentRow < 1)
        return;

    ColorSet a = colorGroup.getColorSet(currentRow);
    ColorSet b = colorGroup.getColorSet(currentRow-1);

    colorGroup.setColorSet(currentRow-1, a);
    colorGroup.setColorSet(currentRow  , b);

    emit sig_colorsChanged();

    display();
}

void StyleColorFillGroup::down()
{
    int currentRow = table->currentRow();
    if (currentRow >= (colorGroup.size()-1))
        return;

    ColorSet a = colorGroup.getColorSet(currentRow);
    ColorSet b = colorGroup.getColorSet(currentRow+1);

    colorGroup.setColorSet(currentRow+1, a);
    colorGroup.setColorSet(currentRow  , b);


    emit sig_colorsChanged();

    display();
}


void StyleColorFillGroup::rptSet()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= colorGroup.size())
        return;

    ColorSet  set = colorGroup.getColorSet(currentRow);
    colorGroup.resize(table->rowCount());
    for (int i = currentRow + 1; i < table->rowCount(); i++)
    {
        colorGroup.setColorSet(i,set);
    }

    emit sig_colorsChanged();

    display();
}

void StyleColorFillGroup::copySet()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= colorGroup.size())
        return;

    copyPasteSet = colorGroup.getColorSet(currentRow);
}

void StyleColorFillGroup::pasteSet()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= colorGroup.size())
        return;

    colorGroup.setColorSet(currentRow,copyPasteSet);

    emit sig_colorsChanged();

    display();
}

void StyleColorFillGroup::colorSetVisibilityChanged(int row)
{
    qDebug() << "colorVisibilityChanged row=" << row;

    QCheckBox * cb  = dynamic_cast<QCheckBox*>(table->cellWidget(row,COL_HIDE));
    bool hide       = cb->isChecked();
    qDebug() << "hide state="  << hide;

    colorGroup.hide(row, hide);

    emit sig_colorsChanged();

    qDebug() << "colorVisibilityChanged: done";
}

void StyleColorFillGroup::slot_click(int row, int col)
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

void StyleColorFillGroup::slot_double_click(int row, int col)
{
    Q_UNUSED(row);
    Q_UNUSED(col);

    modify();
}
