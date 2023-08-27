#include <QCheckBox>
#include <QHeaderView>
#include <QPushButton>
#include <QHBoxLayout>

#include "makers/prototype_maker/prototype.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "mosaic/design_element.h"
#include "motifs/motif.h"
#include "panels/page_prototype_info.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"
#include "tile/tile.h"
#include "tile/tiling.h"
#include "viewers/viewcontrol.h"
#include "widgets/layout_sliderset.h"
#include "panels/panel_misc.h"

using std::string;

typedef std::weak_ptr<class DesignElement>      WeakDesignElementPtr;
Q_DECLARE_METATYPE(WeakDesignElementPtr);

page_prototype_info:: page_prototype_info(ControlPanel * cpanel)  : panel_page(cpanel,"Prototype Info")
{
    protoView       = PrototypeView::getInstance();
    protoMaker      = PrototypeMaker::getInstance();
    protoMakerData  = protoMaker->getProtoMakerData();

    //setMouseTracking(true);

    SpinSet * widthSpin = new SpinSet("Line Width",3,1,9);
    widthSpin->setValue((int)config->protoviewWidth);

    QCheckBox * cbDrawMap        = new QCheckBox("Prototype Map");
    QCheckBox * cbDrawTiles      = new QCheckBox("All Tiles");
    QCheckBox * cbDrawMotifs     = new QCheckBox("All Motifs");
    QCheckBox * cbDrawProto      = new QCheckBox("Prototype");
    QCheckBox * cbHiliteTiles    = new QCheckBox("DEL Tiles");
    QCheckBox * cbHiliteMotifs   = new QCheckBox("DEL Motifs");

    QCheckBox * cbAllVisible        = new QCheckBox("All");
    QCheckBox * cbVisibleMotifs     = new QCheckBox("Motifs");
    QCheckBox * cbVisibleTiles      = new QCheckBox("Tiles");

    QPushButton * hideBtn = new QPushButton("Hide All");
    QPushButton * showBtn = new QPushButton("Show All");

    int mode = config->protoViewMode;

    if (mode & PROTO_DRAW_MAP)
        cbDrawMap->setChecked(true);
    if (mode & PROTO_ALL_TILES)
        cbDrawTiles->setChecked(true);
    if (mode & PROTO_ALL_MOTIFS)
        cbDrawMotifs->setChecked(true);
    if (mode & PROTO_DEL_TILES)
        cbHiliteTiles->setChecked(true);
    if (mode & PROTO_DEL_MOTIFS)
        cbHiliteMotifs->setChecked(true);
    if (mode & PROTO_DRAW_PROTO)
        cbDrawProto->setChecked(true);

    if (mode & PROTO_ALL_VISIBLE)
        cbAllVisible->setChecked(true);
    if (mode & PROTO_VISIBLE_TILE)
        cbVisibleTiles->setChecked(true);
    if (mode & PROTO_VISIBLE_MOTIF)
        cbVisibleMotifs->setChecked(true);

    showSettings = new QGridLayout;
    int row = 0;
    int col = 0;
    showSettings->addWidget(cbDrawMap,row,col++);
    showSettings->addWidget(cbDrawTiles,row,col++);
    showSettings->addWidget(cbDrawMotifs,row,col++);
    showSettings->addWidget(cbDrawProto,row,col++);
    showSettings->addWidget(cbHiliteTiles,row,col++);
    showSettings->addWidget(cbHiliteMotifs,row,col++);

    buildColorGrid();

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(hideBtn);
    hbox->addSpacing(3);
    hbox->addWidget(showBtn);
    hbox->addSpacing(3);
    hbox->addWidget(cbVisibleMotifs);
    hbox->addWidget(cbVisibleTiles);
    hbox->addWidget(cbAllVisible);
    hbox->addStretch();

    protoTable = new AQTableWidget(this);
    protoTable->setColumnCount(3);
    QStringList qslH2;
    qslH2 << "Prototype" << "Tiling" << "Show Prototype";
    protoTable->setHorizontalHeaderLabels(qslH2);
    protoTable->verticalHeader()->setVisible(false);
    protoTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    protoTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    protoTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    protoTable->setSelectionMode(QAbstractItemView::SingleSelection);

    DELTable = new AQTableWidget(this);
    DELTable->setColumnCount(6);
    QStringList qslH;
    qslH << "Prototype" << "Tiling" << "Design Element" << "Tile" << "Motif" << "Show Motif";
    DELTable->setHorizontalHeaderLabels(qslH);
    DELTable->verticalHeader()->setVisible(false);
    DELTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    DELTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    DELTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    DELTable->setSelectionMode(QAbstractItemView::SingleSelection);

    vbox->addLayout(widthSpin);
    vbox->addLayout(showSettings);
    vbox->addSpacing(7);
    vbox->addWidget(protoTable);
    vbox->addSpacing(17);
    vbox->addLayout(hbox);
    vbox->addSpacing(5);
    vbox->addWidget(DELTable);
    vbox->addStretch();

    connect(protoTable,    &AQTableWidget::cellClicked, this, &page_prototype_info::slot_protoSelected);
    connect(DELTable,      &AQTableWidget::cellClicked, this, &page_prototype_info::slot_DELSelected);

    connect(widthSpin,        &SpinSet::valueChanged,   this, &page_prototype_info::slot_widthChanged);
    connect(cbDrawProto,      &QCheckBox::clicked,      this, &page_prototype_info::drawProtoClicked);
    connect(cbDrawMap,        &QCheckBox::clicked,      this, &page_prototype_info::drawMapClicked);
    connect(cbDrawTiles,      &QCheckBox::clicked,      this, &page_prototype_info::drawTileClicked);
    connect(cbDrawMotifs,     &QCheckBox::clicked,      this, &page_prototype_info::drawMotifClicked);
    connect(cbHiliteTiles,    &QCheckBox::clicked,      this, &page_prototype_info::hiliteTileClicked);
    connect(cbHiliteMotifs,   &QCheckBox::clicked,      this, &page_prototype_info::hiliteMotifClicked);

    connect(cbVisibleTiles,   &QCheckBox::clicked,      this, &page_prototype_info::visibleTilesClicked);
    connect(cbVisibleMotifs,  &QCheckBox::clicked,      this, &page_prototype_info::visibleMotifsClicked);
    connect(cbAllVisible,     &QCheckBox::clicked,      this, &page_prototype_info::allVisibleClicked);
    connect(hideBtn,          &QPushButton::clicked,    this, &page_prototype_info::slot_hide);
    connect(showBtn,          &QPushButton::clicked,    this, &page_prototype_info::slot_show);
}

