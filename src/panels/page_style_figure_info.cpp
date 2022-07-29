#include <QHeaderView>

#include "panels/page_style_figure_info.h"
#include "figures/figure.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "mosaic/design_element.h"
#include "mosaic/mosaic.h"
#include "mosaic/prototype.h"
#include "style/style.h"
#include "tile/tiling.h"
#include "widgets/panel_misc.h"

page_mosaic_info:: page_mosaic_info(ControlPanel *panel)  : panel_page(panel, "Mosaic Info")
{
    figureTable = new AQTableWidget(this);
    figureTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    figureTable->setColumnCount(3);
    figureTable->setColumnWidth(0,40);

    QStringList qslH;
    qslH << "Style" << "Tiling" << "Figure";
    figureTable->setHorizontalHeaderLabels(qslH);
    figureTable->verticalHeader()->setVisible(false);

    vbox->addWidget(figureTable);

    vbox->addStretch();
}

void  page_mosaic_info::onEnter()
{
    figureTable->clearContents();

    showFiguresFromStyles();

    figureTable->resizeColumnsToContents();
    figureTable->adjustTableSize();
    updateGeometry();
}

void  page_mosaic_info::onRefresh()
{
}

void page_mosaic_info::showFiguresFromStyles()
{
    int row = 0;
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        const StyleSet sset = mosaic->getStyleSet();
        for (auto& style : sset)
        {
            PrototypePtr pp = style->getPrototype();

            QVector<DesignElementPtr>  dels = pp->getDesignElements();
            for (int i=0; i < dels.size(); i++)
            {
                DesignElementPtr del = dels[i];
                FigurePtr  figp = del->getFigure();

                figureTable->setRowCount(row+1);

                QString stylename = style->getStyleDesc() + " " + addr(style.get());
                QTableWidgetItem * item =  new QTableWidgetItem(stylename);
                figureTable->setItem(row,COL_STYLE_NAME,item);

                QString tileName = pp->getTiling()->getName() + "  " + addr(pp->getTiling().get());
                item =  new QTableWidgetItem(tileName);
                figureTable->setItem(row,COL_TILING_NAME,item);

                QString figName = figp->getFigureDesc() + "  " + addr(figp.get());
                item =  new QTableWidgetItem(figName);
                figureTable->setItem(row,COL_FIGURE_TYPE,item);

                row++;
            }
        }
    }
}
