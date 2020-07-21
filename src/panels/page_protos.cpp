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

#include "panels/page_protos.h"
#include "designs/patterns.h"
#include "style/colored.h"
#include "style/thick.h"
#include "style/filled.h"
#include "style/interlace.h"
#include "style/outline.h"
#include "style/plain.h"
#include "style/sketch.h"
#include "style/emboss.h"

using std::string;

Q_DECLARE_METATYPE(DesignElementPtr);
Q_DECLARE_METATYPE(PrototypePtr);

page_protos:: page_protos(ControlPanel * cpanel)  : panel_page(cpanel,"Prototype Info")
{
    setMouseTracking(true);

    QPushButton * refreshButton = new QPushButton("Refresh");
    QHBoxLayout * hbox = new QHBoxLayout;

    hbox->addWidget(refreshButton);
    hbox->addStretch();

    protoTable = new AQTableWidget(this);
    protoTable->setColumnCount(5);
    QStringList qslH;
    qslH << "Prototype" << "Design Element" << "Feature" << "Figure" << "Transform";
    protoTable->setHorizontalHeaderLabels(qslH);
    protoTable->verticalHeader()->setVisible(false);
    protoTable->setSelectionMode(QAbstractItemView::SingleSelection);
    protoTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    vbox->addLayout(hbox);
    vbox->addSpacing(7);
    vbox->addWidget(protoTable);
    vbox->addStretch();

    connect(refreshButton, &QPushButton::clicked,      this, &page_protos::onEnter);
    connect(protoTable, SIGNAL(cellClicked(int,int)), this,   SLOT(slot_prototypeSelected(int,int)));
}

void  page_protos::refreshPage()
{
}

void page_protos::onEnter()
{
    protoTable->clearContents();

    QVector<PrototypePtr> prototypes = workspace->getPrototypes();
    int row = 0;
    QTableWidgetItem * item;
    for (auto proto : prototypes)
    {
        QVector<DesignElementPtr> dels = proto->getDesignElements();
        for (auto del :  dels)
        {
            protoTable->setRowCount(row + 1);

            item = new QTableWidgetItem(addr(proto.get()));
            item->setData(Qt::UserRole,QVariant::fromValue(proto));
            protoTable->setItem(row,PROTO_COL_PROTO,item);

            item = new QTableWidgetItem(addr(del.get()));
            item->setData(Qt::UserRole,QVariant::fromValue(del));
            protoTable->setItem(row,PROTO_COL_DEL,item);

            FeaturePtr fp = del->getFeature();
            QString astring = addr(fp.get());
            if (fp->isRegular())
            {
                astring += " sides=" + QString::number(fp->numPoints());
            }
            item = new QTableWidgetItem(astring);
            protoTable->setItem(row,PROTO_COL_FEATURE,item);

            FigurePtr figp = del->getFigure();
            astring = addr(figp.get()) + "  " + figp->getFigureDesc();
            item = new QTableWidgetItem(astring);
            protoTable->setItem(row,PROTO_COL_FIGURE,item);

            QTransform t = proto->getTransform(row);
            item = new QTableWidgetItem(Transform::toInfoString(t));
            protoTable->setItem(row,PROTO_COL_TRANSFORM,item);

            row++;
        }
    }

    protoTable->resizeColumnsToContents();
    protoTable->adjustTableSize();
    updateGeometry();
}

void page_protos::slot_prototypeSelected(int row, int col)
{
    Q_UNUSED(col);

    PrototypePtr pp;
    QTableWidgetItem * twi = protoTable->item(row,PROTO_COL_PROTO);
    QVariant var = twi->data(Qt::UserRole);
    if (var.canConvert<PrototypePtr>())
    {
        pp = var.value<PrototypePtr>();
    }
    if (pp)
    {
        workspace->setSelectedPrototype(pp);
    }

    DesignElementPtr dp;
    twi = protoTable->item(row,PROTO_COL_DEL);
    var = twi->data(Qt::UserRole);
    if (var.canConvert<DesignElementPtr>())
    {
        dp = var.value<DesignElementPtr>();
    }
    if (dp)
    {
        workspace->setSelectedDesignElement(dp);
    }
}

