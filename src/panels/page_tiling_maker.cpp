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

#include "page_tiling_maker.h"
#include "base/patterns.h"
#include "base/canvas.h"
#include "base/tiledpatternmaker.h"
#include "makers/TilingDesigner.h"
#include "style/Style.h"

using std::string;

Q_DECLARE_METATYPE(PlacedFeaturePtr)

enum epageTi
{
    TI_INDEX,
    TI_SIDES,
    TI_SCALE,
    TI_ROT,
    TI_X,
    TI_Y,
    TI_REGULAR,
    TI_FEAT_ADDR,
    TI_LOCATION
};

page_tiling_maker:: page_tiling_maker(ControlPanel *panel)  : panel_page(panel,"Tiling Maker")
{
    hideTable = false;

    createTopGrid(vbox);

    AQWidget * widget;
    widget = createTilingTable();
    vbox->addWidget(widget);

    widget = createTilingDesignerControls();
    vbox->addWidget(widget);

    vbox->addStretch(); // end vbox

    connect(&sidesMapper,  SIGNAL(mapped(int)), this, SLOT(slot_sidesChanged(int)));
    connect(&scaleMapper,  SIGNAL(mapped(int)), this, SLOT(slot_transformChanged(int)));
    connect(&rotMapper,    SIGNAL(mapped(int)), this, SLOT(slot_transformChanged(int)));
    connect(&xMapper,      SIGNAL(mapped(int)), this, SLOT(slot_transformChanged(int)));
    connect(&yMapper,      SIGNAL(mapped(int)), this, SLOT(slot_transformChanged(int)));

    connect(&tile_name,  SIGNAL(editingFinished()), this, SLOT(slot_nameChanged()));
    connect(&tile_author,SIGNAL(editingFinished()), this, SLOT(slot_authorChanged()));
    connect(&tile_desc,  SIGNAL(textChanged()),     this, SLOT(slot_descChanged()));

    connect(maker,  &TiledPatternMaker::sig_loadedTiling,   this,   &page_tiling_maker::slot_loadedTiling);
    connect(maker,  &TiledPatternMaker::sig_loadedXML,      this,   &page_tiling_maker::slot_loadedXML);
}

