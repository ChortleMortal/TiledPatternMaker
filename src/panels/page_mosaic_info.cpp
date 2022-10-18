#include <QHeaderView>

#include "panels/page_mosaic_info.h"
#include "motifs/motif.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "mosaic/design_element.h"
#include "mosaic/mosaic.h"
#include "mosaic/prototype.h"
#include "style/style.h"
#include "tile/tiling.h"
#include "widgets/panel_misc.h"

page_mosaic_info:: page_mosaic_info(ControlPanel *panel)  : panel_page(panel, "Mosaic Info")
{
    motifTable = new AQTableWidget(this);
    motifTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    motifTable->setColumnCount(3);
    motifTable->setColumnWidth(0,40);

    QStringList qslH;
    qslH << "Style" << "Tiling" << "Motif";
    motifTable->setHorizontalHeaderLabels(qslH);
    motifTable->verticalHeader()->setVisible(false);

    vbox->addWidget(motifTable);

    vbox->addStretch();
}

void  page_mosaic_info::onEnter()
{
    motifTable->clearContents();

    showMotifsFromStyles();

    motifTable->resizeColumnsToContents();
    motifTable->adjustTableSize();
    updateGeometry();
}

void  page_mosaic_info::onRefresh()
{
}

void page_mosaic_info::showMotifsFromStyles()
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
                MotifPtr  figp = del->getMotif();

                motifTable->setRowCount(row+1);

                QString stylename = style->getStyleDesc() + " " + addr(style.get());
                QTableWidgetItem * item =  new QTableWidgetItem(stylename);
                motifTable->setItem(row,COL_STYLE_NAME,item);

                QString tileName = pp->getTiling()->getName() + "  " + addr(pp->getTiling().get());
                item =  new QTableWidgetItem(tileName);
                motifTable->setItem(row,COL_TILING_NAME,item);

                QString figName = figp->getMotifDesc() + "  " + addr(figp.get());
                item =  new QTableWidgetItem(figName);
                motifTable->setItem(row,COL_TILE_TYPE,item);

                row++;
            }
        }
    }
}
