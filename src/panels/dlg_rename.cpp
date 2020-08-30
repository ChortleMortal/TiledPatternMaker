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

#include "panels/dlg_rename.h"

DlgRename::DlgRename(QWidget * parent) : QDialog(parent)
{
    QGridLayout * grid = new QGridLayout();
    QLabel * oldLabel = new QLabel("Old Name");
    QLabel * newLabel = new QLabel("New Name");

    oldEdit = new QLineEdit();
    oldEdit->setReadOnly(true);
    newEdit = new QLineEdit();

    oldEdit->setMinimumWidth(301);
    newEdit->setMinimumWidth(301);

    QPushButton * cancelBtn = new QPushButton("Cancel");
    QPushButton * renameBtn = new QPushButton("Rename");

    grid->addWidget(oldLabel,0,0);
    grid->addWidget(oldEdit,0,1);
    grid->addWidget(newLabel,1,0);
    grid->addWidget(newEdit,1,1);
    grid->addWidget(cancelBtn,2,0);
    grid->addWidget(renameBtn,2,1);
    setLayout(grid);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(renameBtn, &QPushButton::clicked, this, &QDialog::accept);

    newEdit->setFocus();
}


void DlgRename::keyPressEvent(QKeyEvent *evt)
{
    if(evt->key() == Qt::Key_Enter || evt->key() == Qt::Key_Return)
        return;
    QDialog::keyPressEvent(evt);
}
