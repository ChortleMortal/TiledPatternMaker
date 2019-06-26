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

#include "dlg_rebase.h"

DlgRebase::DlgRebase(QWidget * parent) : QDialog(parent)
{
    QGridLayout * grid = new QGridLayout();
    QLabel * oldLabel = new QLabel("Old Version");
    QLabel * newLabel = new QLabel("New Version");
    QLabel * descrip  = new QLabel("Deletes all versions higher than new version");

    oldVersion = new QLineEdit();
    oldVersion->setReadOnly(true);
    newVersion = new QLineEdit();

    QPushButton * cancelBtn = new QPushButton("Cancel");
    QPushButton * renameBtn = new QPushButton("Rebase");

    grid->addWidget(oldLabel,0,0);
    grid->addWidget(oldVersion,0,1);
    grid->addWidget(newLabel,1,0);
    grid->addWidget(newVersion,1,1);
    grid->addWidget(descrip,2,0,1,2);
    grid->addWidget(cancelBtn,3,0);
    grid->addWidget(renameBtn,3,1);
    setLayout(grid);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(renameBtn, &QPushButton::clicked, this, &QDialog::accept);

    newVersion->setFocus();
}
