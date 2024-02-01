#include <QComboBox>
#include <QCheckBox>
#include <QHeaderView>

#include "panels/page_mosaic_maker.h"
#include "geometry/map.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/mosaic_maker/style_editors.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "mosaic/mosaic.h"
#include "makers/prototype_maker/prototype.h"
#include "style/emboss.h"
#include "style/filled.h"
#include "style/interlace.h"
#include "style/plain.h"
#include "style/sketch.h"
#include "style/thick.h"
#include "style/tile_colors.h"
#include "tile/tiling.h"
#include "viewers/view_controller.h"
#include "widgets/layout_sliderset.h"
#include "panels/panel_misc.h"
#include "widgets/dlg_cleanse.h"

Q_DECLARE_METATYPE(WeakStylePtr)

typedef std::weak_ptr<Mosaic>          WeakMosaicPtr;

using std::make_shared;

page_mosaic_maker:: page_mosaic_maker(ControlPanel * apanel)  : panel_page(apanel,PAGE_MOSIAC_MAKER,"Mosaic Maker")
{
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
    vbox->addLayout(parmsLayout);
    vbox->addStretch();

    connect(delBtn,  &QPushButton::clicked, this, &page_mosaic_maker::slot_deleteStyle);
    connect(upBtn,   &QPushButton::clicked, this, &page_mosaic_maker::slot_moveStyleUp);
    connect(downBtn, &QPushButton::clicked, this, &page_mosaic_maker::slot_moveStyleDown);
    connect(dupBtn,  &QPushButton::clicked, this, &page_mosaic_maker::slot_duplicateStyle);
    connect(analyzeBtn,  &QPushButton::clicked, this, &page_mosaic_maker::slot_analyzeStyleMap);

    QItemSelectionModel * selectModel = styleTable->selectionModel();
    connect(selectModel, &QItemSelectionModel::selectionChanged, this, &page_mosaic_maker::slot_styleSelected);
}

void  page_mosaic_maker::onRefresh()
{
    static WeakMosaicPtr wmp;

    MosaicPtr mosaic = mosaicMaker->getMosaic();

    if (mosaic)
    {
        blockSignals(true);

        int xMin,xMax,yMin,yMax;
        bool singleton;
        const FillData & fd = mosaic->getCanvasSettings().getFillData();
        fd.get(singleton,xMin ,xMax,yMin,yMax);

        chkSingle->setChecked(singleton);
        if (!singleton)
        {
            xRepMin->setDisabled(false);
            xRepMax->setDisabled(false);
            yRepMin->setDisabled(false);
            yRepMax->setDisabled(false);

            xRepMin->setValue(xMin);
            xRepMax->setValue(xMax);
            yRepMin->setValue(yMin);
            yRepMax->setValue(yMax);
        }
        else
        {
            xRepMin->setValue(0);
            xRepMax->setValue(0);
            yRepMin->setValue(0);
            yRepMax->setValue(0);

            xRepMin->setDisabled(true);
            xRepMax->setDisabled(true);
            yRepMin->setDisabled(true);
            yRepMax->setDisabled(true);
        }

        blockSignals(false);

        uint level = mosaic->getCleanseLevel();
        cleanseLevel->setText(QString::number(level,16).toUpper());

        if (mosaic == wmp.lock())
        {
            int row = 0;
            const StyleSet & sset = mosaic->getStyleSet();
            for (auto & style : std::as_const(sset))
            {
                QTableWidgetItem * item = styleTable->item(row,STYLE_COL_TRANS);
                if (item)
                {
                    Xform xf = style->getModelXform();
                    item->setText(xf.toInfoString(8));
                }
                row++;
            }

            styleTable->resizeColumnsToContents();
            styleTable->adjustTableSize();
        }
        else
        {
            wmp = mosaic;
            reEnter();
        }
    }
}

void  page_mosaic_maker::onEnter()
{
    reEnter();
}

void  page_mosaic_maker::reEnter()
{
    blockPage(true);
    styleTable->clearContents();
    styleTable->setRowCount(0);
    blockPage(false);

    displayStyles();
    styleTable->adjustTableSize();
    styleTable->selectRow(0);

    displayStyleParams();
    parmsTable->adjustTableSize();
    parmsTable->selectRow(0);

    //styleTable->setFocus();  // 26FEB23 - this line causes linux crash
    parmsTable->setFocus();

    updateGeometry();

    //qDebug() << "row count   =" << styleTable->rowCount();
    //qDebug() << "current row =" << styleTable->currentRow();
}

