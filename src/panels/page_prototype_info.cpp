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

#include "panels/page_prototype_info.h"
#include "designs/patterns.h"
#include "style/colored.h"
#include "style/thick.h"
#include "style/filled.h"
#include "style/interlace.h"
#include "style/outline.h"
#include "style/plain.h"
#include "style/sketch.h"
#include "style/emboss.h"
#include "makers/motif_maker/motif_maker.h"
#include "viewers/viewcontrol.h"
#include "geometry/transform.h"
#include "viewers/view.h"

using std::string;

Q_DECLARE_METATYPE(WeakDesignElementPtr);
Q_DECLARE_METATYPE(WeakPrototypePtr);

page_prototype_info:: page_prototype_info(ControlPanel * cpanel)  : panel_page(cpanel,"Prototype Info")
{
    setMouseTracking(true);


    QCheckBox * cbDrawMap        = new QCheckBox("DrawFigureMap");
    QCheckBox * cbDrawFeatures   = new QCheckBox("Draw Features");
    QCheckBox * cbDrawfigures    = new QCheckBox("Draw Figures");
    QCheckBox * cbHiliteFeatures = new QCheckBox("Highlight Features");
    QCheckBox * cbHiliteFigures  = new QCheckBox("Highlight Figures");

    int mode = vcontrol->getProtoViewMode();
    if (mode & PROTO_DRAW_MAP)
        cbDrawMap->setChecked(true);
    if (mode & PROTO_DRAW_FEATURES)
        cbDrawFeatures->setChecked(true);
    if (mode & PROTO_DRAW_FIGURES)
        cbDrawfigures->setChecked(true);
    if (mode & PROTO_HIGHLIGHT_FEATURES)
        cbHiliteFeatures->setChecked(true);
    if (mode & PROTO_HIGHLIGHT_FIGURES)
        cbHiliteFigures->setChecked(true);


    QPushButton * refreshButton = new QPushButton("Refresh");
    QHBoxLayout * hbox = new QHBoxLayout;

    hbox->addWidget(cbDrawFeatures);
    hbox->addWidget(cbDrawfigures);
    hbox->addWidget(cbHiliteFeatures);
    hbox->addWidget(cbHiliteFigures);
    hbox->addWidget(cbDrawMap);
    hbox->addStretch();
    hbox->addWidget(refreshButton);
    hbox->addStretch();

    protoTable = new AQTableWidget(this);
    protoTable->setRowCount(9);
    QStringList qslV;
    qslV << "Prototype" << "Tiling" << "Design Element" << "Feature" << "Figure" << "Scale" << "Rotate" << "Trans-X" << "Trans-Y";
    protoTable->setVerticalHeaderLabels(qslV);
    protoTable->horizontalHeader()->setVisible(false);
    protoTable->setMaximumWidth(880);
    protoTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    protoTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    protoTable->setSelectionBehavior(QAbstractItemView::SelectColumns);
    protoTable->setSelectionMode(QAbstractItemView::SingleSelection);
    vbox->addLayout(hbox);
    vbox->addSpacing(7);
    vbox->addWidget(protoTable);
    vbox->addStretch();

    connect(refreshButton,    &QPushButton::clicked,        this, &page_prototype_info::onEnter);
    connect(protoTable,       SIGNAL(cellClicked(int,int)), this,   SLOT(slot_prototypeSelected(int,int)));
    connect(cbDrawMap,        &QCheckBox::clicked, this, &page_prototype_info::drawMapClicked);
    connect(cbDrawFeatures,   &QCheckBox::clicked, this, &page_prototype_info::drawFeatureClicked);
    connect(cbDrawfigures,    &QCheckBox::clicked, this, &page_prototype_info::drawFigureClicked);
    connect(cbHiliteFeatures, &QCheckBox::clicked, this, &page_prototype_info::hiliteFeatureClicked);
    connect(cbHiliteFigures,  &QCheckBox::clicked, this, &page_prototype_info::hiliteFigureClicked);

}

void  page_prototype_info::refreshPage()
{
}

