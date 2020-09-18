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

#include "panels/page_mosaic_maker.h"
#include "base/configuration.h"
#include "base/tiledpatternmaker.h"
#include "designs/patterns.h"
#include "style/colored.h"
#include "style/thick.h"
#include "style/filled.h"
#include "style/interlace.h"
#include "style/outline.h"
#include "style/plain.h"
#include "style/sketch.h"
#include "style/emboss.h"
#include "viewers/workspace_viewer.h"

using std::string;

Q_DECLARE_METATYPE(WeakStylePtr)

page_mosaic_maker:: page_mosaic_maker(ControlPanel * apanel)  : panel_page(apanel,"Mosaic Maker")
{
    styleParms = nullptr;

    styleTable = new AQTableWidget(this);
    styleTable->setColumnCount(STYLE_COL_NUM_COLS);
    styleTable->setSelectionMode(QAbstractItemView::SingleSelection);
    styleTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList qslH;
    qslH << "" << "Tiling" << "Style" << "Addr" << "Transform";
    styleTable->setHorizontalHeaderLabels(qslH);
    styleTable->verticalHeader()->setVisible(false);


    delBtn  = new QPushButton("Delete");
    upBtn   = new QPushButton("Move Up");
    downBtn = new QPushButton("MoveDown");
    dupBtn  = new QPushButton ("Duplicate");
    analyzeBtn  = new QPushButton ("Analyze");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(delBtn);
    hbox->addWidget(upBtn);
    hbox->addWidget(downBtn);
    hbox->addWidget(dupBtn);
    hbox->addWidget(analyzeBtn);

    QGroupBox * fillBox = createFillDataRow();

    vbox->addWidget(fillBox);
    vbox->addSpacing(3);
    vbox->addWidget(styleTable);
    vbox->addLayout(hbox);
    vbox->addSpacing(5);

    parmsTable = new AQTableWidget(this);

    parmsCtrl = new QVBoxLayout;

    vbox->addWidget(parmsTable);
    vbox->addSpacing(7);
    vbox->addLayout(parmsCtrl);

    connect(&styleMapper,     SIGNAL(mapped(int)), this, SLOT(slot_styleChanged(int)));
    connect(&tilingMapper,    SIGNAL(mapped(int)), this, SLOT(slot_tilingChanged(int)));
    connect(&styleVisMapper,  SIGNAL(mapped(int)), this, SLOT(slot_styleVisibilityChanged(int)));

    connect(delBtn,  &QPushButton::clicked, this, &page_mosaic_maker::slot_deleteStyle);
    connect(upBtn,   &QPushButton::clicked, this, &page_mosaic_maker::slot_moveStyleUp);
    connect(downBtn, &QPushButton::clicked, this, &page_mosaic_maker::slot_moveStyleDown);
    connect(dupBtn,  &QPushButton::clicked, this, &page_mosaic_maker::slot_duplicateStyle);
    connect(analyzeBtn,  &QPushButton::clicked, this, &page_mosaic_maker::slot_analyzeStyleMap);

    selectModel = styleTable->selectionModel();
    connect(selectModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slot_styleSelected()));

    connect(tpm,  &TiledPatternMaker::sig_loadedTiling,   this,   &page_mosaic_maker::slot_loadedTiling);
    connect(tpm,  &TiledPatternMaker::sig_loadedXML,      this,   &page_mosaic_maker::slot_loadedXML);
    connect(workspace, &View::sig_unload,                 this,   &page_mosaic_maker::slot_unload);
}

void  page_mosaic_maker::refreshPage()
{
    int row = 0;
    MosaicPtr mosaic = workspace->getMosaic();

    int xMin,xMax,yMin,yMax;
    const FillData & fd = mosaic->getSettings().getFillData();
    fd.get(xMin ,xMax,yMin,yMax);
    xRepMin->setValue(xMin);
    xRepMax->setValue(xMax);
    yRepMin->setValue(yMin);
    yRepMax->setValue(yMax);

    const StyleSet & sset = mosaic->getStyleSet();
    for (auto style : sset)
    {
        QTableWidgetItem * item = styleTable->item(row,STYLE_COL_TRANS);
        if (item)
        {
            Xform xf = style->getCanvasXform();
            item->setText(xf.toInfoString());
        }
        row++;
    }
    styleTable->resizeColumnsToContents();
    styleTable->adjustTableSize();
}

void  page_mosaic_maker::onEnter()
{
    reEnter();
}