void page_prototype_info::buildColorGrid()
{
    int row = 1;
    int col = 0;
    ProtoViewColors & colors = protoView->getColors();

    ClickableLabel * label = new ClickableLabel();
    QVariant variant = colors.mapColor;
    QString colcode  = variant.toString();
    label->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    showSettings->addWidget(label,row,col++);
    connect(label,&ClickableLabel::clicked, this, [this, &colors] { pickColor(colors.mapColor); });

    label = new ClickableLabel();
    variant = colors.tileColor;
    colcode  = variant.toString();
    label->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    showSettings->addWidget(label,row,col++);
    connect(label,&ClickableLabel::clicked, this, [this, &colors] { pickColor(colors.tileColor); });

    label = new ClickableLabel();
    variant = colors.motifColor;
    colcode  = variant.toString();
    label->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    showSettings->addWidget(label,row,col++);
    connect(label,&ClickableLabel::clicked, this, [this, &colors] { pickColor(colors.motifColor); });

    label = new ClickableLabel();
    variant = colors.tileBrushColor;
    colcode  = variant.toString();
    label->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    showSettings->addWidget(label,row,col++);
    connect(label,&ClickableLabel::clicked, this, [this, &colors] { pickColor(colors.tileBrushColor); });

    label = new ClickableLabel();
    variant = colors.delTileColor;
    colcode  = variant.toString();
    label->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    showSettings->addWidget(label,row,col++);
    connect(label,&ClickableLabel::clicked, this, [this, &colors] { pickColor(colors.delTileColor); });

    label = new ClickableLabel();
    variant = colors.delMotifColor;
    colcode  = variant.toString();
    label->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    showSettings->addWidget(label,row,col++);
    connect(label,&ClickableLabel::clicked, this, [this, &colors] { pickColor(colors.delMotifColor); });

    QPushButton * defaultButton = new QPushButton("Default Colors");
    showSettings->addWidget(defaultButton,row,col++);
    connect(defaultButton,    &QPushButton::clicked,    this, &page_prototype_info::setDefaultColors);
}