void page_mosaic_maker::displayStyles()
{
    int row = 0;
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (mosaic && mosaic->hasContent())
    {
        blockPage(true);

        const QVector<TilingPtr> & tilings = tilingMaker->getTilings();  // this is tilings to choose from not tilings used

        const StyleSet & sset = mosaic->getStyleSet();
        for (auto & style : std::as_const(sset))
        {
            styleTable->setRowCount(row+1);

            // build structure
            QCheckBox * cbShow = new QCheckBox();
            styleTable->setCellWidget(row,STYLE_COL_CHECK_SHOW,cbShow);
            styleTable->setColumnWidth(STYLE_COL_CHECK_SHOW,25);

            cbShow->setChecked(style->isVisible());

            QComboBox * qcbStyle = new QComboBox();
            qcbStyle->setEditable(false);
            qcbStyle->setFrame(false);
            styleTable->setCellWidget(row,STYLE_COL_STYLE,qcbStyle);

            qcbStyle->addItem("Plain",STYLE_PLAIN);
            qcbStyle->addItem("Thick Lines",STYLE_THICK);
            qcbStyle->addItem("Filled",STYLE_FILLED);
            qcbStyle->addItem("Outlined",STYLE_OUTLINED);
            qcbStyle->addItem("Interlaced",STYLE_INTERLACED);
            qcbStyle->addItem("Embossed",STYLE_EMBOSSED);
            qcbStyle->addItem("Sketched",STYLE_SKETCHED);
            qcbStyle->addItem("Tile Colors",STYLE_TILECOLORS);

            QString stylename = style->getStyleDesc();
            int index = qcbStyle->findText(stylename);
            Q_ASSERT(index != -1);
            qcbStyle->setCurrentIndex(index);

            QComboBox * qcbTiling = new QComboBox();
            styleTable->setCellWidget(row,STYLE_COL_TILING,qcbTiling);

            for (auto & tiling : std::as_const(tilings))
            {
                qcbTiling->addItem(tiling->getTitle());
            }

            TilingPtr tp = style->getPrototype()->getTiling();
            if (tp)
            {
                int index = qcbTiling->findText(tp->getTitle());
                qcbTiling->setCurrentIndex(index);
            }

            QTableWidgetItem * addrtxt = new QTableWidgetItem(addr(style.get()));
            styleTable->setItem(row,STYLE_COL_ADDR,addrtxt);
            addrtxt->setData(Qt::UserRole,QVariant::fromValue(WeakStylePtr(style)));     // tiling name also stores Style address

            QTableWidgetItem * xftext = new QTableWidgetItem("Xform");
            styleTable->setItem(row,STYLE_COL_TRANS,xftext);
            
            Xform xf = style->getModelXform();
            xftext->setText(xf.toInfoString(8));

            // these three connects all pas the row not the index
            connect(qcbStyle,  QOverload<int>::of(&QComboBox::currentIndexChanged), this,  [this,row] { styleChanged(row); });
            connect(qcbTiling, QOverload<int>::of(&QComboBox::currentIndexChanged), this,  [this,row] { tilingChanged(row); });
            connect(cbShow,    &QCheckBox::toggled,                                 this,  [this,row] { styleVisibilityChanged(row); });

            row++;
        }

        blockPage(false);
    }

    styleTable->resizeColumnsToContents();
}

void  page_mosaic_maker::slot_styleSelected(const QItemSelection &selected, const QItemSelection &deselected)
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

void page_mosaic_maker::displayStyleParams()
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

void page_mosaic_maker::setCurrentEditor(StylePtr style)
{
    if (!style)
    {
        return;
    }

    if (currentStyleEditor)
    {
       currentStyleEditor->onExit();
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
        qCritical("unexpected style");
        break;
    }

    currentStyleEditor->onEnter();
}

void page_mosaic_maker::tilingChanged(int row)
{
    QComboBox * qcb = dynamic_cast<QComboBox*>(styleTable->cellWidget(row,STYLE_COL_TILING));
    QString name = qcb->currentText();
    TilingPtr tp = tilingMaker->findTilingByName(name);
    if (!tp)
        return;

    StylePtr style  = getStyleRow(row);
    ProtoPtr pp = style->getPrototype();
    pp->replaceTiling(tp);

    emit sig_refreshView();

    reEnter();
}

void page_mosaic_maker::styleVisibilityChanged(int row)
{
    qDebug() << "visibility changed: row=" << row;

    QCheckBox * cb = dynamic_cast<QCheckBox*>(styleTable->cellWidget(row,STYLE_COL_CHECK_SHOW));
    bool visible   = cb->isChecked();

    StylePtr style = getStyleRow(row);
    style->setVisible(visible);
    emit sig_refreshView();
}

void page_mosaic_maker::styleChanged(int row)
{
    qDebug() << "style changed: row=" << row;

    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        QComboBox * qcb = dynamic_cast<QComboBox*>(styleTable->cellWidget(row,STYLE_COL_STYLE));
        eStyleType esc  = static_cast<eStyleType>(qcb->currentData().toUInt());

        StylePtr oldStyle = getStyleRow(row);
        StylePtr newStyle = mosaicMaker->makeStyle(esc,oldStyle);

        mosaic->replaceStyle(oldStyle,newStyle);
    }

    emit sig_refreshView();
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
    MosaicPtr mosaic = mosaicMaker->getMosaic();
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