void  page_mosaic_maker::reEnter()
{
    styleTable->clearContents();

    int row = 0;
    MosaicPtr mosaic = workspace->getMosaic();

    UniqueQVector<TilingPtr>tilings = mosaic->getTilings();

    const StyleSet & sset = mosaic->getStyleSet();
    for (auto style : sset)
    {
        styleTable->setRowCount(row+1);

        // build structure
        QCheckBox * cb = new QCheckBox();
        styleTable->setCellWidget(row,STYLE_COL_CHECK_SHOW,cb);
        styleTable->setColumnWidth(STYLE_COL_CHECK_SHOW,25);
        cb->setChecked(style->isVisible());

        QComboBox * qcb = new QComboBox();
        qcb->setEditable(false);
        qcb->setFrame(false);

        qcb->addItem("Plain",STYLE_PLAIN);
        qcb->addItem("Thick Lines",STYLE_THICK);
        qcb->addItem("Filled",STYLE_FILLED);
        qcb->addItem("Outlined",STYLE_OUTLINED);
        qcb->addItem("Interlaced",STYLE_INTERLACED);
        qcb->addItem("Embossed",STYLE_EMBOSSED);
        qcb->addItem("Sketched",STYLE_SKETCHED);
        qcb->addItem("Tile Colors",STYLE_TILECOLORS);
        styleTable->setCellWidget(row,STYLE_COL_STYLE,qcb);

        QComboBox * qcb2 = new QComboBox();
        for (auto tiling : tilings)
        {
            qcb2->addItem(tiling->getName());
        }
        qcb2->addItem("New T");
        int index = qcb2->findText(style->getPrototype()->getTiling()->getName());
        qcb2->setCurrentIndex(index);
        styleTable->setCellWidget(row,STYLE_COL_TILING,qcb2);

        QTableWidgetItem * item = new QTableWidgetItem(addr(style.get()));
        item->setData(Qt::UserRole,QVariant::fromValue(WeakStylePtr(style)));     // tiling name also stores Style address
        styleTable->setItem(row,STYLE_COL_ADDR,item);

        item = new QTableWidgetItem("Xform");
        styleTable->setItem(row,STYLE_COL_TRANS,item);

        QString stylename = style->getStyleDesc();
        index = qcb->findText(stylename);
        Q_ASSERT(index != -1);
        qcb->setCurrentIndex(index);

        connect(qcb, SIGNAL(currentIndexChanged(int)), &styleMapper, SLOT(map()),Qt::UniqueConnection);
        styleMapper.setMapping(qcb,row);

        connect(qcb2, SIGNAL(currentIndexChanged(int)), &tilingMapper, SLOT(map()),Qt::UniqueConnection);
        tilingMapper.setMapping(qcb2,row);

        connect(cb, SIGNAL(toggled(bool)), &styleVisMapper, SLOT(map()),Qt::UniqueConnection);
        styleVisMapper.setMapping(cb,row);

        row++;
    }

    styleTable->resizeColumnsToContents();
    styleTable->adjustTableSize();
    updateGeometry();

    styleTable->selectRow(0);
    styleTable->setFocus();
}

void  page_mosaic_maker::slot_styleSelected()
{
    qDebug() << "page_style_maker::slot_styleSelected";

    int row = styleTable->currentRow();
    if (row == -1) return;

    qDebug() << "page_style_maker row=" << row;

    StylePtr style = getStyleIndex(row);
    if (styleParms && styleParms->style == style)
    {
        return;
    }
    if (styleParms != nullptr )
    {
        delete styleParms;
        styleParms = nullptr;
    }

    setupStyleParms(style,parmsTable);

    PrototypePtr pp = style->getPrototype();
    workspace->setSelectedPrototype(pp);

    emit sig_viewWS();
}

void page_mosaic_maker::slot_tilingChanged(int row)
{
    QComboBox * qcb = dynamic_cast<QComboBox*>(styleTable->cellWidget(row,STYLE_COL_TILING));
    QString name = qcb->currentText();
    TilingPtr tp;
    if (name == "New T")
    {
        tp = loadNewTiling(name);
    }
    else
    {
        tp = workspace->findTiling(name);
    }
    if (!tp)
    {
        return;
    }

    StylePtr sp = getStyleRow(row);
    PrototypePtr pp = sp->getPrototype();
    MosaicPtr mosaic = workspace->getMosaic();
    mosaic->replaceTiling(pp,tp);

    emit sig_viewWS();

    reEnter();
}

