#include <QCheckBox>
#include <QHeaderView>
#include <QPushButton>
#include <QHBoxLayout>
#include <QRadioButton>

#include "gui/panels/page_prototype_info.h"
#include "gui/widgets/panel_misc.h"
#include "gui/top/controlpanel.h"
#include "gui/top/view_controller.h"
#include "gui/widgets/layout_sliderset.h"
#include "gui/viewers/prototype_view.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/motif_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/motifs/motif.h"
#include "model/prototypes/tile_motif.h"
#include "model/prototypes/prototype.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"

typedef std::weak_ptr<class TileMotif> WeakTileMotifPtr;
Q_DECLARE_METATYPE(WeakTileMotifPtr);

page_prototype_info:: page_prototype_info(ControlPanel * cpanel)  : panel_page(cpanel,PAGE_PROTO_INFO,"Prototype Info")
{
    motifMaker = Sys::motifMaker;

    // top row
    SpinSet * widthSpin = new SpinSet("Line Width",3,1,9);
    widthSpin->setValue((int)config->protoviewWidth);

    QPushButton * pbRender = new QPushButton("Render Prototypes");
    pbRender->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QHBoxLayout * toprow   = new QHBoxLayout;
    toprow->addLayout(widthSpin);
    toprow->addStretch();
    toprow->addWidget(pbRender);

    auto box1 = buildPrototypeLayout();
    auto box2 = buildTilingUnitLayout();
    refreshSwatches();

    connect(widthSpin,        &SpinSet::valueChanged,   this, &page_prototype_info::slot_widthChanged);
    connect(pbRender,         &QPushButton::clicked,    this, [] { Sys::render(RENDER_RESET_PROTOTYPES);} );

    connect(tilingMaker,   &TilingMaker::sig_tilingLoaded,  this,   &page_prototype_info::populateTables);
    connect(mosaicMaker,   &MosaicMaker::sig_mosaicLoaded,  this,   &page_prototype_info::populateTables);
    connect(viewControl,   &ViewController::sig_unloaded,   this,   &page_prototype_info::populateTables);

    vbox->addSpacing(5);
    vbox->addLayout(toprow);
    vbox->addSpacing(9);
    vbox->addWidget(box1);
    vbox->addSpacing(9);
    vbox->addWidget(box2);
    vbox->addStretch();
}