void page_tiling_maker::createTopGrid(QVBoxLayout * vbox)
{
    QHBoxLayout* hbox1 = new QHBoxLayout;

    ///
    /// Source select row
    ///

    radioLoadedStyleTileView = new QRadioButton("Style");
    radioWSTileView          = new QRadioButton("Workspace");

    QPushButton * pbClearWS       = new QPushButton("Clear WS");
    chk_autoFill                  = new QCheckBox("Auto-fill on load");
    QCheckBox * chk_all_overlaps  = new QCheckBox("All overlaps");
    QCheckBox * chk_all_features  = new QCheckBox("All features");
    chk_all_features->setChecked(config->all_features_chk);

    hbox1->addWidget(radioLoadedStyleTileView);
    hbox1->addWidget(radioWSTileView);
    hbox1->addStretch();
    hbox1->addWidget(pbClearWS);
    hbox1->addWidget(chk_all_features);
    hbox1->addWidget(chk_all_overlaps);
    hbox1->addWidget(chk_autoFill);

    tilingGroup3.addButton(radioLoadedStyleTileView,TD_STYLE);
    tilingGroup3.addButton(radioWSTileView,TD_WORKSPACE);

    connect(&tilingGroup3,      SIGNAL(buttonClicked(int)), this,    SLOT(slot_sourceSelect(int)));
    connect(chk_all_features,   &QCheckBox::clicked,        this,    &page_tiling_maker::slot_all_features);
    connect(chk_all_overlaps,   &QCheckBox::clicked,        this,    &page_tiling_maker::slot_all_overlaps);
    connect(pbClearWS,          &QPushButton::clicked,      this,    &page_tiling_maker::slot_clearWS);

    ///
    ///  Fill Data Row
    ///

    QHBoxLayout * hbox2 = new QHBoxLayout;

    const int rmin = -1000;
    const int rmax =  1000;

    xRepMin = new SpinSet("xMin",0,rmin,rmax);
    xRepMax = new SpinSet("xMax",0,rmin,rmax);
    yRepMin = new SpinSet("yMin",0,rmin,rmax);
    yRepMax = new SpinSet("yMax",0,rmin,rmax);
    QPushButton * pb =  new QPushButton("Set");
    QPushButton * replaceInDesignBtn   = new QPushButton("Replace in Design");
    replaceInDesignBtn->setMinimumWidth(121);

    hbox2->addLayout(xRepMin);
    hbox2->addLayout(xRepMax);
    hbox2->addLayout(yRepMin);
    hbox2->addLayout(yRepMax);
    hbox2->addWidget(pb);
    hbox2->addStretch();
    hbox2->addWidget(replaceInDesignBtn);

    connect(pb,                  &QPushButton::clicked, this,  &page_tiling_maker::set_reps);
    connect(replaceInDesignBtn,  &QPushButton::clicked, this,  &page_tiling_maker::slot_updateTiling);

    ///
    /// Translations row
    ///

    QGridLayout * grid3 = new QGridLayout;
    int row = 0;

    t1x = new DoubleSpinSet("T1-X",0,-100.0,100.0);
    t1y = new DoubleSpinSet("T1-Y",0,-100.0,100.0);
    t2x = new DoubleSpinSet("T2-X",0,-100.0,100.0);
    t2y = new DoubleSpinSet("T2-Y",0,-100.0,100.0);
    t1x->setAlignment(Qt::AlignLeft);
    t1y->setAlignment(Qt::AlignLeft);
    t2x->setAlignment(Qt::AlignLeft);
    t2y->setAlignment(Qt::AlignLeft);
    t1x->setDecimals(16);
    t1y->setDecimals(16);
    t2x->setDecimals(16);
    t2y->setDecimals(16);


    grid3->addLayout(t1x,row,0);
    grid3->addLayout(t1y,row,1);
    grid3->addLayout(t2x,row,2);
    grid3->addLayout(t2y,row,3);

    QPushButton * swapBtn = new QPushButton("Swap");
    grid3->addWidget(swapBtn,row,4, Qt::AlignCenter);

    QObject::connect(t1x,  SIGNAL(valueChanged(double)), this, SLOT(slot_t1t2Changed(double)));
    QObject::connect(t1y,  SIGNAL(valueChanged(double)), this, SLOT(slot_t1t2Changed(double)));
    QObject::connect(t2x,  SIGNAL(valueChanged(double)), this, SLOT(slot_t1t2Changed(double)));
    QObject::connect(t2y,  SIGNAL(valueChanged(double)), this, SLOT(slot_t1t2Changed(double)));
    QObject::connect(swapBtn,  &QPushButton::clicked,    this, &page_tiling_maker::slot_swapTrans);

    vbox->addLayout(hbox1);
    vbox->addLayout(hbox2);
    vbox->addLayout(grid3);
}

AQWidget * page_tiling_maker::createTilingTable()
{
    // tile info table
    tileInfoTable = new QTableWidget();
    tileInfoTable->setColumnCount(9);
    tileInfoTable->setSelectionMode(QAbstractItemView::SingleSelection);
    tileInfoTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    tileInfoTable->setStyleSheet("selection-color : white; selection-background-color : green");

    QStringList qslh;
    qslh << "index" << "sides" << "scale" << "rot" << "X" << "Y" << "reg" << "feat" << "loc";
    tileInfoTable->setHorizontalHeaderLabels(qslh);
    tileInfoTable->verticalHeader()->setVisible(false);

    tile_desc.setFixedHeight(70);
    pointLabel.setFixedHeight(50);

    debugLabel = new QLabel("debug label");
    chk_hideTable = new QCheckBox("Hide table");

    QPushButton * remove = new QPushButton("Remove row figure");
    remove->setMinimumWidth(111);

    QPushButton * refreshBtn = new QPushButton("Re-enter");

    pbSave.setText("Save XML");
    pbSave.setMinimumWidth(111);

    QGridLayout * grid = new QGridLayout;
    //grid->setVerticalSpacing(5);
    grid->setHorizontalSpacing(13);

    grid->addWidget(debugLabel,0,0);
    grid->addWidget(chk_hideTable,0,2);
    grid->addWidget(remove,0,4);
    grid->addWidget(refreshBtn,0,5);
    grid->addWidget(&pbSave,0,6);

    AQVBoxLayout * layout = new AQVBoxLayout();
    layout->addLayout(grid);
    layout->addSpacing(11);
    layout->addWidget(tileInfoTable);
    layout->addSpacing(11);
    layout->addWidget(&pointLabel);
    layout->addSpacing(3);
    layout->addWidget(&tile_desc);
    layout->addSpacing(3);
    layout->addWidget(&tile_author);
    layout->addSpacing(3);
    layout->addWidget(&tile_name);
    layout->addSpacing(5);

    AQWidget * widget = new AQWidget();
    widget->setLayout(layout);

    connect(remove,     &QPushButton::clicked,   this,   &page_tiling_maker::slot_remove_clicked);
    connect(refreshBtn, &QPushButton::clicked,   this,   &page_tiling_maker::onEnter);
    connect(&pbSave,    &QPushButton::clicked,   this,   &page_tiling_maker::slot_saveTiling);
    connect(chk_hideTable,&QCheckBox::clicked,   this,   &page_tiling_maker::slot_hideTable);

    return widget;
}

