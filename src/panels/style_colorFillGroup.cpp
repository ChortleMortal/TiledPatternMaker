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

#include "panels/style_colorFillGroup.h"
#include "panels/dlg_colorSet.h"
#include "base/utilities.h"

StyleColorFillGroup::StyleColorFillGroup(FilledEditor * editor, ColorGroup & cgroup, QVBoxLayout *vbox) : colorGroup(cgroup)
{
    ed = editor;
    Q_ASSERT(editor);

    connect(this, &StyleColorFillGroup::sig_colorsChanged, editor, &FilledEditor::slot_colorsChanged);

    connect(&mapper, SIGNAL(mapped(int)), this, SLOT(slot_colorSetVisibilityChanged(int)));

    QGridLayout * grid = new QGridLayout;

    QPushButton * addBtn = new QPushButton("Add");
    QPushButton * modBtn = new QPushButton("Modify");
    QPushButton * delBtn = new QPushButton("Delete");
    QPushButton * upBtn  = new QPushButton("Up");
    QPushButton * dwnBtn = new QPushButton("Down");
    QPushButton * rptBtn = new QPushButton("Repeat Set");
    QPushButton * cpyBtn = new QPushButton("Copy Set");
    QPushButton * pstBtn = new QPushButton("Paste Set");

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
    table->setColumnCount(7);
    table->setColumnWidth(0,35);
    table->setColumnWidth(1,40);
    table->setColumnWidth(2,40);
    table->setColumnWidth(3,70);
    table->setColumnWidth(4,80);
    table->setColumnWidth(5,70);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setRowCount(1);

    table->horizontalHeader()->setVisible(true);
    QStringList qslH;
    qslH << "" << "Count" << "Sides" << "Area" << "Hide/Select" << "Edit" << "Colors";
    table->setHorizontalHeaderLabels(qslH);
    table->setMinimumHeight(501);

    vbox->addWidget(table);

    connect(addBtn, &QPushButton::clicked, this, &StyleColorFillGroup::add);
    connect(modBtn, &QPushButton::clicked, this, &StyleColorFillGroup::modify);
    connect(delBtn, &QPushButton::clicked, this, &StyleColorFillGroup::del);
    connect(upBtn,  &QPushButton::clicked, this, &StyleColorFillGroup::up);
    connect(dwnBtn, &QPushButton::clicked, this, &StyleColorFillGroup::down);
    connect(rptBtn, &QPushButton::clicked, this, &StyleColorFillGroup::rptSet);
    connect(cpyBtn, &QPushButton::clicked, this, &StyleColorFillGroup::copySet);
    connect(pstBtn, &QPushButton::clicked, this, &StyleColorFillGroup::pasteSet);
    connect(table,  &QTableWidget::cellClicked,       this, &StyleColorFillGroup::slot_click);
    connect(table,  &QTableWidget::cellDoubleClicked, this, &StyleColorFillGroup::slot_double_click);

    createTable();
}

void StyleColorFillGroup::createTable()
{
    Filled * filled = ed->getFilled();
    FaceGroup & faceGroup = filled->getFaceGroup();

    int rowCount = qMax(faceGroup.size(),colorGroup.size());
    table->setRowCount(rowCount);

    int row = 0;
    for (auto it = faceGroup.begin(); it != faceGroup.end(); it++)
    {
        FaceSetPtr fsp = *it;

        ColorSet cset  = fsp->colorSet;

        QTableWidgetItem * item = new QTableWidgetItem(QString::number(row));
        table->setItem(row,0,item);

        item = new QTableWidgetItem(QString::number(fsp->size()));
        table->setItem(row,1,item);

        item = new QTableWidgetItem(QString::number(fsp->sides));
        table->setItem(row,2,item);

        item = new QTableWidgetItem(QString::number(fsp->area));
        table->setItem(row,3,item);

        QLabel * label = new QLabel("X");
        label->setFixedWidth(40);
        table->setCellWidget(row,4,label);

        label = new QLabel;
        table->setCellWidget(row,5,label);

        row++;
    }

    table->resizeColumnToContents(5);
}

void StyleColorFillGroup::display(ColorGroup & cgroup)
{
    qDebug() << "color groups has" << cgroup.size() << "color sets";
    Filled * filled = ed->getFilled();
    FaceGroup & faceGroup = filled->getFaceGroup();

    int rowCount = qMax(faceGroup.size(),colorGroup.size());
    table->setRowCount(rowCount);

    int size = qMin(cgroup.size(),table->rowCount());
    for (int i=0; i < size; i++)
    {
        int row = i;

        QCheckBox * cb = new QCheckBox("Hide");
        cb->setStyleSheet("padding-left:11px;");
        table->setCellWidget(row,4,cb);
        cb->setChecked(cgroup.isHidden(row));
        mapper.setMapping(cb,row);
        QObject::connect(cb, SIGNAL(toggled(bool)), &mapper, SLOT(map()), Qt::UniqueConnection);

        QPushButton * btn = new QPushButton("Edit");
        btn->setFixedWidth(40);
        table->setCellWidget(row,5,btn);
        connect(btn, &QPushButton::clicked, this, &StyleColorFillGroup::slot_edit, Qt::UniqueConnection);

        ColorSet & cset = cgroup.getColorSet(i);
        AQWidget * widget = cset.createWidget();
        table->setCellWidget(row,6,widget);
    }
    table->setColumnWidth(5,45);
    table->resizeColumnToContents(6);
}

void StyleColorFillGroup::slot_edit()
{
    int row = table->currentRow();
    if (row < 0 || row >= colorGroup.size())
    {
        return;
    }

    ColorSet & colorSet = colorGroup.getColorSet(row);

    DlgColorSet dlg(colorSet);

    connect(&dlg, &DlgColorSet::sig_colorsChanged, this, &StyleColorFillGroup::sig_colorsChanged);

    dlg.exec();

    display(colorGroup);
}


void StyleColorFillGroup::add()
{
    ColorSet set;
    set.addColor(Qt::black);
    colorGroup.addColorSet(set);

    emit sig_colorsChanged();

    display(colorGroup);
}

void StyleColorFillGroup::modify()
{
    slot_edit();
}

void StyleColorFillGroup::del()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= colorGroup.size())
        return;

    colorGroup.removeColorSet(currentRow);

    emit sig_colorsChanged();

    display(colorGroup);
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

    display(colorGroup);
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

    display(colorGroup);
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

    display(colorGroup);
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

    display(colorGroup);
}

void StyleColorFillGroup::slot_colorSetVisibilityChanged(int row)
{
    qDebug() << "slot_colorVisibilityChanged row=" << row;

    if (row >= colorGroup.size())
    {
        qDebug() << "ignored - out of range of color set";
    }

    QCheckBox * cb  = dynamic_cast<QCheckBox*>(table->cellWidget(row,4));
    bool hide       = cb->isChecked();
    qDebug() << "hide state="  << hide;

    colorGroup.hide(row, hide);

    emit sig_colorsChanged();

    display(colorGroup);

    qDebug() << "slot_colorVisibilityChanged: done";
}

void StyleColorFillGroup::slot_click(int row, int col)
{
    FaceGroup & fg = ed->getFilled()->getFaceGroup();
    if (col == 4)
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

void StyleColorFillGroup::slot_double_click(int row, int col)
{
    Q_UNUSED(row);
    Q_UNUSED(col);

    modify();
}
