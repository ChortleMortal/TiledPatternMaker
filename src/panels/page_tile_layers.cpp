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

#include "page_tile_layers.h"
#include "base/patterns.h"
#include "base/canvas.h"
#include "viewers/workspaceviewer.h"

using std::string;

Q_DECLARE_METATYPE(Layer*)

page_tileLayers:: page_tileLayers(ControlPanel *panel)  : panel_page(panel,"Tile Layer Info")
{
    tileTable = new QTableWidget(this);

    vbox->addWidget(tileTable);
    vbox->addStretch();

    connect(&tileVisMapper, SIGNAL(mapped(int)), this, SLOT(slot_tileVisibilityChanged(int)),Qt::UniqueConnection);
    connect(&zMapper,       SIGNAL(mapped(int)), this, SLOT(slot_zChanged(int)),Qt::UniqueConnection);
    connect(&alignMapper,   SIGNAL(mapped(int)), this, SLOT(slot_alignChanged(int)),Qt::UniqueConnection);


    refreshPage();
}

void  page_tileLayers::refreshPage()
{
}

void page_tileLayers::onEnter()
{
    tileTable->clear();
    tileTable->setRowCount(2);
    tileTable->setColumnCount(5);
    tileTable->setColumnWidth(TILE_COL_BOUNDS,280);

    QStringList qslH;
    qslH << "Design" << "Layer" << "Bounds/Deltas" << "z-level" << "" ;
    tileTable->setHorizontalHeaderLabels(qslH);
    tileTable->verticalHeader()->setVisible(false);

    QVector<Layer*> layers = viewer->getActiveLayers();
    int row = 0;

    for (auto it = layers.begin(); it != layers.end(); it++)
    {
        Layer * layer = * it;

        tileTable->setRowCount(row+1);

        // design number
        QTableWidgetItem * twi = new QTableWidgetItem(layer->getName());
        twi->setData(Qt::UserRole,QVariant::fromValue(layer));
        tileTable->setItem(row,TILE_COL_DESIGN_NUMBER,twi);

        // layer number and visibility
        QCheckBox * cb = new QCheckBox();
        tileTable->setCellWidget(row,TILE_COL_LAYER_VISIBILITY,cb);
        cb->setChecked(layer->isVisible());

        // tile bounds anmd delta
        Bounds b = layer->getBounds();
        Bounds d = layer->getDeltas();

        char buf[132];
        memset(buf,0,sizeof(buf));
        sprintf(buf,"bounds(%.2f %.2f %.2f) deltas(%.2f %.2f %.2f)",b.left,b.top,b.width,d.left,d.top,d.width);
        QString s(buf);
        twi = new QTableWidgetItem(s);
        tileTable->setItem(row,TILE_COL_BOUNDS,twi);

        connect(cb, SIGNAL(toggled(bool)), &tileVisMapper, SLOT(map()),Qt::UniqueConnection);
        tileVisMapper.setMapping(cb,row);

        // z-level
        qreal z = layer->zValue();
        QDoubleSpinBox * zBox = new QDoubleSpinBox;
        zBox->setRange(-10,10);
        zBox->setValue(z);
        tileTable->setCellWidget(row,TILE_COL_Z,zBox);

        connect(zBox, SIGNAL(valueChanged(double)), &zMapper, SLOT(map()),Qt::UniqueConnection);
        zMapper.setMapping(zBox,row);

        // align
        if (row != 0)
        {
            QPushButton * abtn = new QPushButton("Align");
            tileTable->setCellWidget(row,TILE_COL_ALIGN,abtn);

            connect(abtn, SIGNAL(clicked(bool)), &alignMapper, SLOT(map()),Qt::UniqueConnection);
            alignMapper.setMapping(abtn,row);
        }

        row++;
    }

    tileTable->resizeColumnsToContents();
    adjustTableSize(tileTable);

}

void page_tileLayers::slot_tileVisibilityChanged(int row)
{
    qDebug() << "visibility changed: row=" << row;

    QCheckBox * cb = dynamic_cast<QCheckBox*>(tileTable->cellWidget(row,TILE_COL_LAYER_VISIBILITY));
    bool visible   = cb->isChecked();

    QTableWidgetItem * twi = tileTable->item(row,TILE_COL_DESIGN_NUMBER);
    QVariant tmp = twi->data(Qt::UserRole);
    Layer * layer = tmp.value<Layer *>();
    layer->setVisible(visible);
    layer->forceRedraw();
}

void page_tileLayers::slot_zChanged(int row)
{
    qDebug() << "z-level changed: row=" << row;

    QDoubleSpinBox * dsp = dynamic_cast<QDoubleSpinBox*>(tileTable->cellWidget(row,TILE_COL_Z));
    qreal z = dsp->value();

    QTableWidgetItem * twi = tileTable->item(row,TILE_COL_DESIGN_NUMBER);
    QVariant tmp = twi->data(Qt::UserRole);
    Layer * layer = tmp.value<Layer *>();

    layer->setZValue(z);
    layer->forceRedraw();
}

void page_tileLayers::slot_alignChanged(int row)
{
    qDebug() << "visibility changed: row=" << row;

    // get from settings
    QTableWidgetItem * twi = tileTable->item(0,TILE_COL_DESIGN_NUMBER);
    QVariant tmp = twi->data(Qt::UserRole);
    Layer * layer = tmp.value<Layer *>();

    Bounds b = layer->getBounds();
    Bounds d = layer->getDeltas();

    // apply settings to
    twi = tileTable->item(row,TILE_COL_DESIGN_NUMBER);
    tmp = twi->data(Qt::UserRole);
    layer = tmp.value<Layer *>();

    layer->setBounds(b);
    layer->setDeltas(d);

    layer->forceUpdateLayer();

    onEnter();
}