void  page_prototype_info::onRefresh()
{
//    auto  protoMakerData =  protoMaker->getProtoMakerData();

    for (int row = 0; row < protoTable->rowCount(); row++)
    {
        auto item = protoTable->item(row,PROTO_COL_PROTO);
        auto v    = item->data(Qt::UserRole);
        WeakProtoPtr wproto = v.value<WeakProtoPtr>();
        auto proto = wproto.lock();
        QString txt = "hidden";
        if (proto && !protoMakerData->isHidden(MVD_PROTO,proto))
        {
            txt = "visible";
        }
        item = protoTable->item(row,PROTO_COL_SHOW);
        item->setText(txt);
    }

    for (int row = 0; row < DELTable->rowCount(); row++)
    {
        auto item = DELTable->item(row,DEL_COL_DEL);
        auto v    = item->data(Qt::UserRole);
        WeakDesignElementPtr wdel = v.value<WeakDesignElementPtr>();
        auto del = wdel.lock();
        QString txt = "hidden";
        if (del && !protoMakerData->isHidden(MVD_PROTO,del))
        {
            txt = "visible";
        }
        item = DELTable->item(row,DEL_COL_SHOW_MOTIF);
        item->setText(txt);
    }
}

void page_prototype_info::onEnter()
{
    static QString msg("<body>"
                       "<span>Click on column to select tile  &nbsp;&nbsp; | &nbsp;&nbsp; Click on color to change</span>"
                       "</body>");

    panel->pushPanelStatus(msg);
    populateTables();
}

void page_prototype_info::onExit()
{
    panel->popPanelStatus();
}

void page_prototype_info::populateTables()
{
    setupProtoTable();
    setupDelTable();
}

void page_prototype_info::setupProtoTable()
{
    protoTable->clearContents();

    auto selected_proto = protoMakerData->getSelectedPrototype();

    const QVector<ProtoPtr> & prototypes = protoMakerData->getPrototypes();
    int row = 0;
    QTableWidgetItem * item;
    for (const auto & proto : qAsConst(prototypes))
    {
        protoTable->setRowCount(row + 1);

        item = new QTableWidgetItem(addr(proto.get()));
        item->setData(Qt::UserRole,QVariant::fromValue(WeakProtoPtr(proto)));
        protoTable->setItem(row,PROTO_COL_PROTO,item);

        item = new QTableWidgetItem(proto->getTiling()->getName());
        protoTable->setItem(row,PROTO_COL_TILING,item);

        item = new QTableWidgetItem(protoMakerData->isHidden(MVD_PROTO,proto) ? "hidden" : "visible");
        protoTable->setItem(row, PROTO_COL_SHOW,item);

        row++;
    }

    protoTable->resizeColumnsToContents();
    protoTable->adjustTableSize(880);
    updateGeometry();
}

void page_prototype_info::setupDelTable()
{
    DELTable->clearContents();

    const QVector<ProtoPtr> & prototypes = protoMakerData->getPrototypes();
    int row = 0;
    QTableWidgetItem * item;
    for (const auto & proto : qAsConst(prototypes))
    {
        const QVector<DesignElementPtr> & dels = proto->getDesignElements();
        for (const auto & del :  qAsConst(dels))
        {
            DELTable->setRowCount(row + 1);

            item = new QTableWidgetItem(addr(proto.get()));
            item->setData(Qt::UserRole,QVariant::fromValue(WeakProtoPtr(proto)));
            DELTable->setItem(row,DEL_COL_PROTO,item);

            item = new QTableWidgetItem(proto->getTiling()->getName());
            DELTable->setItem(row,DEL_COL_TILING,item);

            item = new QTableWidgetItem(addr(del.get()));
            item->setData(Qt::UserRole,QVariant::fromValue(WeakDesignElementPtr(del)));
            DELTable->setItem(row,DEL_COL_DEL,item);

            TilePtr fp = del->getTile();
            Q_ASSERT(fp);
            QString astring = addr(fp.get());
            if (fp->isRegular())
            {
                astring += " sides=" + QString::number(fp->numPoints());
            }
            item = new QTableWidgetItem(astring);
            DELTable->setItem(row,DEL_COL_TILE,item);

            MotifPtr motif = del->getMotif();
            Q_ASSERT(motif);
            astring = addr(motif.get()) + "  " + motif->getMotifDesc();
            item = new QTableWidgetItem(astring);
            DELTable->setItem(row,DEL_COL_MOTIF,item);

            item = new QTableWidgetItem(protoMakerData->isHidden(MVD_PROTO,del) ? "hidden" : "visible");
            DELTable->setItem(row, DEL_COL_SHOW_MOTIF,item);

            row++;
        }
    }

    DELTable->resizeColumnsToContents();
    DELTable->adjustTableSize(880);
    DELTable->adjustTableSize(880);  //second time works a charm
    updateGeometry();
}