void page_mosaic_maker::setupStyleParms(StylePtr style, AQTableWidget * table)
{
    eraseLayout(dynamic_cast<QLayout*>(parmsCtrl));

    switch (style->getStyleType())
    {
    case STYLE_PLAIN:
        styleParms = new ColoredEditor(dynamic_cast<Plain*>(style.get()),table);
        break;
    case STYLE_THICK:
        styleParms = new ThickEditor(dynamic_cast<Thick*>(style.get()),table);
        break;
    case STYLE_FILLED:
        styleParms = new FilledEditor(dynamic_cast<Filled*>(style.get()),table,parmsCtrl);
        break;
    case STYLE_EMBOSSED:
        styleParms = new EmbossEditor(dynamic_cast<Emboss*>(style.get()),table);
        break;
    case STYLE_INTERLACED:
        styleParms = new InterlaceEditor(dynamic_cast<Interlace*>(style.get()),table);
        break;
    case STYLE_OUTLINED:
        styleParms = new ThickEditor(dynamic_cast<Outline*>(style.get()),table);
        break;
    case STYLE_SKETCHED:
        styleParms = new ColoredEditor(dynamic_cast<Sketch*>(style.get()),table);
        break;
    case STYLE_TILECOLORS:
    {
        TilingPtr tiling = workspace->getCurrentTiling();
        if (tiling)
        {
            styleParms = new TileColorsEditor(dynamic_cast<TileColors*>(style.get()),table,tiling);
        }
        break;
    }
    case STYLE_STYLE:
        break;
    }
    table->resizeColumnsToContents();
    table->adjustTableSize();
    updateGeometry();
}

void page_mosaic_maker::slot_styleVisibilityChanged(int row)
{
    qDebug() << "visibility changed: row=" << row;

    QCheckBox * cb = dynamic_cast<QCheckBox*>(styleTable->cellWidget(row,STYLE_COL_CHECK_SHOW));
    bool visible   = cb->isChecked();

    StylePtr style = getStyleRow(row);
    style->setVisible(visible);
    emit sig_viewWS();
}

void page_mosaic_maker::slot_styleChanged(int row)
{
    qDebug() << "style changed: row=" << row;

    QComboBox * qcb = dynamic_cast<QComboBox*>(styleTable->cellWidget(row,STYLE_COL_STYLE));
    eStyleType esc  = static_cast<eStyleType>(qcb->currentData().toUInt());

    StylePtr oldStylePtr = getStyleRow(row);
    Style & oldStyle     = *oldStylePtr.get();
    Style * newStyle     = nullptr;
    switch (esc)
    {
    case STYLE_PLAIN:
        newStyle = new Plain(oldStyle);
        break;
    case STYLE_THICK:
        newStyle = new Thick(oldStyle);
        break;
    case STYLE_OUTLINED:
        newStyle = new Outline(oldStyle);
        break;
    case STYLE_INTERLACED:
        newStyle = new Interlace(oldStyle);
        break;
    case STYLE_EMBOSSED:
        newStyle = new Emboss(oldStyle);
        break;
    case STYLE_SKETCHED:
        newStyle = new Sketch(oldStyle);
        break;
    case STYLE_FILLED:
        newStyle = new Filled(oldStyle);
        break;
    case STYLE_TILECOLORS:
        newStyle= new TileColors(oldStyle);
        break;
    case STYLE_STYLE:
        Q_ASSERT(false);
        break;
    }

    MosaicPtr mosaic = workspace->getMosaic();
    mosaic->replaceStyle(oldStylePtr,StylePtr(newStyle));

    emit sig_viewWS();
    reEnter();
    styleTable->selectRow(row);
    styleTable->setFocus();
}

StylePtr page_mosaic_maker::getStyleRow(int row)
{
    StylePtr sp;

    QTableWidgetItem * twi = styleTable->item(row,STYLE_COL_STYLE_DATA);
    QVariant var = twi->data(Qt::UserRole);

    if (var.canConvert<WeakStylePtr>())
    {
        WeakStylePtr wsp = var.value<WeakStylePtr>();
        sp = wsp.lock();
        Q_ASSERT(sp);
    }
    return sp;
}

StylePtr page_mosaic_maker::getStyleIndex(int index)
{
    StylePtr sp;
    MosaicPtr mosaic = workspace->getMosaic();
    const StyleSet & sset = mosaic->getStyleSet();
    if (index < sset.size())
    {
        sp = sset[index];
    }
    return sp;
}

void page_mosaic_maker::slot_deleteStyle()
{
    int row = styleTable->currentRow();
    if (row == -1) return;

    StylePtr style = getStyleRow(row);

    MosaicPtr mosaic = workspace->getMosaic();
    mosaic->deleteStyle(style);

    if (row > 0)
        row--;

    reEnter();

    emit sig_viewWS();

    styleTable->selectRow(row);
    styleTable->setFocus();
}