AQWidget * page_tiling_maker::createTilingDesignerControls()
{
    // create tiling designer
    designer = TilingDesigner::getInstance();

    // actions
    QPushButton * addPolyBtn      = new QPushButton("Add reguar poly (A)");
    QPushButton * irregPolyBtn    = new QPushButton("Add irregular poly (B)");
    QPushButton * copyBtn         = new QPushButton("Copy (C)");
    QPushButton * deleteBtn       = new QPushButton("Delete (D)");
    QPushButton * exclAllBtn      = new QPushButton("Exclude all (E)");
    QPushButton * fillVectorsBtn  = new QPushButton("Fill using vectors (F)");
    QPushButton * inclusionBtn    = new QPushButton("Include (I)");
    QPushButton * removeExclBtn   = new QPushButton("Remove Excluded (R)");
    QPushButton * clearVectorsBtn = new QPushButton("Clear vectors (V)");

    sides = new QSpinBox;
    sides->setFixedWidth(41);
    sides->setMinimum(3);
    sides->setMaximum(128);

    QGridLayout  * modeBox = new QGridLayout;
    modeBox->setVerticalSpacing(5);
    modeBox->setHorizontalSpacing(9);

    int row = 0;
    modeBox->addWidget(addPolyBtn,row,0);
    modeBox->addWidget(sides,row,1,Qt::AlignLeft);
    modeBox->addWidget(irregPolyBtn,row,2);
    modeBox->addWidget(copyBtn,row,3);
    modeBox->addWidget(deleteBtn,row,4);

    row++;
    modeBox->addWidget(exclAllBtn,row,0);
    modeBox->addWidget(fillVectorsBtn,row,1);
    modeBox->addWidget(inclusionBtn,row,2);
    modeBox->addWidget(removeExclBtn,row,3);
    modeBox->addWidget(clearVectorsBtn,row,4);

    // modes
    QRadioButton * nomode       = new QRadioButton("No Mode (ESC)");
    QRadioButton * transform    = new QRadioButton("Transform (F2)");
    QRadioButton * drawTrans    = new QRadioButton("Draw translations (F3)");
    QRadioButton * newPoly      = new QRadioButton("New poly (F4)");
    QRadioButton * copyPoly     = new QRadioButton("Copy poly (F5)");
    QRadioButton * deletePoly   = new QRadioButton("Delete poly (F6)");
    QRadioButton * includePoly  = new QRadioButton("Include poly (F7)");
    QRadioButton * position     = new QRadioButton("Position (F8)");
    QRadioButton * measure      = new QRadioButton("Measure (F9)");

    row++;
    modeBox->addWidget(nomode,row,0);
    modeBox->addWidget(transform,row,1);
    modeBox->addWidget(drawTrans,row,2);
    modeBox->addWidget(newPoly,row,3);
    modeBox->addWidget(copyPoly,row,4);
    row++;
    modeBox->addWidget(deletePoly,row,0);
    modeBox->addWidget(includePoly,row,1);
    modeBox->addWidget(position,row,2);
    modeBox->addWidget(measure,row,3);

    // grouping
    mouseModeBtnGroup = new QButtonGroup;
    mouseModeBtnGroup->addButton(transform,TRANSFORM_MODE);
    mouseModeBtnGroup->addButton(drawTrans,TRANS_MODE);
    mouseModeBtnGroup->addButton(newPoly,DRAW_POLY_MODE);
    mouseModeBtnGroup->addButton(copyPoly,COPY_MODE);
    mouseModeBtnGroup->addButton(deletePoly,DELETE_MODE);
    mouseModeBtnGroup->addButton(includePoly,INCLUSION_MODE);
    mouseModeBtnGroup->addButton(nomode,NO_MODE);
    mouseModeBtnGroup->addButton(position,POSITION_MODE);
    mouseModeBtnGroup->addButton(measure, MEASURE_MODE);

    AQVBoxLayout * avbox = new AQVBoxLayout;
    avbox->addSpacing(5);
    avbox->addLayout(modeBox);
    avbox->addSpacing(5);

    AQWidget * widget = new AQWidget();
    widget->setLayout(avbox);

    connect(designer,      &TilingDesigner::hasChanged,         this,   &page_tiling_maker::onEnter);
    connect(designer,      &TilingDesigner::sig_current_feature,this,   &page_tiling_maker::currentFeature);

    connect(mouseModeBtnGroup, SIGNAL(buttonClicked(int)),     this,        SLOT(slot_setModes(int)));
    connect(sides,             SIGNAL(valueChanged(int)),      designer,    SLOT(updatePolygonSides(int)));

    connect(addPolyBtn,       &QPushButton::clicked,          designer,    &TilingDesigner::addRegularPolygon);
    connect(irregPolyBtn,     &QPushButton::clicked,          designer,    &TilingDesigner::addIrregularPolygon);
    connect(clearVectorsBtn,  &QPushButton::clicked,          designer,    &TilingDesigner::clearTranslation);
    connect(fillVectorsBtn,   &QPushButton::clicked,          designer,    &TilingDesigner::fillUsingTranslations);
    connect(removeExclBtn,    &QPushButton::clicked,          designer,    &TilingDesigner::removeExcluded);
    connect(exclAllBtn,       &QPushButton::clicked,          designer,    &TilingDesigner::excludeAll);

    sides->setValue(8);

    return widget;
}

