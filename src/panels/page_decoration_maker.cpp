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

#include "panels/page_decoration_maker.h"
#include "base/mosaic.h"
#include "style/thick.h"
#include "style/filled.h"
#include "style/interlace.h"
#include "style/plain.h"
#include "style/sketch.h"
#include "style/emboss.h"
#include "makers/decoration_maker/style_editors.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/decoration_maker/decoration_maker.h"
#include "viewers/viewcontrol.h"
#include "panels/layout_sliderset.h"
#include "settings/model_settings.h"
#include "tile/tiling.h"
#include "tapp/prototype.h"
#include "style/tile_colors.h"
#include "geometry/map.h"
#include "panels/panel_misc.h"

Q_DECLARE_METATYPE(WeakStylePtr)

typedef std::weak_ptr<Mosaic>          WeakMosaicPtr;

using std::make_shared;

page_decoration_maker:: page_decoration_maker(ControlPanel * apanel)  : panel_page(apanel,"Decoration Maker")
{
    decorationMaker = DecorationMaker::getInstance();

    styleTable = new AQTableWidget(this);
    styleTable->setColumnCount(STYLE_COL_NUM_COLS);
    styleTable->setSelectionMode(QAbstractItemView::SingleSelection);
    styleTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    styleTable->setFocusPolicy(Qt::StrongFocus);

    QStringList qslH;
    qslH << "" << "Tiling" << "Style type" << "Style" << "Transform";
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

    QHBoxLayout * fillBox = createFillDataRow();

    parmsTable = new AQTableWidget(this);

    parmsLayout = new QVBoxLayout;

    vbox->addLayout(fillBox);
    vbox->addSpacing(3);
    vbox->addWidget(styleTable);
    vbox->addSpacing(3);
    vbox->addLayout(hbox);
    vbox->addSpacing(5);
    vbox->addWidget(parmsTable);
    vbox->addSpacing(7);
    vbox->addLayout(parmsLayout);


    connect(delBtn,  &QPushButton::clicked, this, &page_decoration_maker::slot_deleteStyle);
    connect(upBtn,   &QPushButton::clicked, this, &page_decoration_maker::slot_moveStyleUp);
    connect(downBtn, &QPushButton::clicked, this, &page_decoration_maker::slot_moveStyleDown);
    connect(dupBtn,  &QPushButton::clicked, this, &page_decoration_maker::slot_duplicateStyle);
    connect(analyzeBtn,  &QPushButton::clicked, this, &page_decoration_maker::slot_analyzeStyleMap);

    QItemSelectionModel * selectModel = styleTable->selectionModel();
    connect(selectModel, &QItemSelectionModel::selectionChanged, this, &page_decoration_maker::slot_styleSelected);
}

void  page_decoration_maker::refreshPage()
{
    static WeakMosaicPtr wmp;
    static int styleRows = 0;

    MosaicPtr mosaic = decorationMaker->getMosaic();
    if (mosaic != wmp.lock())
    {
        wmp = mosaic;
        reEnter();
    }

    if (mosaic)
    {
        int xMin,xMax,yMin,yMax;
        const FillData & fd = mosaic->getSettings().getFillData();
        fd.get(xMin ,xMax,yMin,yMax);
        xRepMin->setValue(xMin);
        xRepMax->setValue(xMax);
        yRepMin->setValue(yMin);
        yRepMax->setValue(yMax);

        int row = 0;
        const StyleSet & sset = mosaic->getStyleSet();
        for (auto& style : sset)
        {
            QTableWidgetItem * item = styleTable->item(row,STYLE_COL_TRANS);
            if (item)
            {
                Xform xf = style->getCanvasXform();
                item->setText(xf.toInfoString());
            }
            row++;
        }
        if (row != styleRows)
        {
            styleRows = row;
            styleTable->resizeColumnsToContents();
            styleTable->adjustTableSize();
        }
    }
}

void  page_decoration_maker::onEnter()
{
    reEnter();
    styleTable->selectRow(0);
    parmsTable->selectRow(0);
    parmsTable->setFocus();
}

void  page_decoration_maker::reEnter()
{
    blockPage(true);
    styleTable->clearContents();
    styleTable->setRowCount(0);
    blockPage(false);

    displayStyles();
    styleTable->adjustTableSize();

    displayStyleParams();
    parmsTable->adjustTableSize();

    styleTable->setFocus();
    parmsTable->setFocus();

    updateGeometry();

    qDebug() << "row count   =" << styleTable->rowCount();
    qDebug() << "current row =" << styleTable->currentRow();
}