void page_prototype_info::slot_hide()
{
    DELTable->clearSelection();

    DesignElementPtr nullDELL;
    protoMakerData->select(MVD_PROTO,nullDELL,false);

    view->update();
}

void page_prototype_info::slot_show()
{
    for (int row = 0; row < DELTable->rowCount(); row++)
    {
        auto item = DELTable->item(row,DEL_COL_DEL);
        auto v    = item->data(Qt::UserRole);
        WeakDesignElementPtr wdel = v.value<WeakDesignElementPtr>();
        auto del = wdel.lock();

        protoMakerData->select(MVD_PROTO,del,true);
    }
}

void page_prototype_info::slot_protoSelected(int row, int col)
{
    Q_UNUSED(col);
    qDebug() << "page_prototype_info::slot_protoSelected";

    auto item = protoTable->item(row,PROTO_COL_PROTO);
    auto v    = item->data(Qt::UserRole);
    WeakProtoPtr wproto = v.value<WeakProtoPtr>();
    auto proto = wproto.lock();

    protoMakerData->select(MVD_PROTO,proto,false,!protoMakerData->isHidden(MVD_PROTO,proto));

    emit sig_refreshView();
}

void page_prototype_info::slot_DELSelected(int row, int col)
{
    Q_UNUSED(col);
    qDebug() << "page_prototype_info::slot_DELSelected";
    
    auto item = DELTable->item(row,DEL_COL_DEL);
    auto v    = item->data(Qt::UserRole);
    WeakDesignElementPtr wdel= v.value<WeakDesignElementPtr>();
    auto del = wdel.lock();

    protoMakerData->select(MVD_PROTO,del,true, !protoMakerData->isHidden(MVD_PROTO,del));

    emit sig_refreshView();
}

void page_prototype_info::slot_widthChanged(int val)
{
    config->protoviewWidth = qreal(val);
    view->update();
}

void  page_prototype_info::drawProtoClicked(bool enb)
{
    setProtoViewMode(PROTO_DRAW_PROTO,enb);
}

void  page_prototype_info::drawMapClicked(bool enb)
{
    setProtoViewMode(PROTO_DRAW_MAP,enb);
}

void page_prototype_info::drawMotifClicked(bool enb)
{
    setProtoViewMode(PROTO_ALL_MOTIFS,enb);
}

void page_prototype_info::drawTileClicked(bool enb)
{
    setProtoViewMode(PROTO_ALL_TILES,enb);
}

void page_prototype_info::hiliteMotifClicked(bool enb)
{
    setProtoViewMode(PROTO_DEL_MOTIFS,enb);
}

void page_prototype_info::hiliteTileClicked(bool enb)
{
    setProtoViewMode(PROTO_DEL_TILES,enb);
}

void page_prototype_info::visibleTilesClicked(bool enb)
{
    setProtoViewMode(PROTO_VISIBLE_TILE,enb);
}

void page_prototype_info::visibleMotifsClicked(bool enb)
{
    setProtoViewMode(PROTO_VISIBLE_MOTIF,enb);
}

void page_prototype_info::allVisibleClicked(bool enb)
{
    setProtoViewMode(PROTO_ALL_VISIBLE,enb);
}

void  page_prototype_info::setProtoViewMode(eProtoViewMode mode, bool enb)
{
    int pvm = config->protoViewMode;
    if (enb)
        pvm |= (int)mode;
    else
        pvm &= ~mode;
    config->protoViewMode = pvm;

    //emit sig_refreshView();
    view->update();
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
        
        config->protoViewColors = protoView->getColors().getColors();
        buildColorGrid();
        emit sig_refreshView();
    }
}

void page_prototype_info::setDefaultColors()
{
    ProtoViewColors colors;
    colors.mapColor          = QColor(20,150,210);
    colors.motifColor        = QColor(0, 108, 0);
    colors.tileColor         = QColor(214,0,0);
    colors.delMotifColor     = QColor(Qt::blue);
    colors.delTileColor      = QColor(Qt::yellow);
    colors.tileBrushColor    = QColor(255, 217, 217,128);
    ProtoViewColors & viewColors = protoView->getColors();
    viewColors = colors;
    
    config->protoViewColors = protoView->getColors().getColors();
    buildColorGrid();
    emit sig_refreshView();
}


