#include <QHeaderView>

#include "model/makers/mosaic_maker.h"
#include "model/prototypes/prototype.h"
#include "model/makers/tiling_maker.h"
#include "model/prototypes/design_element.h"
#include "model/mosaics/mosaic.h"
#include "model/motifs/motif.h"
#include "gui/top/controlpanel.h"
#include "gui/panels/page_mosaic_info.h"
#include "gui/panels/panel_misc.h"
#include "model/styles/style.h"
#include "model/tilings/tiling.h"

page_mosaic_info:: page_mosaic_info(ControlPanel *panel)  : panel_page(panel,PAGE_MOSAIC_INFO, "Mosaic Info")
{
    motifTable = new AQTableWidget(this);
    motifTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    motifTable->setColumnCount(4);
    motifTable->setColumnWidth(0,40);

    QStringList qslH;
    qslH << "Style" << "Prototype" <<"Tiling" << "Motif";
    motifTable->setHorizontalHeaderLabels(qslH);
    motifTable->verticalHeader()->setVisible(false);

    vbox->addWidget(motifTable);

    vbox->addStretch();

    connect(tilingMaker,   &TilingMaker::sig_tilingLoaded,      this,   &page_mosaic_info::onEnter);
    connect(mosaicMaker,   &MosaicMaker::sig_mosaicLoaded,      this,   &page_mosaic_info::onEnter);
}

void  page_mosaic_info::onEnter()
{
    if (!panel->isVisiblePage(this))
        return;

    motifTable->clearContents();

    showMotifsFromStyles();

    motifTable->resizeColumnsToContents();
    motifTable->adjustTableSize();
    updateGeometry();
}

void  page_mosaic_info::onRefresh()
{
    showMotifsFromStyles();
}

void page_mosaic_info::showMotifsFromStyles()
{
    motifTable->clearContents();

    int row = 0;
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        const StyleSet sset = mosaic->getStyleSet();
        for (auto & style : std::as_const(sset))
        {

            ProtoPtr pp = style->getPrototype();
            if (pp)
            {
                motifTable->setRowCount(row+1);
                QString stylename = style->getStyleDesc() + " " + addr(style.get());
                QTableWidgetItem * item =  new QTableWidgetItem(stylename);
                motifTable->setItem(row,COL_STYLE_NAME,item);

                QString pinfo = addr(pp.get());
                item =  new QTableWidgetItem(pinfo);
                motifTable->setItem(row,COL_PROTO,item);

                TilingPtr tiling = pp->getTiling();
                if (tiling)
                {
                    QString tileName = tiling->getVName().get() + "  " + addr(tiling.get());
                    item =  new QTableWidgetItem(tileName);
                    motifTable->setItem(row,COL_TILING_NAME,item);
                }

                QVector<DesignElementPtr>  dels = pp->getDesignElements();
                for (auto & del : dels)
                {
                    MotifPtr motif = del->getMotif();
                    if (motif)
                    {
                        QString figName = motif->getMotifDesc() + "  " + addr(motif.get());
                        item =  new QTableWidgetItem(figName);
                        motifTable->setItem(row,COL_TILE_TYPE,item);
                    }

                    row++;
                    motifTable->setRowCount(row+1);
                }
            }
        }
    }
}
