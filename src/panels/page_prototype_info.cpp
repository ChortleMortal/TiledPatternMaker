#include <QCheckBox>
#include <QHeaderView>
#include <QPushButton>
#include <QHBoxLayout>

#include "motifs/motif.h"
#include "panels/panel.h"
#include "panels/page_prototype_info.h"
#include "makers/motif_maker/motif_maker.h"
#include "mosaic/design_element.h"
#include "tile/tile.h"
#include "mosaic/prototype.h"
#include "tile/tiling.h"
#include "viewers/viewcontrol.h"
#include "widgets/layout_sliderset.h"
#include "widgets/panel_misc.h"

using std::string;


page_prototype_info:: page_prototype_info(ControlPanel * cpanel)  : panel_page(cpanel,"Prototype Info")
{
    pview = PrototypeView::getSharedInstance();

    setMouseTracking(true);

    SpinSet * widthSpin = new SpinSet("LineWidth",3,1,9);
    widthSpin->setValue((int)config->protoviewWidth);

    QCheckBox * cbDrawTiles      = new QCheckBox("All Tiles");
    QCheckBox * cbDrawMotifs     = new QCheckBox("All Motifs");
    QCheckBox * cbDrawDEL        = new QCheckBox("Design Element");
    QCheckBox * cbHiliteTiles    = new QCheckBox("DEL Tiles");
    QCheckBox * cbHiliteMotifs   = new QCheckBox("DEL Motifs");
    QCheckBox * cbDrawMap        = new QCheckBox("Prototype Map");

    int mode = config->protoViewMode;
    if (mode & PROTO_DRAW_DESIGN_ELEMENT)
        cbDrawDEL->setChecked(true);
    if (mode & PROTO_DRAW_MAP)
        cbDrawMap->setChecked(true);
    if (mode & PROTO_DRAW_TILES)
        cbDrawTiles->setChecked(true);
    if (mode & PROTO_DRAW_MOTIFS)
        cbDrawMotifs->setChecked(true);
    if (mode & PROTO_DEL_TILES)
        cbHiliteTiles->setChecked(true);
    if (mode & PROTO_DEL_MOTIFS)
        cbHiliteMotifs->setChecked(true);

    QPushButton * refreshButton  = new QPushButton("Refresh");
    QPushButton * deselectButton = new QPushButton("Deselect");

    showSettings = new QGridLayout;
    int row = 0;
    int col = 0;
    showSettings->addWidget(cbDrawMap,row,col++);
    showSettings->addWidget(cbDrawTiles,row,col++);
    showSettings->addWidget(cbDrawMotifs,row,col++);
    showSettings->addWidget(cbDrawDEL,row,col++);
    showSettings->addWidget(cbHiliteTiles,row,col++);
    showSettings->addWidget(cbHiliteMotifs,row,col++);
    showSettings->addWidget(deselectButton,row,col++);
    showSettings->addWidget(refreshButton,row,col++);

    buildColorGrid();

    protoTable = new AQTableWidget(this);
    protoTable->setRowCount(6);
    QStringList qslV;
    qslV << "Prototype" << "Show" << "Tiling" << "Design Element" << "Tile" << "Motif";
    //protoTable->verticalHeader()->setVisible(true);
    protoTable->setVerticalHeaderLabels(qslV);
    protoTable->horizontalHeader()->setVisible(false);
    protoTable->setMaximumWidth(880);
    protoTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    protoTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    protoTable->setSelectionBehavior(QAbstractItemView::SelectColumns);
    protoTable->setSelectionMode(QAbstractItemView::SingleSelection);

    vbox->addLayout(widthSpin);
    vbox->addLayout(showSettings);
    vbox->addSpacing(7);
    vbox->addWidget(protoTable);
    vbox->addStretch();

    connect(refreshButton,    &QPushButton::clicked,    this, &page_prototype_info::setup);
    connect(deselectButton,   &QPushButton::clicked,    this, &page_prototype_info::slot_deselect);
    connect(protoTable,       &AQTableWidget::cellClicked, this, &page_prototype_info::slot_prototypeSelected);
    connect(widthSpin,        &SpinSet::valueChanged,   this, &page_prototype_info::slot_widthChanged);
    connect(cbDrawDEL,        &QCheckBox::clicked,      this, &page_prototype_info::drawDELClicked);
    connect(cbDrawMap,        &QCheckBox::clicked,      this, &page_prototype_info::drawMapClicked);
    connect(cbDrawTiles,      &QCheckBox::clicked,      this, &page_prototype_info::drawTileClicked);
    connect(cbDrawMotifs,     &QCheckBox::clicked,      this, &page_prototype_info::drawMotifClicked);
    connect(cbHiliteTiles,    &QCheckBox::clicked,      this, &page_prototype_info::hiliteTileClicked);
    connect(cbHiliteMotifs,   &QCheckBox::clicked,      this, &page_prototype_info::hiliteMotifClicked);
}

void page_prototype_info::buildColorGrid()
{
    int row = 1;
    int col = 0;
    ProtoViewColors & colors = pview->getColors();

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
}

