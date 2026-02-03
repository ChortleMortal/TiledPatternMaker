#include <QCheckBox>
#include <QHeaderView>
#include <QPushButton>
#include <QHBoxLayout>

#include "gui/panels/page_prototype_info.h"
#include "gui/panels/panel_misc.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view_controller.h"
#include "gui/widgets/layout_sliderset.h"
#include "gui/viewers/prototype_view.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/prototype_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/motifs/motif.h"
#include "model/prototypes/design_element.h"
#include "model/prototypes/prototype.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/transform.h"

typedef std::weak_ptr<class DesignElement> WeakDELPtr;
Q_DECLARE_METATYPE(WeakDELPtr);

page_prototype_info:: page_prototype_info(ControlPanel * cpanel)  : panel_page(cpanel,PAGE_PROTO_INFO,"Prototype Info")
{
    protoMaker = Sys::prototypeMaker;

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

    auto box3 = buildDistortionsLayout();

    connect(widthSpin,        &SpinSet::valueChanged,   this, &page_prototype_info::slot_widthChanged);
    connect(pbRender,         &QPushButton::clicked,    this, [] { Sys::render(RENDER_RESET_PROTOTYPES);} );

    connect(tilingMaker,   &TilingMaker::sig_tilingLoaded,      this,   &page_prototype_info::populateTables);
    connect(mosaicMaker,   &MosaicMaker::sig_mosaicLoaded,      this,   &page_prototype_info::populateTables);
    connect(viewControl,   &SystemViewController::sig_unloaded, this,   &page_prototype_info::populateTables);

    vbox->addSpacing(5);
    vbox->addLayout(toprow);
    vbox->addSpacing(9);
    vbox->addWidget(box1);
    vbox->addSpacing(9);
    vbox->addWidget(box3);
    vbox->addSpacing(9);
    vbox->addWidget(box2);
    vbox->addStretch();
}

QGroupBox * page_prototype_info::buildDistortionsLayout()
{
    chkDistort = new QCheckBox("Distort");

    xBox = new DoubleSpinSet("X:",1.0,-9.99,9.99);
    xBox->setSingleStep(0.01);
    yBox = new DoubleSpinSet("Y:",1.0,-9.99,9.99);
    yBox->setSingleStep(0.01);

    connect(xBox, &DoubleSpinSet::valueChanged, this, &page_prototype_info::setDistortion);
    connect(yBox, &DoubleSpinSet::valueChanged, this, &page_prototype_info::setDistortion);
    connect(chkDistort, &QCheckBox::clicked,    this, &page_prototype_info::enbDistortion);

    QHBoxLayout * dist = new QHBoxLayout;
    dist->addWidget(chkDistort);
    dist->addLayout(xBox);
    dist->addLayout(yBox);
    dist->addStretch(10);

    QGroupBox * gb = new QGroupBox("Distortions");
    gb->setLayout(dist);

    return gb;
}

QGroupBox * page_prototype_info::buildPrototypeLayout()
{
    QCheckBox * cbShowMap        = new QCheckBox("Show Map");
    QCheckBox * cbShowTiles      = new QCheckBox("Show Tiles");
    QCheckBox * cbShowMotifs     = new QCheckBox("Show Motifs");
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
    showSettings->addWidget(cbShowTilingUnit,row,col++);
    showSettings->addWidget(cbHiliteTiles,row,col++);
    showSettings->addWidget(cbHiliteMotifs,row,col++);

    buildColorGrid();

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

    connect(cbShowTilingUnit, &QCheckBox::clicked,      this, &page_prototype_info::drawProtoClicked);
    connect(cbShowMap,        &QCheckBox::clicked,      this, &page_prototype_info::drawMapClicked);
    connect(cbShowTiles,      &QCheckBox::clicked,      this, &page_prototype_info::drawTileClicked);
    connect(cbShowMotifs,     &QCheckBox::clicked,      this, &page_prototype_info::drawMotifClicked);
    connect(cbHiliteTiles,    &QCheckBox::clicked,      this, &page_prototype_info::hiliteTileClicked);
    connect(cbHiliteMotifs,   &QCheckBox::clicked,      this, &page_prototype_info::hiliteMotifClicked);

    QVBoxLayout * vb = new QVBoxLayout;
    vb->addWidget(protoTable);
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

    connect(DELTable,      &AQTableWidget::cellClicked, this, &page_prototype_info::DELChecked);
    connect(cbVisibleTiles,   &QCheckBox::clicked,      this, &page_prototype_info::visibleTilesClicked);
    connect(cbVisibleMotifs,  &QCheckBox::clicked,      this, &page_prototype_info::visibleMotifsClicked);
    connect(rSingle,          &QRadioButton::clicked,   this, &page_prototype_info::singleClicked);
    connect(rPlaced,          &QRadioButton::clicked,   this, &page_prototype_info::placedClicked);

    QVBoxLayout * vb = new QVBoxLayout;
    vb->addLayout(showSettings2);
    vb->addWidget(DELTable);

    QGroupBox * gb = new QGroupBox("Tiling Unit");
    gb->setLayout(vb);

    return gb;
}