QGroupBox * page_prototype_info::buildPrototypeLayout()
{
    QCheckBox * cbShowMap        = new QCheckBox("Show Map");
    QCheckBox * cbShowTiles      = new QCheckBox("Show Tiles");
    QCheckBox * cbShowMotifs     = new QCheckBox("Show Motifs");
    QCheckBox * cbShowExtBoundary= new QCheckBox("Show EB");
    QCheckBox * cbShowTilingUnit = new QCheckBox("Show TU");
    QCheckBox * cbHiliteTiles    = new QCheckBox("Show TU Tiles");
    QCheckBox * cbHiliteMotifs   = new QCheckBox("Show TU Motifs");

    int mode = config->protoViewMode;

    if (mode & SHOW_MAP)
        cbShowMap->setChecked(true);
    if (mode & SHOW_TILES)
        cbShowTiles->setChecked(true);
    if (mode & SHOW_MOTIFS)
        cbShowMotifs->setChecked(true);
    if (mode & SHOW_EXT_BOUNDARY)
        cbShowExtBoundary->setChecked(true);
    if (mode & SHOW_TILING_UNIT)
        cbShowTilingUnit->setChecked(true);
    if (mode & SHOW_TU_TILES)
        cbHiliteTiles->setChecked(true);
    if (mode & SHOW_TU_MOTIFS)
        cbHiliteMotifs->setChecked(true);

    showSettings = new QGridLayout;
    int row = 0;
    int col = 0;
    showSettings->addWidget(cbShowMap,row,col++);
    showSettings->addWidget(cbShowTiles,row,col++);
    showSettings->addWidget(cbShowMotifs,row,col++);
    showSettings->addWidget(cbShowExtBoundary,row,col++);
    showSettings->addWidget(cbShowTilingUnit,row,col++);
    showSettings->addWidget(cbHiliteTiles,row,col++);
    showSettings->addWidget(cbHiliteMotifs,row,col++);

    buildColorGrid();

    protoTable = new AQTableWidget(this);
    protoTable->setColumnCount(3);
    QStringList qslH2;
    qslH2 << "Prototype" << "Tiling" << "Show" ;
    protoTable->setHorizontalHeaderLabels(qslH2);
    protoTable->verticalHeader()->setVisible(false);
    protoTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    protoTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    protoTable->setSelectionMode(QAbstractItemView::NoSelection);

    QPushButton * defaultButton = new QPushButton("Default Colors");

    QHBoxLayout * tableRow = new QHBoxLayout;
    tableRow->addWidget(protoTable);
    tableRow->addStretch();
    tableRow->addWidget(defaultButton,0);

    showSettings->setRowMinimumHeight(1,defaultButton->sizeHint().height());

    connect(cbShowTilingUnit, &QCheckBox::clicked,      this, &page_prototype_info::drawProtoClicked);
    connect(cbShowMap,        &QCheckBox::clicked,      this, &page_prototype_info::drawMapClicked);
    connect(cbShowTiles,      &QCheckBox::clicked,      this, &page_prototype_info::drawTileClicked);
    connect(cbShowMotifs,     &QCheckBox::clicked,      this, &page_prototype_info::drawMotifClicked);
    connect(cbShowExtBoundary,&QCheckBox::clicked,      this, &page_prototype_info::drawExtBoundaryClicked);
    connect(cbHiliteTiles,    &QCheckBox::clicked,      this, &page_prototype_info::hiliteTileClicked);
    connect(cbHiliteMotifs,   &QCheckBox::clicked,      this, &page_prototype_info::hiliteMotifClicked);
    connect(defaultButton,    &QPushButton::clicked,    this, &page_prototype_info::setDefaultColors);

    QVBoxLayout * vb = new QVBoxLayout;
    vb->addLayout(tableRow);
    vb->addLayout(showSettings);

    QGroupBox * gb = new QGroupBox("Prototype and Tiling Unit (TU)");
    gb->setLayout(vb);

    return gb;
}

QGroupBox * page_prototype_info::buildTilingUnitLayout()
{
    QCheckBox * cbVisibleTiles  = new QCheckBox("Show Tiles");
    QCheckBox * cbVisibleMotifs = new QCheckBox("Show Motifs");
    QRadioButton *  rSingle     = new QRadioButton("Single");
    QRadioButton *  rPlaced     = new QRadioButton("Placed");
    QHBoxLayout *  hbox         = new QHBoxLayout;
    hbox->addWidget(rSingle);
    hbox->addWidget(rPlaced);

    int mode = config->protoViewMode;

    if (mode & SHOW_ALL_TU_TILES)
        rPlaced->setChecked(true);
    else
        rSingle->setChecked(true);
    if (mode & SHOW_SELECTED_TU_TILES)
        cbVisibleTiles->setChecked(true);
    if (mode & SHOW_SELECTED_TU_MOTIFS)
        cbVisibleMotifs->setChecked(true);

    showSettings2 = new QGridLayout;
    showSettings2->addLayout(hbox,0,0);
    showSettings2->addWidget(cbVisibleTiles,0,1);
    showSettings2->addWidget(cbVisibleMotifs,0,2);
    showSettings2->addWidget(new QLabel(),0,3);
    showSettings2->addWidget(new QLabel(),0,4);
    showSettings2->addWidget(new QLabel(),0,5);

    buildColorGrid2();

    tileMotifTable = new AQTableWidget(this);
    tileMotifTable->setColumnCount(6);
    QStringList qslH;
    qslH << "Prototype" << "Tiling" << "Tile Motif" << "Tile" << "Motif" << "Show Motif";
    tileMotifTable->setHorizontalHeaderLabels(qslH);
    tileMotifTable->verticalHeader()->setVisible(false);
    tileMotifTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    tileMotifTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tileMotifTable->setSelectionMode(QAbstractItemView::NoSelection);

    connect(cbVisibleTiles,   &QCheckBox::clicked,      this, &page_prototype_info::visibleTilesClicked);
    connect(cbVisibleMotifs,  &QCheckBox::clicked,      this, &page_prototype_info::visibleMotifsClicked);
    connect(rSingle,          &QRadioButton::clicked,   this, &page_prototype_info::singleClicked);
    connect(rPlaced,          &QRadioButton::clicked,   this, &page_prototype_info::placedClicked);

    QVBoxLayout * vb = new QVBoxLayout;
    vb->addLayout(showSettings2);
    vb->addWidget(tileMotifTable);

    QGroupBox * gb = new QGroupBox("Tiling Unit");
    gb->setLayout(vb);

    return gb;
}