void page_tiling_maker::slot_all_features(bool checked)
{
    config->all_features_chk = checked;
    onEnter();
}

void page_tiling_maker::slot_all_overlaps(bool checked)
{
    designer->setDrawAllOverlaps(checked);
    sig_viewWS();
}

void page_tiling_maker::slot_setModes(int mode)
{
    eMouseMode mm =static_cast<eMouseMode>(mode);
    designer->setMouseMode(mm);
}

void page_tiling_maker::clear()
{
    tile_desc.blockSignals(true);
    tile_desc.clear();
    tile_desc.blockSignals(false);
    tile_name.clear();
    tile_author.clear();
    pointLabel.clear();

    tileInfoTable->clearContents();
    tileInfoTable->setMaximumHeight(401);

    t1x->blockSignals(true);
    t1y->blockSignals(true);
    t2x->blockSignals(true);
    t2y->blockSignals(true);

    t1x->setValue(0);
    t1y->setValue(0);
    t2x->setValue(0);
    t2y->setValue(0);

    t1x->blockSignals(false);
    t1y->blockSignals(false);
    t2x->blockSignals(false);
    t2y->blockSignals(false);
}

void  page_tiling_maker::refreshPage()
{
    TilingPtr tiling = workspace->getLoadedStyles().getTiling();
    QString txt = "Style ";
    txt        += addr(tiling.get());
    if (tiling)
    {
        txt += " ";
        txt += tiling->getName();
    }
    radioLoadedStyleTileView->setText(txt);

    tiling = workspace->getTiling();
    txt  = "Workspace ";
    txt += addr(tiling.get());
    if (tiling)
    {
        txt += " ";
        txt += tiling->getName();
    }
    radioWSTileView->setText(txt);

    tilingGroup3.button(config->tilingMakerViewer)->setChecked(true);
    mouseModeBtnGroup->button(designer->getMouseMode())->setChecked(true);

    QPointF a = designer->mousePos;
    QPointF b = designer->screenToWorld(a);
    QString astring;
    QTextStream ts(&astring);
    ts << "pos: (" << a.x() << ", " << a.y() << ") ("
                   << b.x() << ", " << b.y() << ")";
    debugLabel->setText(astring);

    sides->setValue(designer->getPolygonSides());

    designer->updateStatus();
}

void  page_tiling_maker::onEnter()
{
    clear();

    int id = designer->getMouseMode();
    mouseModeBtnGroup->button(id)->setChecked(true);

    tilingGroup3.button(config->tilingMakerViewer)->setChecked(true);

    TilingPtr tiling = getSourceTiling();
    displayTilingDesigner(tiling);
}