void page_decoration_maker::displayStyles()
{
    int row = 0;
    MosaicPtr mosaic = decorationMaker->getMosaic();
    if (mosaic && mosaic->hasContent())
    {
        blockPage(true);

        const QVector<TilingPtr> & tilings = tilingMaker->getTilings();  // this is tilings to choose from not tilings used

        const StyleSet & sset = mosaic->getStyleSet();
        for (auto& style : sset)
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
            for (auto& tiling : tilings)
            {
                qcb2->addItem(tiling->getName());
            }
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

            connect(qcb, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this,row] { page_decoration_maker::styleChanged(row); });
            connect(qcb2,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this,row] { page_decoration_maker::tilingChanged(row); });
            connect(cb,  &QCheckBox::toggled,             [this,row] { page_decoration_maker::styleVisibilityChanged(row); });

            row++;
        }

        blockPage(false);
    }

    styleTable->resizeColumnsToContents();
}

void  page_decoration_maker::slot_styleSelected(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    Q_UNUSED(selected);

    if (pageBlocked())
    {
        return;
    }

    qDebug() << "page_style_maker::slot_styleSelected";

    displayStyleParams();
}

void page_decoration_maker::displayStyleParams()
{
    static StylePtr selectedStyle;

    int row = styleTable->currentRow();  // can be -1
    qDebug() << "displayStyleParams row =" << row;

    StylePtr style = getStyleIndex(row);
    if (style == selectedStyle)
    {
        // no need to redisplay
        return;
    }
    // the style can be null - it's handled

    parmsTable->clear();
    eraseLayout(dynamic_cast<QLayout*>(parmsLayout));

    setCurrentEditor(style);

    updateGeometry();

    parmsTable->resizeColumnsToContents();
    parmsTable->adjustTableSize();
}

void page_decoration_maker::setCurrentEditor(StylePtr style)
{
    if (!style)
    {
        return;
    }

    switch (style->getStyleType())
    {
    case STYLE_PLAIN:
        currentStyleEditor = make_shared<ColoredEditor>(dynamic_cast<Plain*>(style.get()),parmsTable);
        break;
    case STYLE_THICK:
        currentStyleEditor = make_shared<ThickEditor>(dynamic_cast<Thick*>(style.get()),parmsTable);
        break;
    case STYLE_FILLED:
        currentStyleEditor = make_shared<FilledEditor>(std::dynamic_pointer_cast<Filled>(style),parmsTable,parmsLayout);
        break;
    case STYLE_EMBOSSED:
        currentStyleEditor = make_shared<EmbossEditor>(dynamic_cast<Emboss*>(style.get()),parmsTable);
        break;
    case STYLE_INTERLACED:
        currentStyleEditor = make_shared<InterlaceEditor>(dynamic_cast<Interlace*>(style.get()),parmsTable);
        break;
    case STYLE_OUTLINED:
        currentStyleEditor = make_shared<ThickEditor>(dynamic_cast<Outline*>(style.get()),parmsTable);
        break;
    case STYLE_SKETCHED:
        currentStyleEditor = make_shared<ColoredEditor>(dynamic_cast<Sketch*>(style.get()),parmsTable);
        break;
    case STYLE_TILECOLORS:
        currentStyleEditor = make_shared<TileColorsEditor>(dynamic_cast<TileColors*>(style.get()),parmsTable,style->getTiling());
        break;
    case STYLE_STYLE:
        break;
    }
}

void page_decoration_maker::tilingChanged(int row)
{
    QComboBox * qcb = dynamic_cast<QComboBox*>(styleTable->cellWidget(row,STYLE_COL_TILING));
    QString name = qcb->currentText();
    TilingPtr tp = tilingMaker->findTilingByName(name);
    if (!tp)
        return;

    StylePtr style  = getStyleRow(row);
    PrototypePtr pp = style->getPrototype();
    pp->setTiling(tp);

    emit sig_refreshView();

    reEnter();
}

void page_decoration_maker::styleVisibilityChanged(int row)
{
    qDebug() << "visibility changed: row=" << row;

    QCheckBox * cb = dynamic_cast<QCheckBox*>(styleTable->cellWidget(row,STYLE_COL_CHECK_SHOW));
    bool visible   = cb->isChecked();

    StylePtr style = getStyleRow(row);
    style->setVisible(visible);
    emit sig_refreshView();
}

void page_decoration_maker::styleChanged(int row)
{
    qDebug() << "style changed: row=" << row;

    MosaicPtr mosaic = decorationMaker->getMosaic();
    if (mosaic)
    {
        QComboBox * qcb = dynamic_cast<QComboBox*>(styleTable->cellWidget(row,STYLE_COL_STYLE));
        eStyleType esc  = static_cast<eStyleType>(qcb->currentData().toUInt());

        StylePtr oldStyle = getStyleRow(row);
        StylePtr newStyle = decorationMaker->makeStyle(esc,oldStyle);

        mosaic->replaceStyle(oldStyle,newStyle);
    }

    emit sig_refreshView();
    reEnter();
    styleTable->selectRow(row);
    styleTable->setFocus();
}

StylePtr page_decoration_maker::getStyleRow(int row)
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