void page_mosaic_maker::slot_deleteStyle()
{
    int row = styleTable->currentRow();
    if (row == -1) return;

    StylePtr style = getStyleRow(row);

    MosaicPtr mosaic = mosaicMaker->getMosaic();
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

void page_mosaic_maker::slot_moveStyleUp()
{
    int row = styleTable->currentRow();
    if (row == -1) return;
    StylePtr style = getStyleRow(row);

    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if  (!mosaic) return;
    mosaic->moveUp(style);
    reEnter();

    emit sig_refreshView();

    styleTable->selectRow(row);
    styleTable->setFocus();
}

void  page_mosaic_maker::slot_moveStyleDown()
{
    int row = styleTable->currentRow();
    if (row == -1) return;
    StylePtr style = getStyleRow(row);

    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if  (!mosaic) return;
    mosaic->moveDown(style);

    reEnter();

    emit sig_refreshView();

    styleTable->selectRow(row);
    styleTable->setFocus();
}

void  page_mosaic_maker::slot_duplicateStyle()
{
    int row = styleTable->currentRow();
    if (row == -1) return;
    StylePtr style = getStyleRow(row);

    StylePtr style2 = copyStyle(style);

    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if  (!mosaic) return;
    mosaic->addStyle(style2);

    reEnter();

    emit sig_refreshView();

    styleTable->selectRow(row);
    styleTable->setFocus();
}

void  page_mosaic_maker::slot_analyzeStyleMap()
{
    int row = styleTable->currentRow();
    if (row == -1) return;
    qDebug() << "page_mosaic_maker::slot_analyzeStyleMap()" << row;
    StylePtr style = getStyleRow(row);
    //style->setStyleMap();
    MapPtr map = style->getProtoMap();
    if (map)
    {
        qDebug().noquote() << map->displayVertexEdgeCounts();
    }
}

StylePtr page_mosaic_maker::copyStyle(const StylePtr style)
{
    return mosaicMaker->makeStyle(style->getStyleType(),style);
}

QHBoxLayout * page_mosaic_maker::createFillDataRow()
{
    QHBoxLayout * hbox = new QHBoxLayout;

    const int rmin = -1000;
    const int rmax =  1000;

    QLabel * replabel = new QLabel("Fill:");

    chkSingle = new QCheckBox("Singleton");

    xRepMin = new SpinSet("xMin",0,rmin,rmax);
    xRepMax = new SpinSet("xMax",0,rmin,rmax);
    yRepMin = new SpinSet("yMin",0,rmin,rmax);
    yRepMax = new SpinSet("yMax",0,rmin,rmax);

    QPushButton * pbRender = new QPushButton("Render");
    pbRender->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    hbox->addWidget(chkSingle);
    hbox->addSpacing(3);
    hbox->addWidget(replabel);
    hbox->addSpacing(3);
    hbox->addLayout(xRepMin);
    hbox->addSpacing(3);
    hbox->addLayout(xRepMax);
    hbox->addSpacing(3);
    hbox->addLayout(yRepMin);
    hbox->addSpacing(3);
    hbox->addLayout(yRepMax);
    hbox->addSpacing(11);

    connect(chkSingle,&QCheckBox::clicked,   this, &page_mosaic_maker::singleton_changed);
    connect(xRepMin, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(xRepMax, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(yRepMin, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(yRepMax, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(pbRender,&QPushButton::clicked,  this, &panel_page::sig_render);

    QPushButton * pbCleanse = new QPushButton("Set cleanse");

    cleanseLevel = new QLineEdit();
    cleanseLevel->setFixedWidth(23);
    hbox->addSpacing(13);
    hbox->addWidget(pbRender);

    connect(pbCleanse,&QPushButton::clicked,  this, &page_mosaic_maker::slot_setCleanse);

    hbox->addStretch();
    hbox->addWidget(cleanseLevel);
    hbox->addWidget(pbCleanse);
    return hbox;
}

void page_mosaic_maker::slot_set_reps()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (!mosaic) return;

    FillData fd;
    fd.set(chkSingle->isChecked(),xRepMin->value(), xRepMax->value(), yRepMin->value(), yRepMax->value());

    CanvasSettings & cs = mosaic->getCanvasSettings();
    cs.setFillData(fd);

    viewControl->getCanvas().setFillData(fd);

    emit sig_render();
}

void page_mosaic_maker::slot_setCleanse()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (!mosaic) return;

    uint clevel = mosaic->getCleanseLevel();
    DlgCleanse dlg(mosaic,clevel,this);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    int level = dlg.getLevel();
    mosaic->setCleanseLevel(level);

    mosaic->resetProtoMaps();

    emit sig_render();
}


void page_mosaic_maker::singleton_changed(bool checked)
{
    FillData fd;
    if (!checked)
        fd.set(false,-3, 3, -3, 3);
    else
        fd.set(true,0,0,0,0);

    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        CanvasSettings & cs = mosaic->getCanvasSettings();
        cs.setFillData(fd);
    }

    viewControl->getCanvas().setFillData(fd);

    emit sig_render();
}