void page_tiling_maker::displayTilingDesigner(TilingPtr tiling)
{
    if (!tiling)
    {
        // if workspace is selected then make a new tiling;
        if (config->tilingMakerViewer== TD_WORKSPACE)
        {
            tiling = make_shared<Tiling>();
            workspace->setTiling(tiling);
        }
        else
        {
            // nothing to work on
            Q_ASSERT(config->tilingMakerViewer== TD_STYLE);
            designer->clearDesigner();
            lastTiling.reset();
            return;
        }
    }

    if (tiling != lastTiling)
    {
        // only set tiling if it has changed
        lastTiling = tiling;
        designer->setTiling(tiling);
    }

    tile_name.setText(tiling->getName());
    tile_author.setText(tiling->getAuthor());
    tile_desc.blockSignals(true);
    tile_desc.setText(tiling->getDescription());
    tile_desc.blockSignals(false);

    QPointF t1 = designer->getTrans1();
    QPointF t2 = designer->getTrans2();
    tiling->setTrans1(t1);
    tiling->setTrans2(t2);

    t1x->blockSignals(true);
    t1y->blockSignals(true);
    t2x->blockSignals(true);
    t2y->blockSignals(true);

    t1x->setValue(t1.x());
    t1y->setValue(t1.y());
    t2x->setValue(t2.x());
    t2y->setValue(t2.y());

    t1x->blockSignals(false);
    t1y->blockSignals(false);
    t2x->blockSignals(false);
    t2y->blockSignals(false);

    int xMin,xMax,yMin,yMax;
    tiling->getFillData().get(xMin ,xMax,yMin,yMax);
    xRepMin->setValue(xMin);
    xRepMax->setValue(xMax);
    yRepMin->setValue(yMin);
    yRepMax->setValue(yMax);

    tileInfoTable->clearContents();
    tileInfoTable->setRowCount(0);

    int row = 0;

    QVector<PlacedFeaturePtr> & pfeatures = (config->all_features_chk)
        ? designer->getFeatures()
        : designer->getInTiling();
    QString label = (config->all_features_chk) ? "features" : "in tiling";

    for (auto it = pfeatures.begin(); it != pfeatures.end(); it++, row++)
    {
        PlacedFeaturePtr pf = *it;
        displayPlacedFeature(pf,row,label);
    }

    tileInfoTable->resizeColumnsToContents();
    adjustTableSize(tileInfoTable);

    tileInfoTable->setVisible(!hideTable);
}

void page_tiling_maker::displayPlacedFeature(PlacedFeaturePtr pf, int row, QString from)
{
    FeaturePtr feature  = pf->getFeature();
    Transform T         = pf->getTransform();

    tileInfoTable->setRowCount(row+1);

    QTableWidgetItem * twi = new QTableWidgetItem(QString::number(row));
    twi->setData(Qt::UserRole,QVariant::fromValue(pf));
    tileInfoTable->setItem(row,TI_INDEX,twi);

    QSpinBox * sp = new QSpinBox;
    sp->setValue(feature->numPoints());
    tileInfoTable->setCellWidget(row,TI_SIDES,sp);
    QObject::connect(sp, SIGNAL(valueChanged(int)), &sidesMapper, SLOT(map()));
    sidesMapper.setMapping(sp,row);

    QDoubleSpinBox * dsp = new QDoubleSpinBox;
    dsp->setRange(0,128);
    dsp->setDecimals(16);
    dsp->setSingleStep(0.01);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(T.scalex());
    tileInfoTable->setCellWidget(row,TI_SCALE,dsp);
    QObject::connect(dsp, SIGNAL(valueChanged(double)), &scaleMapper, SLOT(map()));
    scaleMapper.setMapping(dsp,row);

    dsp = new QDoubleSpinBox;
    dsp->setRange(-360.0,360.0);
    dsp->setDecimals(16);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(qRadiansToDegrees(T.rotation()));
    tileInfoTable->setCellWidget(row,TI_ROT,dsp);
    QObject::connect(dsp, SIGNAL(valueChanged(double)), &rotMapper, SLOT(map()));
    rotMapper.setMapping(dsp,row);

    dsp = new QDoubleSpinBox;
    dsp->setRange(-100.0,100.0);
    dsp->setDecimals(16);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(T.transx());
    tileInfoTable->setCellWidget(row,TI_X,dsp);
    QObject::connect(dsp, SIGNAL(valueChanged(double)), &xMapper, SLOT(map()));
    xMapper.setMapping(dsp,row);

    dsp = new QDoubleSpinBox;
    dsp->setRange(-100.0,100.0);
    dsp->setDecimals(16);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(T.transy());
    tileInfoTable->setCellWidget(row,TI_Y,dsp);
    QObject::connect(dsp, SIGNAL(valueChanged(double)), &yMapper, SLOT(map()));
    yMapper.setMapping(dsp,row);

    QString regular = feature->isRegular() ? "Y" : "N";
    twi = new QTableWidgetItem(regular);
    tileInfoTable->setItem(row,TI_REGULAR,twi);

    twi = new QTableWidgetItem(addr(feature.get()));
    tileInfoTable->setItem(row,TI_FEAT_ADDR,twi);

    twi = new QTableWidgetItem(from);
    tileInfoTable->setItem(row,TI_LOCATION,twi);

    connect(tileInfoTable, SIGNAL(cellClicked(int,int)),  this, SLOT(slot_cellSelected(int,int)));
}

