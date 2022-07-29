#include <QCheckBox>
#include <QHeaderView>
#include <QPushButton>
#include <QHBoxLayout>

#include "figures/figure.h"
#include "panels/panel.h"
#include "panels/page_prototype_info.h"
#include "makers/motif_maker/motif_maker.h"
#include "mosaic/design_element.h"
#include "tile/feature.h"
#include "tile/tiling.h"
#include "viewers/viewcontrol.h"
#include "widgets/panel_misc.h"

using std::string;


page_prototype_info:: page_prototype_info(ControlPanel * cpanel)  : panel_page(cpanel,"Prototype Info")
{
    pview = PrototypeView::getSharedInstance();

    setMouseTracking(true);

    QCheckBox * cbDrawFeatures   = new QCheckBox("All Features");
    QCheckBox * cbDrawfigures    = new QCheckBox("All Figures");
    QCheckBox * cbDrawDEL        = new QCheckBox("Design Element");
    QCheckBox * cbHiliteFeatures = new QCheckBox("DEL Features");
    QCheckBox * cbHiliteFigures  = new QCheckBox("DEL Figures");
    QCheckBox * cbDrawMap        = new QCheckBox("Prototype Map");

    int mode = config->protoViewMode;
    if (mode & PROTO_DRAW_DESIGN_ELEMENT)
        cbDrawDEL->setChecked(true);
    if (mode & PROTO_DRAW_MAP)
        cbDrawMap->setChecked(true);
    if (mode & PROTO_DRAW_FEATURES)
        cbDrawFeatures->setChecked(true);
    if (mode & PROTO_DRAW_FIGURES)
        cbDrawfigures->setChecked(true);
    if (mode & PROTO_DEL_FEATURES)
        cbHiliteFeatures->setChecked(true);
    if (mode & PROTO_DEL_FIGURES)
        cbHiliteFigures->setChecked(true);

    QPushButton * refreshButton = new QPushButton("Refresh");
    QPushButton * defaultButton = new QPushButton("Default Colors");

    showSettings = new QGridLayout;
    int row = 0;
    int col = 0;
    showSettings->addWidget(cbDrawMap,row,col++);
    showSettings->addWidget(cbDrawFeatures,row,col++);
    showSettings->addWidget(cbDrawfigures,row,col++);
    showSettings->addWidget(cbDrawDEL,row,col++);
    showSettings->addWidget(cbHiliteFeatures,row,col++);
    showSettings->addWidget(cbHiliteFigures,row,col++);
    showSettings->addWidget(defaultButton,row,col++);
    showSettings->addWidget(refreshButton,row,col++);

    buildColorGrid();

    protoTable = new AQTableWidget(this);
    protoTable->setRowCount(9);
    QStringList qslV;
    qslV << "Prototype" << "Tiling" << "Design Element" << "Feature" << "Figure" << "Scale" << "Rotate" << "Trans-X" << "Trans-Y";
    //protoTable->verticalHeader()->setVisible(true);
    protoTable->setVerticalHeaderLabels(qslV);
    protoTable->horizontalHeader()->setVisible(false);
    protoTable->setMaximumWidth(880);
    protoTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    protoTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    protoTable->setSelectionBehavior(QAbstractItemView::SelectColumns);
    protoTable->setSelectionMode(QAbstractItemView::SingleSelection);
    vbox->addLayout(showSettings);
    vbox->addSpacing(7);
    vbox->addWidget(protoTable);
    vbox->addStretch();

    connect(refreshButton,    &QPushButton::clicked,    this, &page_prototype_info::setup);
    connect(defaultButton,    &QPushButton::clicked,    this, &page_prototype_info::setDefaultColors);
    connect(protoTable,       SIGNAL(cellClicked(int,int)), this,   SLOT(slot_prototypeSelected(int,int)));
    connect(cbDrawDEL,        &QCheckBox::clicked,      this, &page_prototype_info::drawDELClicked);
    connect(cbDrawMap,        &QCheckBox::clicked,      this, &page_prototype_info::drawMapClicked);
    connect(cbDrawFeatures,   &QCheckBox::clicked,      this, &page_prototype_info::drawFeatureClicked);
    connect(cbDrawfigures,    &QCheckBox::clicked,      this, &page_prototype_info::drawFigureClicked);
    connect(cbHiliteFeatures, &QCheckBox::clicked,      this, &page_prototype_info::hiliteFeatureClicked);
    connect(cbHiliteFigures,  &QCheckBox::clicked,      this, &page_prototype_info::hiliteFigureClicked);
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
    variant = colors.featureColor;
    colcode  = variant.toString();
    label->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    showSettings->addWidget(label,row,col++);
    connect(label,&ClickableLabel::clicked, this, [this, &colors] { pickColor(colors.featureColor); });

    label = new ClickableLabel();
    variant = colors.figureColor;
    colcode  = variant.toString();
    label->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    showSettings->addWidget(label,row,col++);
    connect(label,&ClickableLabel::clicked, this, [this, &colors] { pickColor(colors.figureColor); });

    label = new ClickableLabel();
    variant = colors.featureBrushColor;
    colcode  = variant.toString();
    label->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    showSettings->addWidget(label,row,col++);
    connect(label,&ClickableLabel::clicked, this, [this, &colors] { pickColor(colors.featureBrushColor); });

    label = new ClickableLabel();
    variant = colors.delFeatureColor;
    colcode  = variant.toString();
    label->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    showSettings->addWidget(label,row,col++);
    connect(label,&ClickableLabel::clicked, this, [this, &colors] { pickColor(colors.delFeatureColor); });

    label = new ClickableLabel();
    variant = colors.delFigureColor;
    colcode  = variant.toString();
    label->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    showSettings->addWidget(label,row,col++);
    connect(label,&ClickableLabel::clicked, this, [this, &colors] { pickColor(colors.delFigureColor); });
}