void page_prototype_info::buildColorGrid()
{
    int row = 1;
    int col = 0;
    ProtoViewColors & colors = Sys::prototypeView->getColors();

    addSwatch(showSettings,row,col++,colors.mapColor);
    addSwatch(showSettings,row,col++,colors.tileColor);
    addSwatch(showSettings,row,col++,colors.motifColor);
    addSwatch(showSettings,row,col++,colors.extBoundaryColor);
    addSwatch(showSettings,row,col++,colors.tileBrushColor);
    addSwatch(showSettings,row,col++,colors.delTileColor);
    addSwatch(showSettings,row,col++,colors.delMotifColor);
}

void page_prototype_info::buildColorGrid2()
{
    int row = 1;
    int col = 1;
    ProtoViewColors & colors = Sys::prototypeView->getColors();

    addSwatch(showSettings2,row,col++,colors.visibleTileColor);
    addSwatch(showSettings2,row,col++,colors.visibleMotifColor);

    QPushButton * defaultButton = new QPushButton("Default Colors");
    showSettings2->addWidget(defaultButton,row,col++);
    connect(defaultButton,    &QPushButton::clicked,    this, &page_prototype_info::setDefaultTUColors);
}

void page_prototype_info::addSwatch(QGridLayout * grid, int row, int col, QColor & color)
{
    ClickableLabel * label = new ClickableLabel();
    grid->addWidget(label,row,col);
    connect(label,&ClickableLabel::clicked, this, [this, &color] { pickColor(color); });
    swatches.push_back({label,&color});
}

void page_prototype_info::refreshSwatches()
{
    for (const auto & swatch : std::as_const(swatches))
    {
        QString colcode = swatch.second->name(QColor::HexArgb);
        swatch.first->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    }
}

void  page_prototype_info::onRefresh()
{
    for (int row = 0; row < protoTable->rowCount() ; row++)
    {
        QTableWidgetItem * item = protoTable->item(row,(int)ProtoCol::Proto);
        if (!item) continue;

        QVariant v          = item->data(Qt::UserRole);
        WeakProtoPtr wproto = v.value<WeakProtoPtr>();
        ProtoPtr proto      = wproto.lock();
        if(!proto)
        {
            continue;
        }

        bool visible = motifMaker->protoData.isVisible(proto);

        QCheckBox * widget = dynamic_cast<QCheckBox*>(protoTable->cellWidget(row,(int)ProtoCol::Show));
        if (!widget) continue;
        widget->setChecked(visible);
    }

    for (int row = 0; row < tileMotifTable->rowCount(); row++)
    {
        QTableWidgetItem * item = tileMotifTable->item(row,(int)MotifCol::TileMotif);
        if (!item) continue;

        QVariant v             = item->data(Qt::UserRole);
        WeakTileMotifPtr wtimo = v.value<WeakTileMotifPtr>();
        TileMotifPtr timo      = wtimo.lock();
        if (!timo) continue;

        bool visible = motifMaker->protoData.isVisible(timo);

        QCheckBox* widget = dynamic_cast<QCheckBox*>(tileMotifTable->cellWidget(row,(int)MotifCol::TileMotif));
        if (!widget) return;
        widget->setChecked(visible);
    }
}