PlacedFeaturePtr page_tiling_maker::getFeatureRow(int row)
{
    PlacedFeaturePtr pf;
    if (row == -1) return pf;

    QTableWidgetItem * twi = tileInfoTable->item(row,TI_INDEX);
    if (twi)
    {
        QVariant var = twi->data(Qt::UserRole);
        if (var.canConvert<PlacedFeaturePtr>())
        {
            pf = var.value<PlacedFeaturePtr>();
        }
        Q_ASSERT(pf);
    }
    return pf;
}

void page_tiling_maker::slot_sidesChanged(int row)
{
    QWidget * cw  = tileInfoTable->cellWidget(row,TI_SIDES);
    QSpinBox * sp = dynamic_cast<QSpinBox*>(cw);
    Q_ASSERT(sp);
    int sides = sp->value();
    FeaturePtr f2 = make_shared<Feature>(sides);

    PlacedFeaturePtr pf = getFeatureRow(row);
    pf->setFeature(f2);

    emit sig_viewWS();
    emit sig_tilingChanged();
}

void page_tiling_maker::slot_transformChanged(int row)
{
    QWidget        * cw  = tileInfoTable->cellWidget(row,TI_SCALE);
    QDoubleSpinBox * dsp = dynamic_cast<QDoubleSpinBox*>(cw);
    Q_ASSERT(dsp);
    qreal scale = dsp->value();
    if (scale <= 0.0 || scale > 128.0)
    {
        return;     // fixes problem with scales of 0 when entering data
    }

    cw  = tileInfoTable->cellWidget(row,TI_ROT);
    dsp = dynamic_cast<QDoubleSpinBox*>(cw);
    Q_ASSERT(dsp);
    qreal rotation = qDegreesToRadians(dsp->value());

    cw  = tileInfoTable->cellWidget(row,TI_X);
    dsp = dynamic_cast<QDoubleSpinBox*>(cw);
    Q_ASSERT(dsp);
    qreal tx = dsp->value();

    cw  = tileInfoTable->cellWidget(row,TI_Y);
    dsp = dynamic_cast<QDoubleSpinBox*>(cw);
    Q_ASSERT(dsp);
    qreal ty  = dsp->value();

    Transform t = Transform::compose(scale, rotation, QPointF(tx,ty));
    qDebug().noquote() << "row=" << row << "T=" << t.toString();

    PlacedFeaturePtr pf = getFeatureRow(row);
    pf->setTransform(t);

    canvas->invalidate();
    emit sig_tilingChanged();
}

void page_tiling_maker::slot_t1t2Changed(double)
{
    qreal x1 = t1x->value();
    qreal y1 = t1y->value();
    qreal x2 = t2x->value();
    qreal y2 = t2y->value();


    TilingPtr tiling = getSourceTiling();
    if (!tiling) return;

    tiling->setTrans1(QPointF(x1,y1));
    tiling->setTrans2(QPointF(x2,y2));

    designer->setTiling(tiling);

    emit sig_tilingChanged();
}

