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

#include "page_designs.h"

using std::string;

page_designs::page_designs(ControlPanel * panel)  : panel_page(panel, "Design Info")
{
    designTable = new QTableWidget();
    //connect(designTable,    SIGNAL(cellClicked(int,int)),   this,   SLOT(slot_designTableCell(int,int)));
    //connect(&designVisMapper, SIGNAL(mapped(int)), this, SLOT(slot_designVisibilityChanged(int)),Qt::UniqueConnection);
    QObject::connect(&designVisMapper, SIGNAL(mapped(int)), this, SLOT(slot_designVisibilityChanged(int)));

    vbox->addWidget(designTable);

    refreshPage();

    vbox->addStretch();
}

void page_designs::onEnter()
{
    designTable->clear();
    designTable->setColumnCount(2);
    designTable->setRowCount(3);

    QStringList qslH;
    qslH << "Vis" << "Design" ;
    designTable->setHorizontalHeaderLabels(qslH);
    designTable->verticalHeader()->setVisible(false);

    QVector<DesignPtr> & designs = workspace->getDesigns();
    int row = 0;
    for (int i=0; i < designs.size(); i++ )
    {
        designTable->setRowCount(row+1);

        DesignPtr d = designs[i];
        QString title = d->getTitle();

        QCheckBox * cb = new QCheckBox();
        designTable->setCellWidget(row,DESIGN_COL_VISIBILITY,cb);
        cb->setChecked(d->isVisible());

        QTableWidgetItem * twi = new QTableWidgetItem(d->getTitle());
        designTable->setItem(row,DESIGN_COL_INFO,twi);

        QObject::connect(cb, SIGNAL(toggled(bool)), &designVisMapper, SLOT(map()));
        designVisMapper.setMapping(cb,row);

        row++;
    }

    designTable->resizeColumnsToContents();
    adjustTableSize(designTable);
}

void page_designs::refreshPage()
{

}

void page_designs::slot_designVisibilityChanged(int row)
{
    qDebug() << "design visibility changed: row=" << row;

    QCheckBox * cb = (QCheckBox*)designTable->cellWidget(row,0);
    bool visible   = cb->isChecked();

    QVector<DesignPtr> & designs = workspace->getDesigns();
    DesignPtr d = designs[row];
    d->setVisible(visible);
}

void page_designs::slot_designTableCell(int row, int col)
{
    Q_UNUSED(col);
    QCheckBox * cb = (QCheckBox*)designTable->cellWidget(row,0);
    cb->setChecked(!cb->isChecked());
}