void  page_prototype_info::onRefresh()
{
}

void page_prototype_info::onEnter()
{
    static QString msg("<body>"
                       "<span>Click on column to select feature  &nbsp;&nbsp; | &nbsp;&nbsp; Click on color to change</span>"
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

            item = new QTableWidgetItem(proto->getTiling()->getName());
            protoTable->setItem(PROTO_ROW_TILING,col,item);

            item = new QTableWidgetItem(addr(del.get()));
            protoTable->setItem(PROTO_ROW_DEL,col,item);
            colData.wdel = del;

            FeaturePtr fp = del->getFeature();
            Q_ASSERT(fp);
            QString astring = addr(fp.get());
            if (fp->isRegular())
            {
                astring += " sides=" + QString::number(fp->numPoints());
            }
            item = new QTableWidgetItem(astring);
            protoTable->setItem(PROTO_ROW_FEATURE,col,item);
            colData.wfeatp = fp;

            FigurePtr figp = del->getFigure();
            Q_ASSERT(figp);
            astring = addr(figp.get()) + "  " + figp->getFigureDesc();
            item = new QTableWidgetItem(astring);
            protoTable->setItem(PROTO_ROW_FIGURE,col,item);
            colData.wfigp = figp;
#if 0
            QTransform t = proto->getTransform(col);

            item = new QTableWidgetItem(QString::number(Transform::scalex(t),'f',6));
            protoTable->setItem(PROTO_ROW_SCALE,col,item);

            qreal rotation = Transform::rotation(t);
            qreal degrees  = qRadiansToDegrees(rotation);
            item = new QTableWidgetItem(QString::number(degrees,'f',6));
            protoTable->setItem(PROTO_ROW_ROT,col,item);

            item = new QTableWidgetItem(QString::number(Transform::transx(t),'f',6));
            protoTable->setItem(PROTO_ROW_X,col,item);

            item = new QTableWidgetItem(QString::number(Transform::transy(t),'f',6));
            protoTable->setItem(PROTO_ROW_Y,col,item);
#endif
            data.push_back(colData);
            col++;
        }
    }

    protoTable->resizeColumnsToContents();
    protoTable->adjustTableSize(880);
    updateGeometry();
}

void page_prototype_info::slot_prototypeSelected(int row, int col)
{
    Q_UNUSED(row);

    const sColData & colData = data.at(col);

    PrototypePtr pp = colData.wpp.lock();
    if (pp)
    {
        motifMaker->setSelectedPrototype(pp);
    }

    auto del = colData.wdel.lock();
    if (del)
    {
        motifMaker->setSelectedDesignElement(del);
    }

    auto fp = colData.wfeatp.lock();
    if (fp)
    {
        view->selectFeature(fp);
    }

    emit sig_refreshView();
}

void  page_prototype_info::drawDELClicked(bool enb)
{
    setProtoViewMode(PROTO_DRAW_DESIGN_ELEMENT,enb);
}

void  page_prototype_info::drawMapClicked(bool enb)
{
    setProtoViewMode(PROTO_DRAW_MAP,enb);
}

void page_prototype_info::drawFigureClicked(bool enb)
{
    setProtoViewMode(PROTO_DRAW_FIGURES,enb);
}

void page_prototype_info::drawFeatureClicked(bool enb)
{
    setProtoViewMode(PROTO_DRAW_FEATURES,enb);
}

void page_prototype_info::hiliteFigureClicked(bool enb)
{
    setProtoViewMode(PROTO_DEL_FIGURES,enb);
}

void page_prototype_info::hiliteFeatureClicked(bool enb)
{
    setProtoViewMode(PROTO_DEL_FEATURES,enb);
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
    colors.featureColor         = QColor(0, 108, 0);
    colors.figureColor          = QColor(214,0,0);
    colors.delFigureColor       = QColor(Qt::blue);
    colors.delFeatureColor      = QColor(Qt::yellow);
    colors.featureBrushColor    = QColor(255, 217, 217, 32);
    ProtoViewColors & viewColors = pview->getColors();
    viewColors = colors;

    config->protoViewColors = pview->getColors().getColors();
    buildColorGrid();
    emit sig_refreshView();
}