void page_tiling_maker::slot_swapTrans()
{
    qreal x1 = t1x->value();
    qreal y1 = t1y->value();
    qreal x2 = t2x->value();
    qreal y2 = t2y->value();

    t1x->setValue(x2);
    t1y->setValue(y2);
    t2x->setValue(x1);
    t2y->setValue(y1);

    slot_t1t2Changed(0);
}

void page_tiling_maker::slot_nameChanged()
{
    TilingPtr tiling = getSourceTiling();
    if (!tiling) return;
    tiling->setName(tile_name.text());
}

void page_tiling_maker::slot_authorChanged()
{
    TilingPtr tiling = getSourceTiling();
    if (!tiling) return;
    tiling->setAuthor(tile_author.text());
}

void page_tiling_maker::slot_descChanged()
{
    TilingPtr tiling = getSourceTiling();
    if (!tiling) return;
    tiling->setDescription(tile_desc.toPlainText());
}

void page_tiling_maker::slot_saveTiling()
{
    if (tile_name.text().isEmpty())
    {
        QMessageBox box(this);
        box.setText("Nothing to save");
        box.exec();
        return;
    }

    TilingPtr tiling = getSourceTiling();
    if (!tiling) return;
    designer->updateTilingFromData(tiling);

    workspace->saveTiling(tile_name.text(),tiling);
}

void page_tiling_maker::slot_updateTiling()
{
    TilingPtr tiling = getSourceTiling();
    if (!tiling) return;

    // update local tiling
    designer->updateTilingFromData(tiling);

    // update styles
    StyledDesign & sd = workspace->getLoadedStyles();
    const StyleSet & sset = sd.getStyleSet();
    for (auto it = sset.begin(); it != sset.end(); it++)
    {
        StylePtr sp = *it;
        PrototypePtr pp = sp->getPrototype();
        if (pp->getTiling()->getName() == tiling->getName())
        {
            pp->setTiling(tiling);  // can be set several times with same value
        }
    }

    emit sig_tilingChanged();
    emit sig_render();
    emit sig_viewWS();
    onEnter();
}

void page_tiling_maker::slot_sourceSelect(int id)
{
    config->tilingMakerViewer = eTilingMakerView(id);
    onEnter();
    emit sig_viewWS();
    emit sig_tilingChanged();
}

void page_tiling_maker::slot_remove_clicked()
{
    TilingPtr tiling = getSourceTiling();
    if (!tiling) return;

    int row = tileInfoTable->currentRow();
    PlacedFeaturePtr pf = getFeatureRow(row);

    tiling->remove(pf);
    designer->removeFeature(pf);
    onEnter();
    emit sig_viewWS();
    emit sig_tilingChanged();
}

void page_tiling_maker::set_reps()
{
    TilingPtr tiling = getSourceTiling();
    if (!tiling) return;
    FillData fd;
    fd.set(xRepMin->value(), xRepMax->value(), yRepMin->value(), yRepMax->value());
    tiling->setFillData(fd);

    emit sig_viewWS();
    emit sig_tilingChanged();
}

void page_tiling_maker::slot_cellSelected(int row, int col)
{
    Q_UNUSED(col);
    PlacedFeaturePtr pfp = getFeatureRow(row);
    FeaturePtr       fp  = pfp->getFeature();
    QPolygonF      poly  = fp->getPoints();

    QString astring;
    for (int i=0; i < poly.size(); i++)
    {
        QPointF pt = poly.at(i);
        QString bstring = "[" + QString::number(pt.x(),'g',16) + "," + QString::number(pt.y(),'g',16) + "]  ";
        astring += bstring;
    }
    pointLabel.setText(astring);
}

TilingPtr page_tiling_maker::getSourceTiling()
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

void page_tiling_maker::currentFeature(int index)
{
    tileInfoTable->setFocus();
    tileInfoTable->selectRow(index);
}

void page_tiling_maker::slot_loadedXML(QString name)
{
    Q_UNUSED(name);
    onEnter();
}

void page_tiling_maker::slot_loadedTiling (QString name)
{
    Q_UNUSED(name);
    onEnter();
    if (chk_autoFill->isChecked())
    {
        designer->fillUsingTranslations();
    }
}

void page_tiling_maker::slot_hideTable(bool checked)
{
    hideTable = checked;
    tileInfoTable->setVisible(!checked);
}

void page_tiling_maker::slot_clearWS()
{
    TilingPtr tp;
    workspace->setTiling(tp);
    onEnter();
}
