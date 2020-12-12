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

#include "panels/page_design_elements.h"
#include "base/shared.h"
#include "base/tiledpatternmaker.h"
#include "base/utilities.h"
#include "designs/patterns.h"
#include "style/style.h"
#include "viewers/viewcontrol.h"
#include "viewers/placed_designelement_view.h"
#include "viewers/prototype_view.h"
#include "makers/motif_maker/motif_maker.h"

Q_DECLARE_METATYPE(WeakFeaturePtr)
Q_DECLARE_METATYPE(WeakPrototypePtr)

page_design_elements:: page_design_elements(ControlPanel * cpanel)  : panel_page(cpanel,"Des Element Info")
{
	// top line
    QLabel  * tilingLabel    = new QLabel("Tiling:");
    protoListBox             = new QComboBox();
    protoListBox->setMinimumWidth(131);

    QPushButton * refreshButton = new QPushButton("Refresh");
    QHBoxLayout * hbox = new QHBoxLayout;

    hbox->addWidget(tilingLabel);
    hbox->addWidget(protoListBox);
    hbox->addStretch();
    hbox->addWidget(refreshButton);

    vbox->addLayout(hbox);

    delTable = new AQTableWidget(this);
    delTable->setColumnCount(5);
    delTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    QStringList qslH;
    qslH << "DEL" << "Feature" << "Figure" << "Desc" << "Figure Type" << "Infill Type";
    delTable->setHorizontalHeaderLabels(qslH);
    delTable->verticalHeader()->setVisible(false);
    vbox->addWidget(delTable);

    vbox->addStretch();


    connect(refreshButton,      &QPushButton::clicked,      this,   &page_design_elements::onEnter);
    connect(delTable,           &QTableWidget::cellClicked, this,   &page_design_elements::slot_rowSelected);

    connect(theApp,  &TiledPatternMaker::sig_tilingLoaded,     this,   &page_design_elements::onEnter);
    connect(theApp,  &TiledPatternMaker::sig_mosaicLoaded,     this,   &page_design_elements::onEnter);
    connect(theApp,  &TiledPatternMaker::sig_loadedDesign,     this,   &page_design_elements::onEnter);

    connect(vcontrol,&ViewControl::sig_selected_proto_changed,this, &page_design_elements::onEnter);

    connect(protoListBox, SIGNAL(currentIndexChanged(int)), this,   SLOT(slot_prototypeSelected(int)));
}

void  page_design_elements::refreshPage()
{
}

void  page_design_elements::onEnter()
{
    protoListBox->blockSignals(true);
    protoListBox->clear();
    const QVector<PrototypePtr> & protos = motifMaker->getPrototypes();
    for (auto proto : protos)
    {
        protoListBox->addItem(proto->getTiling()->getName(),QVariant::fromValue(WeakPrototypePtr(proto)));
    }

    PrototypePtr proto = motifMaker->getSelectedPrototype();
    if (proto)
    {
        QString name = proto->getTiling()->getName();
        int index = protoListBox->findText(name);
        protoListBox->setCurrentIndex(index);
    }
    protoListBox->blockSignals(false);

    delTable->clearContents();

    if (proto)
    {
        QVector<DesignElementPtr> & dels = proto->getDesignElements();
        int row = 0;
        for (auto it = dels.begin(); it != dels.end(); it++)
        {
            delTable->setRowCount(row+1);

            DesignElementPtr de = *it;
            FeaturePtr       fp = de->getFeature();
            FigurePtr      figp = de->getFigure();

            // DesignElement
            QTableWidgetItem * twi = new QTableWidgetItem(addr(de.get()));
            delTable->setItem(row,DEL_COL_DEL,twi);

            //  "Feature"
            twi = new QTableWidgetItem(addr(fp.get()));
            twi->setData(Qt::UserRole,QVariant::fromValue(WeakFeaturePtr(fp)));
            delTable->setItem(row,DEL_COL_FEATURE,twi);

            // "Figure"
            twi = new QTableWidgetItem(addr(figp.get()));
            delTable->setItem(row,DEL_COL_FIGURE,twi);

            // "Desc"
            twi = new QTableWidgetItem(figp->getFigureDesc());
            delTable->setItem(row,DEL_COL_DESC,twi);

            // "Figure Type"
            twi = new QTableWidgetItem(sFigType[figp->getFigType()]);
            delTable->setItem(row,DEL_COL_FIG_TYPE,twi);

            row++;
        }
    }

    delTable->resizeColumnsToContents();
    delTable->adjustTableSize();
    updateGeometry();
}

void page_design_elements::slot_rowSelected(int row, int col)
{
    Q_UNUSED(col)
    QTableWidgetItem * item = delTable->item(row,DEL_COL_FEATURE);
    QVariant var = item->data(Qt::UserRole);
    if (var.canConvert<WeakFeaturePtr>())
    {
        WeakFeaturePtr wfp = var.value<WeakFeaturePtr>();
        vcontrol->selectFeature(wfp);
        emit sig_refreshView();
    }
}

void page_design_elements::slot_prototypeSelected(int row)
{
    WeakPrototypePtr wpp;
    QVariant var = protoListBox->itemData(row);
    if (var.canConvert<WeakPrototypePtr>())
    {
        wpp = var.value<WeakPrototypePtr>();
        PrototypePtr pp = wpp.lock();
        motifMaker->setSelectedPrototype(pp);
        emit sig_refreshView();
        onEnter();
    }
}