void page_prototype_info::onEnter()
{
    setPageStatus();

    populateTables();
}

void page_prototype_info::onExit()
{
    clearPageStatus();
}

void page_prototype_info::populateTables()
{
    populateProtoTable();
    populateTileMotifTable();
}

void page_prototype_info::populateProtoTable()
{
    protoTable->clearContents();

    const QVector<ProtoPtr> & prototypes = motifMaker->protoData.getPrototypes();
    int row = 0;
    QTableWidgetItem * item;
    for (const auto & proto : std::as_const(prototypes))
    {
        protoTable->setRowCount(row + 1);

        item = new QTableWidgetItem(addr(proto.get()));
        item->setData(Qt::UserRole,QVariant::fromValue(WeakProtoPtr(proto)));
        protoTable->setItem(row,(int)ProtoCol::Proto,item);

        TilingPtr tiling = proto->getTiling();
        item = new QTableWidgetItem(tiling ? tiling->getVName().get() : "(none - explicit map)");
        protoTable->setItem(row,(int)ProtoCol::Tiling,item);

        QCheckBox * cb = new QCheckBox("Visible ");
        cb->setStyleSheet("padding-left : 8px");
        cb->setChecked(motifMaker->protoData.isVisible(proto));
        protoTable->setCellWidget(row,(int)ProtoCol::Show,cb);
        connect(cb, &QCheckBox::clicked, this, [this, row](bool checked){ protoChecked(row, checked); });

        row++;
    }

    protoTable->resizeColumnsToContents();
    protoTable->adjustTableSize(880);
    updateGeometry();
}

void page_prototype_info::populateTileMotifTable()
{
    tileMotifTable->clearContents();

    const QVector<ProtoPtr> & prototypes = motifMaker->protoData.getPrototypes();
    int row = 0;
    QTableWidgetItem * item;
    for (const auto & proto : std::as_const(prototypes))
    {
        const TileMotifSet & timos = proto->getTileMotifs();
        for (const auto & timo :  std::as_const(timos))
        {
            tileMotifTable->setRowCount(row + 1);

            item = new QTableWidgetItem(addr(proto.get()));
            item->setData(Qt::UserRole,QVariant::fromValue(WeakProtoPtr(proto)));
            tileMotifTable->setItem(row,(int)MotifCol::Proto,item);

            item = new QTableWidgetItem(proto->getTiling()->getVName().get());
            tileMotifTable->setItem(row,(int)MotifCol::Tiling,item);

            item = new QTableWidgetItem(addr(timo.get()));
            item->setData(Qt::UserRole,QVariant::fromValue(WeakTileMotifPtr(timo)));
            tileMotifTable->setItem(row,(int)MotifCol::TileMotif,item);

            TilePtr fp = timo->getTile();
            Q_ASSERT(fp);
            QString astring = addr(fp.get());
            if (fp->isRegular())
            {
                astring += " sides=" + QString::number(fp->numPoints());
            }
            item = new QTableWidgetItem(astring);
            tileMotifTable->setItem(row,(int)MotifCol::Tile,item);

            MotifPtr motif = timo->getMotif();
            Q_ASSERT(motif);
            astring = addr(motif.get()) + "  " + motif->getMotifDesc();
            item = new QTableWidgetItem(astring);
            tileMotifTable->setItem(row,(int)MotifCol::Motif,item);

            QCheckBox * cb = new QCheckBox("Visible");
            cb->setStyleSheet("padding-left : 8px");
            cb->setChecked(motifMaker->protoData.isVisible(timo));
            tileMotifTable->setCellWidget(row,(int)MotifCol::Show,cb);
            connect(cb, &QCheckBox::clicked, this, [this, row](bool checked){ tileMotifChecked(row, checked); });

            row++;
        }
    }

    tileMotifTable->resizeColumnsToContents();
    tileMotifTable->adjustTableSize(880);
    tileMotifTable->adjustTableSize(880);  //second time works a charm
    updateGeometry();
}