StylePtr page_decoration_maker::getStyleIndex(int index)
{
    StylePtr sp;
    MosaicPtr mosaic = decorationMaker->getMosaic();
    if (mosaic)
    {
        const StyleSet & sset = mosaic->getStyleSet();
        if ( index >= 0 && index < sset.size())
        {
            sp = sset[index];
        }
    }
    return sp;
}

void page_decoration_maker::slot_deleteStyle()
{
    int row = styleTable->currentRow();
    if (row == -1) return;

    StylePtr style = getStyleRow(row);

    MosaicPtr mosaic = decorationMaker->getMosaic();
    if  (!mosaic) return;
    mosaic->deleteStyle(style);

    if (row > 0)
        row--;

    reEnter();

    emit sig_refreshView();

    if (styleTable->rowCount() > 0)
    {
        styleTable->selectRow(row);
        styleTable->setFocus();
    }
    else
    {
        displayStyleParams();
    }
}

void page_decoration_maker::slot_moveStyleUp()
{
    int row = styleTable->currentRow();
    if (row == -1) return;
    StylePtr style = getStyleRow(row);

    MosaicPtr mosaic = decorationMaker->getMosaic();
    if  (!mosaic) return;
    mosaic->moveUp(style);
    reEnter();

    emit sig_refreshView();

    styleTable->selectRow(row);
    styleTable->setFocus();
}

void  page_decoration_maker::slot_moveStyleDown()
{
    int row = styleTable->currentRow();
    if (row == -1) return;
    StylePtr style = getStyleRow(row);

    MosaicPtr mosaic = decorationMaker->getMosaic();
    if  (!mosaic) return;
    mosaic->moveDown(style);

    reEnter();

    emit sig_refreshView();

    styleTable->selectRow(row);
    styleTable->setFocus();
}

void  page_decoration_maker::slot_duplicateStyle()
{
    int row = styleTable->currentRow();
    if (row == -1) return;
    StylePtr style = getStyleRow(row);

    StylePtr style2 = copyStyle(style);

    MosaicPtr mosaic = decorationMaker->getMosaic();
    if  (!mosaic) return;
    mosaic->addStyle(style2);

    reEnter();

    emit sig_refreshView();

    styleTable->selectRow(row);
    styleTable->setFocus();
}

void  page_decoration_maker::slot_analyzeStyleMap()
{
    int row = styleTable->currentRow();
    if (row == -1) return;
    qDebug() << "page_decoration_maker::slot_analyzeStyleMap()" << row;
    StylePtr style = getStyleRow(row);
    //style->setStyleMap();
    MapPtr map = style->getMap();
    if (map)
    {
        qDebug().noquote() << map->displayVertexEdgeCounts();
    }
}

StylePtr page_decoration_maker::copyStyle(const StylePtr style)
{
    return decorationMaker->makeStyle(style->getStyleType(),style);
}

QHBoxLayout * page_decoration_maker::createFillDataRow()
{
    QHBoxLayout * hbox = new QHBoxLayout;

    const int rmin = -1000;
    const int rmax =  1000;

    QLabel * replabel = new QLabel("Repititons:");

    xRepMin = new SpinSet("xMin",0,rmin,rmax);
    xRepMax = new SpinSet("xMax",0,rmin,rmax);
    yRepMin = new SpinSet("yMin",0,rmin,rmax);
    yRepMax = new SpinSet("yMax",0,rmin,rmax);

    QPushButton * pbRender = new QPushButton("Render");
    pbRender->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    hbox->addWidget(pbRender);
    hbox->addSpacing(13);
    hbox->addWidget(replabel);
    hbox->addSpacing(3);
    hbox->addLayout(xRepMin);
    hbox->addSpacing(3);
    hbox->addLayout(xRepMax);
    hbox->addSpacing(3);
    hbox->addLayout(yRepMin);
    hbox->addSpacing(3);
    hbox->addLayout(yRepMax);

    connect(xRepMin, &SpinSet::valueChanged, this, &page_decoration_maker::slot_set_reps);
    connect(xRepMax, &SpinSet::valueChanged, this, &page_decoration_maker::slot_set_reps);
    connect(yRepMin, &SpinSet::valueChanged, this, &page_decoration_maker::slot_set_reps);
    connect(yRepMax, &SpinSet::valueChanged, this, &page_decoration_maker::slot_set_reps);
    connect(pbRender,&QPushButton::clicked,  this, &panel_page::sig_render);

    return hbox;
}

void page_decoration_maker::slot_set_reps()
{
    MosaicPtr mosaic = decorationMaker->getMosaic();
    if (!mosaic) return;

    FillData fd;
    fd.set(xRepMin->value(), xRepMax->value(), yRepMin->value(), yRepMax->value());

    mosaic->getSettings().setFillData(fd);
    vcontrol->setFillData(fd);

    emit sig_render();
}
