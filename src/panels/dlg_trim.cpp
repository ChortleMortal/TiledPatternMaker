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

#include "panels/dlg_trim.h"
#include "base/utilities.h"
#include "panels/layout_sliderset.h"

DlgTrim::DlgTrim(QWidget * parent) :  QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    trimmerX = new DoubleSpinSet("Trim X",0,-100,100);
    trimmerY = new DoubleSpinSet("TrimY ",0,-100,100);
    QPushButton * applyBtn = new QPushButton("Apply");
    QPushButton * doneBtn  = new QPushButton("Quit");

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(trimmerX);
    vbox->addLayout(trimmerY);
    vbox->addWidget(applyBtn);
    vbox->addWidget(doneBtn);

    setLayout(vbox);

    setFixedWidth(300);

    connect(doneBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(applyBtn,&QPushButton::clicked, this, &DlgTrim::slot_apply);
}

void DlgTrim::slot_ok()
{
    accept();
}

void DlgTrim::slot_apply()
{
    emit sig_apply(trimmerX->value(), trimmerY->value());
}