void page_prototype_info::protoChecked(int row, bool checked)
{
    qDebug() << "page_prototype_info::protoChecked";

    auto item = protoTable->item(row,(int)ProtoCol::Proto);
    auto v    = item->data(Qt::UserRole);
    WeakProtoPtr wproto = v.value<WeakProtoPtr>();
    auto proto = wproto.lock();

    motifMaker->protoData.show(proto,checked);

    emit sig_reconstructView();
}

void page_prototype_info::tileMotifChecked(int row, bool checked)
{
    qDebug() << "page_prototype_info::tileMotifChecked";
    
    auto item = tileMotifTable->item(row,(int)MotifCol::TileMotif);
    auto v    = item->data(Qt::UserRole);
    WeakTileMotifPtr wtimo = v.value<WeakTileMotifPtr>();
    auto timo = wtimo .lock();

    motifMaker->protoData.show(timo,checked);

    emit sig_reconstructView();
}

void page_prototype_info::slot_widthChanged(int val)
{
    config->protoviewWidth = qreal(val);
    emit sig_updateView();
}

void  page_prototype_info::drawProtoClicked(bool enb)
{
    setProtoViewMode(SHOW_TILING_UNIT,enb);
}

void  page_prototype_info::drawMapClicked(bool enb)
{
    setProtoViewMode(SHOW_MAP,enb);
}

void page_prototype_info::drawMotifClicked(bool enb)
{
    setProtoViewMode(SHOW_MOTIFS,enb);
}

void page_prototype_info::drawExtBoundaryClicked(bool enb)
{
    setProtoViewMode(SHOW_EXT_BOUNDARY,enb);
}

void page_prototype_info::drawTileClicked(bool enb)
{
    setProtoViewMode(SHOW_TILES,enb);
}

void page_prototype_info::hiliteMotifClicked(bool enb)
{
    setProtoViewMode(SHOW_TU_MOTIFS,enb);
}

void page_prototype_info::hiliteTileClicked(bool enb)
{
    setProtoViewMode(SHOW_TU_TILES,enb);
}

void page_prototype_info::visibleTilesClicked(bool enb)
{
    setProtoViewMode(SHOW_SELECTED_TU_TILES,enb);
}

void page_prototype_info::visibleMotifsClicked(bool enb)
{
    setProtoViewMode(SHOW_SELECTED_TU_MOTIFS,enb);
}

void page_prototype_info::placedClicked(bool enb)
{
    setProtoViewMode(SHOW_ALL_TU_TILES,enb);
}

void page_prototype_info::singleClicked(bool enb)
{
    setProtoViewMode(SHOW_ALL_TU_TILES,!enb);
}

void  page_prototype_info::setProtoViewMode(eProtoViewMode mode, bool enb)
{
    qDebug() << "page_prototype_info::setProtoViewMode" << mode << enb;
    int pvm = config->protoViewMode;
    if (enb)
        pvm |= (int)mode;
    else
        pvm &= ~mode;
    config->protoViewMode = pvm;

    //emit sig_reconstructView();
    emit sig_updateView();
}

void page_prototype_info::pickColor(QColor & color)
{
    AQColorDialog dlg(color,this);
    dlg.setCurrentColor(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    QColor acolor = dlg.selectedColor();
    if (acolor.isValid())
    {
        color = acolor;
        
        config->protoViewColors = Sys::prototypeView->getColors().getColors();
        refreshSwatches();
        emit sig_reconstructView();
    }
}

void page_prototype_info::setDefaultColors()
{
    ProtoViewColors & colors = Sys::prototypeView->getColors();
    QStringList defaults = Configuration::defaultProtoViewColors();
    colors.setColors(defaults);

    config->protoViewColors = colors.getColors();
    refreshSwatches();
    emit sig_reconstructView();
}

void page_prototype_info::setDefaultTUColors()
{
    ProtoViewColors defaults;
    ProtoViewColors & colors = Sys::prototypeView->getColors();
    colors.visibleTileColor  = defaults.visibleTileColor;
    colors.visibleMotifColor = defaults.visibleMotifColor;

    refreshSwatches();
    emit sig_reconstructView();
}
