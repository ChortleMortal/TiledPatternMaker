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

#include "page_tile_colors.h"
#include "base/patterns.h"
#include "base/canvas.h"
#include "base/tiledpatternmaker.h"
#include "tile/Tiling.h"
#include "panels/dlg_colorSet.h"

page_tileColorMaker:: page_tileColorMaker(ControlPanel *panel)  : panel_page(panel,"Tile Color Maker")
{
    createSourceSelect();

    table = new QTableWidget(this);
    table->setMinimumHeight(601);

    vbox->addWidget(table);
    vbox->addStretch();

    connect(maker,  &TiledPatternMaker::sig_loadedTiling,   this,   &page_tileColorMaker::slot_loadedTiling);
    connect(maker,  &TiledPatternMaker::sig_loadedXML,      this,   &page_tileColorMaker::slot_loadedXML);

    refreshPage();
}
void page_tileColorMaker::createSourceSelect()
{
    QLabel * makerLabel = new QLabel("Designer Source:");

    radioLoadedStyleTileView = new QRadioButton("Style");
    radioWSTileView = new QRadioButton("Workspace");

    labTileMakerView = new QLabel;
    labXmlTileView   = new QLabel;

    AQHBoxLayout * hbox = new AQHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(makerLabel);
    hbox->addSpacing(11);
    hbox->addWidget(radioLoadedStyleTileView);
    hbox->addWidget(labXmlTileView);
    hbox->addSpacing(17);
    hbox->addWidget(radioWSTileView);
    hbox->addWidget(labTileMakerView);
    hbox->addStretch();

    makerSourceBox = new QWidget;
    makerSourceBox->setLayout(hbox);

    vbox->addWidget(makerSourceBox);

    selectedTiling = new QLabel("Selected tiling:");
    vbox->addWidget(selectedTiling);

    vbox->addSpacing(7);

    tilingGroup3.addButton(radioLoadedStyleTileView,TD_STYLE);
    tilingGroup3.addButton(radioWSTileView,TD_WORKSPACE);

    connect(&tilingGroup3,  SIGNAL(buttonClicked(int)), this,    SLOT(slot_sourceSelect(int)));
}

void page_tileColorMaker::slot_sourceSelect(int id)
{
    config->tilingMakerViewer = eTilingMakerView(id);
    onEnter();
    emit sig_viewWS();
    emit sig_tilingChanged();
}


void  page_tileColorMaker::refreshPage()
{
    TilingPtr tiling = workspace->getLoadedStyles().getTiling();
    QString txt = addr(tiling.get());
    if (tiling)
    {
        txt += " ";
        txt += tiling->getName();
    }
    labXmlTileView->setText(txt);

    tiling = workspace->getTiling();
    txt = addr(tiling.get());
    if (tiling)
    {
        txt += " ";
        txt += tiling->getName();
    }
    labTileMakerView->setText(txt);

    tilingGroup3.button(config->tilingMakerViewer)->setChecked(true);

    tiling = getSourceTiling();
    if (tiling)
        txt = addr(tiling.get()) + " " + tiling->getName();
    else
        txt.clear();
    selectedTiling->setText(txt);
}

void page_tileColorMaker::onEnter()
{
    tilingGroup3.button(config->tilingMakerViewer)->setChecked(true);

    table->clear();
    table->setRowCount(2);
    table->setColumnCount(4);
    table->setColumnWidth(TILE_COLORS_COLORS,340);

    QStringList qslH;
    qslH << "Addr" << "Sides" << "Btn" << "Colors" ;
    table->setHorizontalHeaderLabels(qslH);
    table->verticalHeader()->setVisible(false);

    int row = 0;
    TilingPtr tiling = getSourceTiling();
    if (!tiling)
    {
        return;
    }
    qlfp = tiling->getUniqueFeatures();
    table->setRowCount(qlfp.size());
    for (auto it = qlfp.begin(); it != qlfp.end(); it++)
    {
        FeaturePtr fp = *it;

        QTableWidgetItem * twi = new QTableWidgetItem(addr(fp.get()));
        table->setItem(row,TILE_COLORS_ADDR,twi);

        QString str = QString("%1 %2").arg(fp->numPoints()).arg((fp->isRegular()) ? "Regular" : "Not-regular");
        twi = new QTableWidgetItem(str);
        table->setItem(row,TILE_COLORS_SIDES,twi);

        QPushButton * btn = new QPushButton("Edit");
        table->setCellWidget(row,TILE_COLORS_BTN,btn);
        connect(btn, &QPushButton::clicked, this, &page_tileColorMaker::slot_edit);

        ColorSet & bkgdColors = fp->getBkgdColors();
        AQWidget * widget = bkgdColors.createWidget();
        table->setCellWidget(row,TILE_COLORS_COLORS,widget);

        row++;
    }
    adjustTableSize(table);
}

TilingPtr page_tileColorMaker::getSourceTiling()
{
    TilingPtr tiling;
    switch (config->tilingMakerViewer)
    {
    case TD_STYLE:
        tiling = workspace->getLoadedStyles().getTiling();
        break;
    case TD_WORKSPACE:
        tiling = workspace->getTiling();
        break;
    }
    return tiling;
}

void page_tileColorMaker::slot_edit()
{
    int row = table->currentRow();
    if (row < 0 || row >= qlfp.size())
        return;

    FeaturePtr fp = qlfp[row];
    ColorSet & colorSet = fp->getBkgdColors();
    DlgColorSet dlg(colorSet,this);
    dlg.exec();

    emit sig_render();

    onEnter();
}

void  page_tileColorMaker::slot_loadedXML(QString name)
{
    Q_UNUSED(name);
    onEnter();
}

void page_tileColorMaker::slot_loadedTiling (QString name)
{
    Q_UNUSED(name);
    onEnter();
}