void page_prototype_info::onEnter()
{
    protoTable->clearContents();

    const QVector<PrototypePtr> & prototypes = motifMaker->getPrototypes();
    int col = 0;
    QTableWidgetItem * item;
    for (auto proto : prototypes)
    {
        QVector<DesignElementPtr> dels = proto->getDesignElements();
        for (auto del :  dels)
        {
            protoTable->setColumnCount(col + 1);

            item = new QTableWidgetItem(addr(proto.get()));
            item->setData(Qt::UserRole,QVariant::fromValue(WeakPrototypePtr(proto)));
            protoTable->setItem(PROTO_ROW_PROTO,col,item);

            item = new QTableWidgetItem(proto->getTiling()->getName());
            protoTable->setItem(PROTO_ROW_TILING,col,item);

            item = new QTableWidgetItem(addr(del.get()));
            item->setData(Qt::UserRole,QVariant::fromValue(WeakDesignElementPtr(del)));
            protoTable->setItem(PROTO_ROW_DEL,col,item);

            FeaturePtr fp = del->getFeature();
            QString astring = addr(fp.get());
            if (fp->isRegular())
            {
                astring += " sides=" + QString::number(fp->numPoints());
            }
            item = new QTableWidgetItem(astring);
            protoTable->setItem(PROTO_ROW_FEATURE,col,item);

            FigurePtr figp = del->getFigure();
            astring = addr(figp.get()) + "  " + figp->getFigureDesc();
            item = new QTableWidgetItem(astring);
            protoTable->setItem(PROTO_ROW_FIGURE,col,item);

            QTransform t = proto->getTransform(col);

            item = new QTableWidgetItem(QString::number(Transform::scalex(t),'f',6));
            protoTable->setItem(PROTO_ROW_SCALE,col,item);

            qreal rotation = Transform::rotation(t);
            qreal degrees  = qRadiansToDegrees(rotation);
            item = new QTableWidgetItem(QString::number(degrees,'f',6));
            protoTable->setItem(PROTO_ROW_ROT,col,item);

            item = new QTableWidgetItem(QString::number(Transform::transx(t),'f',6));
            protoTable->setItem(PROTO_ROW_X,col,item);

            item = new QTableWidgetItem(QString::number(Transform::transy(t),'f',6));
            protoTable->setItem(PROTO_ROW_Y,col,item);

            col++;
        }
    }

    protoTable->resizeColumnsToContents();
    protoTable->adjustTableSize(880);
    updateGeometry();
}

void page_prototype_info::slot_prototypeSelected(int row, int col)
{
    Q_UNUSED(row);

    QTableWidgetItem * twi = protoTable->item(PROTO_ROW_PROTO,col);
    QVariant var = twi->data(Qt::UserRole);
    if (var.canConvert<WeakPrototypePtr>())
    {
        WeakPrototypePtr wpp = var.value<WeakPrototypePtr>();
        PrototypePtr pp = wpp.lock();
        if (pp)
        {
            motifMaker->setSelectedPrototype(pp);
        }
    }

    twi = protoTable->item(PROTO_ROW_DEL,col);
    var = twi->data(Qt::UserRole);
    if (var.canConvert<WeakDesignElementPtr>())
    {
        WeakDesignElementPtr wdp = var.value<WeakDesignElementPtr>();
        if (wdp.lock())
        {
            motifMaker->setSelectedDesignElement(wdp.lock());
        }
    }

    emit sig_refreshView();
}

void  page_prototype_info::drawMapClicked(bool enb)
{
    vcontrol->setProtoViewMode(PROTO_DRAW_MAP,enb);
    emit sig_refreshView();
}

void page_prototype_info::drawFigureClicked(bool enb)
{
    vcontrol->setProtoViewMode(PROTO_DRAW_FIGURES,enb);
    emit sig_refreshView();
}

void page_prototype_info::drawFeatureClicked(bool enb)
{
    vcontrol->setProtoViewMode(PROTO_DRAW_FEATURES,enb);
    emit sig_refreshView();
}

void page_prototype_info::hiliteFigureClicked(bool enb)
{
    vcontrol->setProtoViewMode(PROTO_HIGHLIGHT_FIGURES,enb);
    emit sig_refreshView();
}

void page_prototype_info::hiliteFeatureClicked(bool enb)
{
    vcontrol->setProtoViewMode(PROTO_HIGHLIGHT_FEATURES,enb);
    emit sig_refreshView();
}
