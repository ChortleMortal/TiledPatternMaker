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

#include "panels/page_style_figure_info.h"
#include "base/shared.h"
#include "base/tiledpatternmaker.h"
#include "designs/patterns.h"
#include "tapp/Prototype.h"
#include "style/Style.h"

using std::string;


page_style_figure_info:: page_style_figure_info(ControlPanel *panel)  : panel_page(panel, "Style Figure Info")
{
    figureTable = new QTableWidget(this);
    figureTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    figureTable->setColumnCount(2);
    figureTable->setColumnWidth(0,40);

    QStringList qslH;
    qslH << "Style" << "Figure";
    figureTable->setHorizontalHeaderLabels(qslH);
    figureTable->verticalHeader()->setVisible(false);

    vbox->addWidget(figureTable);

    vbox->addStretch();
}

void  page_style_figure_info::onEnter()
{
    figureTable->clearContents();

    showFiguresFromStyles();

    figureTable->resizeColumnsToContents();
    adjustTableSize(figureTable);
    updateGeometry();
}

void  page_style_figure_info::refreshPage()
{
}

void page_style_figure_info::showFiguresFromStyles()
{
    int row = 0;

    StyledDesign & sd = (config->designViewer == DV_LOADED_STYLE) ? workspace->getLoadedStyles() : workspace->getWsStyles();
    const StyleSet & sset = sd.getStyleSet();
    for (auto it = sset.begin(); it != sset.end(); it++)
    {
        StylePtr style  = *it;
        PrototypePtr pp = style->getPrototype();

        // from the prototype we can find things
        QVector<DesignElementPtr>  dels = pp->getDesignElements();
        TilingPtr  tp = pp->getTiling();
        // also from the tiling
        //QList <PlacedFeaturePtr> & qpf = tp->getPlacedFeatures();

        for (int i=0; i < dels.size(); i++)
        {
            DesignElementPtr del = dels[i];
            FigurePtr  figp = del->getFigure();

            figureTable->setRowCount(row+1);

            QString stylename = style->getStyleDesc();
            QTableWidgetItem * item =  new QTableWidgetItem(stylename);
            item->setData(Qt::UserRole,QVariant::fromValue(WeakTilingPtr(tp)));
            figureTable->setItem(row,COL_STYLE_NAME,item);

            QString figName = figp->getFigureDesc();
            item =  new QTableWidgetItem(figName);
            figureTable->setItem(row,COL_FIGURE_TYPE,item);

            row++;
        }
    }
}