void page_prototype_info::onEnter()
{
    static QString msg("<body>"
                       "<span>Click on column to select tile  &nbsp;&nbsp; | &nbsp;&nbsp; Click on color to change</span>"
                       "</body>");

    panel->pushPanelStatus(msg);

    setup();
}

void page_prototype_info::onExit()
{
    panel->popPanelStatus();
}

void page_prototype_info::setup()
{
    data.clear();
    protoTable->clearContents();

    const QVector<PrototypePtr> & prototypes = motifMaker->getPrototypes();
    int col = 0;
    QTableWidgetItem * item;
    for (const auto & proto : qAsConst(prototypes))
    {
        const QVector<DesignElementPtr> & dels = proto->getDesignElements();
        for (const auto & del :  qAsConst(dels))
        {
            sColData colData;
            protoTable->setColumnCount(col + 1);

            item = new QTableWidgetItem(addr(proto.get()));
            protoTable->setItem(PROTO_ROW_PROTO,col,item);
            colData.wpp = proto;

            QCheckBox * cb = new QCheckBox();
            cb->setStyleSheet("padding-left: 10px;");
            protoTable->setCellWidget(PROTO_ROW_SHOW,col,cb);
            bool checked = del->getMotif()->getDisplay();
            cb->setChecked(checked);
            connect(cb, &QCheckBox::stateChanged, this,
                    [this,col,cb] { slot_showMotifChanged(col,cb->isChecked()); });

            item = new QTableWidgetItem(proto->getTiling()->getName());
            protoTable->setItem(PROTO_ROW_TILING,col,item);

            item = new QTableWidgetItem(addr(del.get()));
            protoTable->setItem(PROTO_ROW_DEL,col,item);
            colData.wdel = del;

            TilePtr fp = del->getTile();
            Q_ASSERT(fp);
            QString astring = addr(fp.get());
            if (fp->isRegular())
            {
                astring += " sides=" + QString::number(fp->numPoints());
            }
            item = new QTableWidgetItem(astring);
            protoTable->setItem(PROTO_ROW_TILE,col,item);
            colData.wtilep = fp;

            MotifPtr figp = del->getMotif();
            Q_ASSERT(figp);
            astring = addr(figp.get()) + "  " + figp->getMotifDesc();
            item = new QTableWidgetItem(astring);
            protoTable->setItem(PROTO_ROW_MOTIF,col,item);
            colData.wmotifp = figp;

            data.push_back(colData);
            col++;
        }
    }

    protoTable->resizeColumnsToContents();
    protoTable->adjustTableSize(880);
    updateGeometry();
}

void page_prototype_info::slot_deselect()
{
    protoTable->clearSelection();

    pview->getSelectedDEL().reset();

    view->update();
}

void page_prototype_info::slot_prototypeSelected(int row, int col)
{
    Q_UNUSED(row);

    pview->selectDEL(data.at(col));

    emit sig_refreshView();
}

void page_prototype_info::slot_widthChanged(int val)
{
    config->protoviewWidth = qreal(val);
    view->update();
}

void  page_prototype_info::drawDELClicked(bool enb)
{
    setProtoViewMode(PROTO_DRAW_DESIGN_ELEMENT,enb);
}

void  page_prototype_info::drawMapClicked(bool enb)
{
    setProtoViewMode(PROTO_DRAW_MAP,enb);
}

void page_prototype_info::drawMotifClicked(bool enb)
{
    setProtoViewMode(PROTO_DRAW_MOTIFS,enb);
}

void page_prototype_info::drawTileClicked(bool enb)
{
    setProtoViewMode(PROTO_DRAW_TILES,enb);
}

void page_prototype_info::hiliteMotifClicked(bool enb)
{
    setProtoViewMode(PROTO_DEL_MOTIFS,enb);
}

void page_prototype_info::hiliteTileClicked(bool enb)
{
    setProtoViewMode(PROTO_DEL_TILES,enb);
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

        config->protoViewColors = pview->getColors().getColors();
        buildColorGrid();
        emit sig_refreshView();
    }
}

void page_prototype_info::setDefaultColors()
{
    ProtoViewColors colors;
    colors.mapColor             = QColor(20,150,210);
    colors.tileColor         = QColor(0, 108, 0);
    colors.motifColor          = QColor(214,0,0);
    colors.delMotifColor       = QColor(Qt::blue);
    colors.delTileColor      = QColor(Qt::yellow);
    colors.tileBrushColor    = QColor(255, 217, 217,128);
    ProtoViewColors & viewColors = pview->getColors();
    viewColors = colors;

    config->protoViewColors = pview->getColors().getColors();
    buildColorGrid();
    emit sig_refreshView();
}

void page_prototype_info::slot_showMotifChanged(int col, bool checked)
{
    const sColData & colData = data.at(col);

    auto fp = colData.wmotifp.lock();
    if (fp)
    {
        fp->setDisplay(checked);
    }
    view->update();
}