void page_mosaic_maker::slot_moveStyleUp()
{
    int row = styleTable->currentRow();
    if (row == -1) return;
    StylePtr style = getStyleRow(row);

    MosaicPtr mosaic = workspace->getMosaic();
    mosaic->moveUp(style);
    reEnter();

    emit sig_viewWS();

    styleTable->selectRow(row);
    styleTable->setFocus();
}

void  page_mosaic_maker::slot_moveStyleDown()
{
    int row = styleTable->currentRow();
    if (row == -1) return;
    StylePtr style = getStyleRow(row);

    MosaicPtr mosaic = workspace->getMosaic();
    mosaic->moveDown(style);

    reEnter();

    emit sig_viewWS();

    styleTable->selectRow(row);
    styleTable->setFocus();
}

void  page_mosaic_maker::slot_duplicateStyle()
{
    int row = styleTable->currentRow();
    if (row == -1) return;
    StylePtr style = getStyleRow(row);

    StylePtr style2 = copyStyle(style);

    MosaicPtr mosaic = workspace->getMosaic();
    mosaic->addStyle(style2);

    reEnter();

    emit sig_viewWS();

    styleTable->selectRow(row);
    styleTable->setFocus();
}

void  page_mosaic_maker::slot_analyzeStyleMap()
{
    int row = styleTable->currentRow();
    if (row == -1) return;

    StylePtr style = getStyleRow(row);
    //style->setStyleMap();
    MapPtr map = style->getMap();
    if (map)
    {
        if (map->analyzeVertices())
        {
            emit sig_viewWS();
        }
    }
}

StylePtr page_mosaic_maker::copyStyle(const StylePtr style)
{
    Style & s = *style.get();
    Style *  newStyle = nullptr;
    switch (style->getStyleType())
    {
    case STYLE_FILLED:
        newStyle =  new Filled(s);
        break;
    case STYLE_EMBOSSED:
        newStyle =  new Emboss(s);
        break;
    case STYLE_INTERLACED:
        newStyle = new Interlace(s);
        break;
    case STYLE_OUTLINED:
        newStyle = new Outline(s);
        break;
    case STYLE_PLAIN:
        newStyle = new Plain(s);
        break;
    case STYLE_SKETCHED:
        newStyle = new Sketch(s);
        break;
    case STYLE_THICK:
        newStyle = new Thick(s);
        break;
    case STYLE_TILECOLORS:
        newStyle = new TileColors(s);
        break;
    case STYLE_STYLE:
        Q_ASSERT(false);
        break;
    }
    return StylePtr(newStyle);
}

void  page_mosaic_maker::slot_loadedXML(QString name)
{
    Q_UNUSED(name)
    reEnter();
}

void page_mosaic_maker::slot_loadedTiling (QString name)
{
    Q_UNUSED(name)
    reEnter();
}

void page_mosaic_maker::slot_unload()
{
    styleTable->clearContents();
    if (styleParms)
    {
        styleParms->style.reset();
    }
}


TilingPtr page_mosaic_maker::loadNewTiling(QString name)
{
    TilingPtr tp;

    // FIXME now - what is this all about !!!!

    return  tp;
}


QGroupBox * page_mosaic_maker::createFillDataRow()
{
    QHBoxLayout * hbox = new QHBoxLayout;

    const int rmin = -1000;
    const int rmax =  1000;

    xRepMin = new SpinSet("xMin",0,rmin,rmax);
    xRepMax = new SpinSet("xMax",0,rmin,rmax);
    yRepMin = new SpinSet("yMin",0,rmin,rmax);
    yRepMax = new SpinSet("yMax",0,rmin,rmax);

    hbox->addLayout(xRepMin);
    hbox->addSpacing(7);
    hbox->addLayout(xRepMax);
    hbox->addSpacing(7);
    hbox->addLayout(yRepMin);
    hbox->addSpacing(7);
    hbox->addLayout(yRepMax);
    hbox->addStretch();

    connect(xRepMin, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(xRepMax, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(yRepMin, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(yRepMax, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);

    QGroupBox * gb = new  QGroupBox("Repeats");
    gb->setLayout(hbox);

    return gb;
}

void page_mosaic_maker::slot_set_reps()
{
    MosaicPtr mosaic = workspace->getMosaic();
    Q_ASSERT(mosaic);

    FillData fd;
    fd.set(xRepMin->value(), xRepMax->value(), yRepMin->value(), yRepMax->value());

    mosaic->getSettings().setFillData(fd);
    workspace->setFillData(fd);

    emit sig_render();
}
