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
#include "style/Colored.h"
#include "style/Thick.h"
#include "style/Filled.h"
#include "style/Interlace.h"
#include "style/Outline.h"
#include "style/Plain.h"
#include "style/Sketch.h"
#include "style/Emboss.h"

using std::string;


page_protos:: page_protos(ControlPanel * cpanel)  : panel_page(cpanel,"Prototype Info")
{
    setMouseTracking(true);

    wsProtoLabel = new QLabel;
    wsProtoLabel->setFixedWidth(251);

    QPushButton * refreshButton = new QPushButton("Refresh");
    QHBoxLayout * hbox = new QHBoxLayout;

    sourceStyle = new QRadioButton("Source: style");
    sourceWS    = new QRadioButton("Source: workspace");

    hbox->addWidget(wsProtoLabel);
    hbox->addWidget(refreshButton);
    hbox->addWidget(sourceStyle);
    hbox->addWidget(sourceWS);
    hbox->addStretch();

    protoTable = new QTableWidget(this);
    vbox->addLayout(hbox);
    vbox->addSpacing(7);
    vbox->addWidget(protoTable);
    vbox->addStretch();

    bgroup.addButton(sourceStyle, PV_STYLE);
    bgroup.addButton(sourceWS, PV_WS);
    bgroup.button(config->protoViewer)->setChecked(true);

    connect(refreshButton, &QPushButton::clicked,      this, &page_protos::onEnter);
    connect(&bgroup,       SIGNAL(buttonClicked(int)), this, SLOT(slot_display(int)));
}

void  page_protos::refreshPage()
{
    bgroup.button(config->protoViewer)->setChecked(true);

    eWsData wsdata     = (sourceStyle->isChecked()) ? WS_LOADED : WS_TILING;
    PrototypePtr proto = workspace->getPrototype(wsdata);
    if (proto)
    {
        wsProtoLabel->setText(QString("Style Proto ptr: is 0x%1").arg(addr(proto.get())));
    }
    else
    {
        wsProtoLabel->setText("WS Proto ptr: is null");
        return;
    }

    protoTable->clearContents();

    int row = 0;
    QVector<DesignElementPtr> dels = proto->getDesignElements();
    for (auto del :  dels)
    {
        protoTable->setRowCount(row + 1);

        QTableWidgetItem * item;
        item = new QTableWidgetItem(addr(del.get()));
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
        astring = addr(figp.get()) + "  " + figp->getFigTypeString();
        item = new QTableWidgetItem(astring);
        protoTable->setItem(row,PROTO_COL_FIGURE,item);

        QTransform t = proto->getTransform(row);
        item = new QTableWidgetItem(Transform::toInfoString(t));
        protoTable->setItem(row,PROTO_COL_TRANSFORM,item);

        row++;
    }

    protoTable->resizeColumnsToContents();
    adjustTableSize(protoTable);
    updateGeometry();
}

void page_protos::slot_display(int id)
{
    config->protoViewer = static_cast<eProtoViewer>(id);
    onEnter();
}

void page_protos::onEnter()
{
    protoTable->clear();

    protoTable->setColumnCount(4);

    QStringList qslH;
    qslH << "Design Element" << "Feature" << "Figure" << "Transform";
    protoTable->setHorizontalHeaderLabels(qslH);
    protoTable->verticalHeader()->setVisible(false);
}