void page_prototype_info::buildColorGrid()
{
    int row = 1;
    int col = 0;
    ProtoViewColors & colors = Sys::prototypeView->getColors();

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

void page_prototype_info::buildColorGrid2()
{
    int row = 1;
    int col = 1;
    ProtoViewColors & colors = Sys::prototypeView->getColors();

    ClickableLabel * label = new ClickableLabel();
    QVariant variant = colors.visibleTileColor;
    QString colcode  = variant.toString();
    label->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    showSettings2->addWidget(label,row,col++);
    connect(label,&ClickableLabel::clicked, this, [this, &colors] { pickColor(colors.visibleTileColor); });

    label = new ClickableLabel();
    variant = colors.visibleMotifColor;
    colcode  = variant.toString();
    label->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    showSettings2->addWidget(label,row,col++);
    connect(label,&ClickableLabel::clicked, this, [this, &colors] { pickColor(colors.visibleMotifColor); });

    QPushButton * defaultButton = new QPushButton("Default Colors");
    showSettings2->addWidget(defaultButton,row,col++);
    connect(defaultButton,    &QPushButton::clicked,    this, &page_prototype_info::setDefaultColors);
}

void  page_prototype_info::onRefresh()
{
    for (int row = 0; row < protoTable->rowCount(); row++)
    {
        QTableWidgetItem * item = protoTable->item(row,PROTO_COL_PROTO);
        if (!item) continue;

        QVariant v          = item->data(Qt::UserRole);
        WeakProtoPtr wproto = v.value<WeakProtoPtr>();
        auto proto          = wproto.lock();
        if(!proto) return;

        bool hidden = prototypeMaker->isHidden(MVD_PROTO,proto);

        QCheckBox * widget = dynamic_cast<QCheckBox*>(protoTable->cellWidget(row,PROTO_COL_SHOW));
        if (!widget) return;

        widget->setChecked(hidden);
    }

    for (int row = 0; row < DELTable->rowCount(); row++)
    {
        QTableWidgetItem * item = DELTable->item(row,DEL_COL_DEL);
        if (!item) continue;

        QVariant v      = item->data(Qt::UserRole);
        WeakDELPtr wdel = v.value<WeakDELPtr>();
        auto del = wdel.lock();
        if (!del) return;

        bool hidden = prototypeMaker->isHidden(MVD_PROTO,del);

        QCheckBox* widget = dynamic_cast<QCheckBox*>(DELTable->cellWidget(row,DEL_COL_SHOW_DEL));
        if (!widget) return;

        widget->setChecked(hidden);
    }

    auto proto  = protoMaker->getSelectedPrototype();
    if (proto)
    {
        chkDistort->setChecked(proto->getDistortionEnable());
        QTransform t = proto->getDistortion();
        xBox->setValue(Transform::scalex(t));
        yBox->setValue(Transform::scaley(t));
    }
    else
    {
        chkDistort->setChecked(false);
        xBox->setValue(0);
        yBox->setValue(0);
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
    populateDelTable();
}

void page_prototype_info::populateProtoTable()
{
    protoTable->clearContents();

    auto selected_proto = prototypeMaker->getSelectedPrototype();

    const QVector<ProtoPtr> & prototypes = prototypeMaker->getPrototypes();
    int row = 0;
    QTableWidgetItem * item;
    for (const auto & proto : std::as_const(prototypes))
    {
        protoTable->setRowCount(row + 1);

        item = new QTableWidgetItem(addr(proto.get()));
        item->setData(Qt::UserRole,QVariant::fromValue(WeakProtoPtr(proto)));
        protoTable->setItem(row,PROTO_COL_PROTO,item);

        item = new QTableWidgetItem(proto->getTiling()->getVName().get());
        protoTable->setItem(row,PROTO_COL_TILING,item);

        QCheckBox * cb = new QCheckBox("Hide");
        cb->setStyleSheet("padding-left : 8px");
        cb->setChecked(prototypeMaker->isHidden(MVD_PROTO,proto));
        protoTable->setCellWidget(row, PROTO_COL_SHOW,cb);
        connect(cb, &QCheckBox::clicked, this, [this, row](bool checked){ protoChecked(row, checked); });

        if (proto == selected_proto)
            protoTable->selectRow(row);

        row++;
    }

    protoTable->resizeColumnsToContents();
    protoTable->adjustTableSize(880);
    updateGeometry();
}

void page_prototype_info::populateDelTable()
{
    DELTable->clearContents();

    auto selected_del = prototypeMaker->getSelectedDEL();

    const QVector<ProtoPtr> & prototypes = prototypeMaker->getPrototypes();
    int row = 0;
    QTableWidgetItem * item;
    for (const auto & proto : std::as_const(prototypes))
    {
        const QVector<DELPtr> & dels = proto->getDesignElements();
        for (const auto & del :  std::as_const(dels))
        {
            DELTable->setRowCount(row + 1);

            item = new QTableWidgetItem(addr(proto.get()));
            item->setData(Qt::UserRole,QVariant::fromValue(WeakProtoPtr(proto)));
            DELTable->setItem(row,DEL_COL_PROTO,item);

            item = new QTableWidgetItem(proto->getTiling()->getVName().get());
            DELTable->setItem(row,DEL_COL_TILING,item);

            item = new QTableWidgetItem(addr(del.get()));
            item->setData(Qt::UserRole,QVariant::fromValue(WeakDELPtr(del)));
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

            QCheckBox * cb = new QCheckBox("Hide");
            cb->setStyleSheet("padding-left : 8px");
            cb->setChecked(prototypeMaker->isHidden(MVD_PROTO,del));
            DELTable->setCellWidget(row, DEL_COL_SHOW_DEL,cb);
            connect(cb, &QCheckBox::clicked, this, [this, row](bool checked){ DELChecked(row, checked); });

            if (del == selected_del)
                DELTable->selectRow(row);

            row++;
        }
    }

    DELTable->resizeColumnsToContents();
    DELTable->adjustTableSize(880);
    DELTable->adjustTableSize(880);  //second time works a charm
    updateGeometry();
}

void page_prototype_info::protoChecked(int row, bool checked)
{
    qDebug() << "page_prototype_info::protoChecked";

    auto item = protoTable->item(row,PROTO_COL_PROTO);
    auto v    = item->data(Qt::UserRole);
    WeakProtoPtr wproto = v.value<WeakProtoPtr>();
    auto proto = wproto.lock();

    prototypeMaker->select(MVD_PROTO,proto,false,checked);

    emit sig_reconstructView();
}

void page_prototype_info::DELChecked(int row, bool checked)
{
    qDebug() << "page_prototype_info::DELChecked";
    
    auto item = DELTable->item(row,DEL_COL_DEL);
    auto v    = item->data(Qt::UserRole);
    WeakDELPtr wdel= v.value<WeakDELPtr>();
    auto del = wdel.lock();

    prototypeMaker->select(MVD_PROTO,del,true,checked);

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
        buildColorGrid();
        emit sig_reconstructView();
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
    ProtoViewColors & viewColors = Sys::prototypeView->getColors();
    viewColors = colors;
    
    config->protoViewColors = Sys::prototypeView->getColors().getColors();
    buildColorGrid();
    emit sig_reconstructView();
}

void page_prototype_info::setDistortion()
{
    // distortion is used to force desings into a fixed space
    // by defornming regular polygons by transforming the map

    qreal x = xBox->value();
    qreal y = yBox->value();
    QTransform t = QTransform::fromScale(x,y);
    qDebug() << "scale x" << t.m11() << "y" << t.m22();

    auto proto = protoMaker->getSelectedPrototype();
    if (proto)
    {
        proto->setDistortion(t);

        if (proto->getDistortionEnable())
        {
            auto mosaic = mosaicMaker->getMosaic();
            mosaic->resetProtoMaps();   // really is wipeout
            mosaic->resetStyleMaps();
            emit sig_reconstructView();
        }
    }
}

void page_prototype_info::enbDistortion(bool checked)
{
    auto proto = protoMaker->getSelectedPrototype();
    if (proto)
    {
        proto->enableDistortion(checked);

        auto mosaic = mosaicMaker->getMosaic();
        mosaic->resetProtoMaps();   // really is wipeout
        mosaic->resetStyleMaps();
        emit sig_reconstructView();
    }
}
