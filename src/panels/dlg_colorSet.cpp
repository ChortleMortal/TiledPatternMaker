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

#include "panels/dlg_colorSet.h"
#include "base/utilities.h"

DlgColorSet::DlgColorSet(ColorSet & cset, QWidget * parent) :  QDialog(parent), colorSet(cset)
{
    currentRow = 0;

    QVBoxLayout * vbox = new QVBoxLayout;

    QGridLayout * grid = new QGridLayout;

    QPushButton * addBtn = new QPushButton("Add");
    QPushButton * modBtn = new QPushButton("Modify");
    QPushButton * delBtn = new QPushButton("Delete");
    QPushButton * upBtn  = new QPushButton("Up");
    QPushButton * dwnBtn = new QPushButton("Down");

    grid->addWidget(addBtn,0,0);
    grid->addWidget(modBtn,0,1);
    grid->addWidget(delBtn,0,2);
    grid->addWidget(upBtn, 1,0);
    grid->addWidget(dwnBtn,1,1);
    vbox->addLayout(grid);

    table = new QTableWidget(this);
    table->horizontalHeader()->setVisible(false);
    table->verticalHeader()->setVisible(false);
    table->setRowCount(colorSet.size());
    table->setColumnCount(4);
    table->setColumnWidth(0,40);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    vbox->addWidget(table);

    QHBoxLayout * hbox = new QHBoxLayout;

    QPushButton * canBtn = new QPushButton("Cancel");
    QPushButton * okBtn  = new QPushButton("OK");

    hbox->addStretch();
    hbox->addWidget(canBtn);
    hbox->addWidget(okBtn);
    vbox->addLayout(hbox);

    setLayout(vbox);
    setMinimumWidth(400);
    setMinimumHeight(500);

    connect(addBtn, &QPushButton::clicked, this, &DlgColorSet::add);
    connect(modBtn, &QPushButton::clicked, this, &DlgColorSet::modify);
    connect(delBtn, &QPushButton::clicked, this, &DlgColorSet::del);
    connect(upBtn,  &QPushButton::clicked, this, &DlgColorSet::up);
    connect(dwnBtn, &QPushButton::clicked, this, &DlgColorSet::down);
    connect(canBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn,  &QPushButton::clicked, this, &DlgColorSet::slot_ok);

    displayTable();
}


void DlgColorSet::displayTable()
{
    table->clear();
    int count = colorSet.size();
    table->setRowCount(count);
    colorSet.resetIndex();
    int row = 0;
    for (int i=0; i < count; i++)
    {
        TPColor tpcolor = colorSet.getNextColor();
        QColor color    = tpcolor.color;
        QColor fullColor= color;
        fullColor.setAlpha(255);
        QTableWidgetItem * twi = new QTableWidgetItem(QString::number(row));
        table->setItem(row,0,twi);

        twi = new QTableWidgetItem(color.name());
        table->setItem(row,1,twi);

        QLabel * label = new QLabel;
        QVariant variant= fullColor;
        QString colcode = variant.toString();
        label->setStyleSheet("QLabel { background-color :"+colcode+" ;}");
        table->setCellWidget(row,2,label);

        QCheckBox * cb = new QCheckBox("   Hide");
        cb->setStyleSheet("padding-left:11px;");
        table->setCellWidget(row,3,cb);
        cb->setChecked(tpcolor.hidden);
        connect(cb, &QCheckBox::toggled, [this,row] { colorVisibilityChanged(row); });

        row++;
    }
    table->setCurrentCell(currentRow,1);
}

void DlgColorSet::add()
{
    currentRow = table->currentRow();

    AQColorDialog dlg(this);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    QColor newColor = dlg.selectedColor();
    if (newColor.isValid())
    {
        colorSet.addColor(newColor);
        displayTable();
        emit sig_colorsChanged();
    }
}

void DlgColorSet::modify()
{
    currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= colorSet.size())
        return;

    TPColor tpcolor = colorSet.getColor(currentRow);

    AQColorDialog dlg(tpcolor.color,this);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    QColor color = dlg.selectedColor();
    if (color.isValid())
    {
        tpcolor.color = color;
        colorSet.setColor(currentRow, tpcolor);
        displayTable();
        emit sig_colorsChanged();
    }
}

void DlgColorSet::del()
{
    currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= colorSet.size())
        return;

    colorSet.removeColor(currentRow);
    displayTable();
    emit sig_colorsChanged();
}

void DlgColorSet::slot_ok()
{
    accept();
}

void DlgColorSet::up()
{
    currentRow = table->currentRow();
    if (currentRow < 1)
        return;

    TPColor a = colorSet.getColor(currentRow);
    TPColor b = colorSet.getColor(currentRow-1);

    colorSet.setColor(currentRow-1, a);
    colorSet.setColor(currentRow,   b);

    currentRow--;
    displayTable();
    emit sig_colorsChanged();
}

void DlgColorSet::down()
{
    currentRow = table->currentRow();
    if (currentRow >= (colorSet.size()-1))
        return;

    TPColor a = colorSet.getColor(currentRow);
    TPColor b = colorSet.getColor(currentRow+1);

    colorSet.setColor(currentRow+1, a);
    colorSet.setColor(currentRow,   b);

    currentRow++;
    displayTable();
    emit sig_colorsChanged();
}

void DlgColorSet::colorVisibilityChanged(int row)
{
    qDebug() << "row=" << row;
    TPColor tpcolor = colorSet.getColor(row);
     QCheckBox * cb = dynamic_cast<QCheckBox*>(table->cellWidget(row,3));
    bool hide       = cb->isChecked();
    tpcolor.hidden  = hide;
    colorSet.setColor(row, tpcolor);

    displayTable();
    emit sig_colorsChanged();
}
