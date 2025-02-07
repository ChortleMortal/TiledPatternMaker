#include <QComboBox>
#include <QCheckBox>
#include <QHeaderView>

#include "gui/panels/page_mosaic_maker.h"
#include "sys/geometry/map.h"
#include "model/makers/mosaic_maker.h"
#include "gui/model_editors/style_edit/style_editors.h"
#include "model/prototypes/prototype.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "gui/panels/panel_misc.h"
#include "model/styles/style.h"
#include "model/tilings/tiling.h"
#include "gui/top/view_controller.h"
#include "gui/widgets/dlg_cleanse.h"
#include "gui/widgets/layout_sliderset.h"

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
    hbox->addStretch();

    QHBoxLayout * fillBox = createFillDataRow();

    lowerWidget = new QWidget();
    emptyWidget = new StyleEditor();
    QVBoxLayout * evbox = new QVBoxLayout();
    evbox->addWidget(emptyWidget);
    lowerWidget->setLayout(evbox);

    currentEditor = emptyWidget;

    QVBoxLayout * wvbox = new AQVBoxLayout;
    wvbox->addLayout(fillBox);
    wvbox->addSpacing(3);
    wvbox->addWidget(styleTable);
    wvbox->addSpacing(3);
    wvbox->addLayout(hbox);
    wvbox->addSpacing(5);
    wvbox->addWidget(lowerWidget);
    wvbox->addStretch();

    QWidget * w = new QWidget;
    w->setContentsMargins(0,0,0,0);
    w->setFixedWidth(PANEL_RHS_WIDTH-40);
    w->setLayout(wvbox);

    vbox->addWidget(w);

    connect(delBtn,  &QPushButton::clicked, this, &page_mosaic_maker::slot_deleteStyle);
    connect(upBtn,   &QPushButton::clicked, this, &page_mosaic_maker::slot_moveStyleUp);
    connect(downBtn, &QPushButton::clicked, this, &page_mosaic_maker::slot_moveStyleDown);
    connect(dupBtn,  &QPushButton::clicked, this, &page_mosaic_maker::slot_duplicateStyle);
    connect(analyzeBtn,  &QPushButton::clicked, this, &page_mosaic_maker::slot_analyzeStyleMap);

    QItemSelectionModel * selectModel = styleTable->selectionModel();
    connect(selectModel, &QItemSelectionModel::selectionChanged, this, &page_mosaic_maker::slot_styleSelected);
}

QHBoxLayout * page_mosaic_maker::createFillDataRow()
{
    const int rmin = -99;
    const int rmax =  99;

    chkSingle = new QCheckBox("Singleton");

    xRepMin = new SpinSet("xMin",0,rmin,rmax);
    xRepMax = new SpinSet("xMax",0,rmin,rmax);
    yRepMin = new SpinSet("yMin",0,rmin,rmax);
    yRepMax = new SpinSet("yMax",0,rmin,rmax);

    QPushButton * pbRender = new QPushButton("Render");
    pbRender->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    pbClean       = new QPushButton("Cleanse");
    cleanseStatus = new QLabel();

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(chkSingle);
    hbox->addSpacing(3);
    hbox->addLayout(xRepMin);
    hbox->addLayout(xRepMax);
    hbox->addLayout(yRepMin);
    hbox->addLayout(yRepMax);
    hbox->addSpacing(9);
    hbox->addWidget(pbRender);
    hbox->addSpacing(9);
    hbox->addWidget(pbClean);
    hbox->addWidget(cleanseStatus);
    hbox->addStretch();

    connect(chkSingle,&QCheckBox::clicked,   this, &page_mosaic_maker::singleton_changed);
    connect(xRepMin, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(xRepMax, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(yRepMin, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(yRepMax, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(pbRender,&QPushButton::clicked,  this, &panel_page::sig_render);
    connect(pbClean, &QPushButton::clicked,  this, &page_mosaic_maker::slot_setCleanse);

    return hbox;
}
void  page_mosaic_maker::onRefresh()
{
    static WeakMosaicPtr wmp;

    MosaicPtr mosaic = mosaicMaker->getMosaic();

    if (!mosaic)
        return;

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
    if (level > 0)
    {
        pbClean->setText("Cleanse : ON ");
        qreal sensitivity = mosaic->getCleanseSensitivity();
        QString str = QString("0x%1 : %2").arg(QString::number(level,16)).arg(sensitivity);
        cleanseStatus->setText(str);
    }
    else
    {
        pbClean->setText("Cleanse : OFF");
        cleanseStatus->setText(" ");
    }

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
                item->setText(xf.info(8));
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

    if (currentEditor)
        currentEditor->onRefresh();
}

void  page_mosaic_maker::onEnter()
{
    reEnter();
}

void  page_mosaic_maker::reEnter()
{
    int sel = styleTable->currentRow();

    blockPage(true);
    styleTable->clearContents();
    styleTable->setRowCount(0);
    blockPage(false);

    displayStyles();
    styleTable->adjustTableSize();

    blockPage(true);
    if (sel == -1)
        styleTable->selectRow(0);
    else
        styleTable->selectRow(sel);
    blockPage(false);

    displayStyleParams();

    lowerWidget->setFocus();

    //updateGeometry();

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
                qcbTiling->addItem(tiling->getName().get());
            }

            TilingPtr tp = style->getPrototype()->getTiling();
            if (tp)
            {
                int index = qcbTiling->findText(tp->getName().get());
                qcbTiling->setCurrentIndex(index);
            }

            QTableWidgetItem * addrtxt = new QTableWidgetItem(addr(style.get()));
            styleTable->setItem(row,STYLE_COL_ADDR,addrtxt);
            addrtxt->setData(Qt::UserRole,QVariant::fromValue(WeakStylePtr(style)));     // tiling name also stores Style address

            QTableWidgetItem * xftext = new QTableWidgetItem("Xform");
            styleTable->setItem(row,STYLE_COL_TRANS,xftext);
            
            Xform xf = style->getModelXform();
            xftext->setText(xf.info(8));

            // these three connects all pas the row not the index
            connect(qcbStyle,  QOverload<int>::of(&QComboBox::currentIndexChanged), this,  [this,row] { styleChanged(row); });
            connect(qcbTiling, QOverload<int>::of(&QComboBox::currentIndexChanged), this,  [this,row] { tilingChanged(row); });
            connect(cbShow,    &QCheckBox::clicked,                                 this,  [this,row] { styleVisibilityChanged(row); });

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

    qDebug() << "page_mosaic_maker::slot_styleSelected";

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

    setCurrentEditor(style);

    //updateGeometry();

//    parmsTable->resizeColumnsToContents();
//    parmsTable->adjustTableSize();
}

void page_mosaic_maker::setCurrentEditor(StylePtr style)
{
    if (!style)
    {
        return;
    }

    currentEditor->onExit();


    StyleEditor * newEditor = emptyWidget;
    switch (style->getStyleType())
    {
    case STYLE_PLAIN:
        newEditor = new ColoredEditor(style);
        break;
    case STYLE_THICK:
        newEditor = new ThickEditor(style);
        break;
    case STYLE_FILLED:
        newEditor = new FilledEditor(style);
        break;
    case STYLE_EMBOSSED:
        newEditor = new EmbossEditor(style);
        break;
    case STYLE_INTERLACED:
        newEditor = new InterlaceEditor(style);
        break;
    case STYLE_OUTLINED:
        newEditor = new ThickEditor(style);
        break;
    case STYLE_SKETCHED:
        newEditor = new ColoredEditor(style);
        break;
    case STYLE_TILECOLORS:
        newEditor =  new TileColorsEditor(style);
        break;
    case STYLE_STYLE:
    case STYLE_BORDER:
        qCritical("unexpected style");
        break;
    }

    lowerWidget->layout()->replaceWidget(currentEditor,newEditor);
    delete currentEditor;
    currentEditor = newEditor;

    currentEditor->onEnter();
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

    emit sig_reconstructView();

    reEnter();
}

void page_mosaic_maker::styleVisibilityChanged(int row)
{
    qDebug() << "visibility changed: row=" << row;

    QCheckBox * cb = dynamic_cast<QCheckBox*>(styleTable->cellWidget(row,STYLE_COL_CHECK_SHOW));
    bool visible   = cb->isChecked();

    StylePtr style = getStyleRow(row);
    style->setVisible(visible);
    emit sig_reconstructView();
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

    emit sig_reconstructView();
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

    emit sig_reconstructView();

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

    emit sig_reconstructView();

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

    emit sig_reconstructView();

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

    emit sig_reconstructView();

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
    MapPtr map = style->getExistingProtoMap();
    if (map)
    {
        qDebug().noquote() << map->displayVertexEdgeCounts();
    }
}

StylePtr page_mosaic_maker::copyStyle(const StylePtr style)
{
    return mosaicMaker->makeStyle(style->getStyleType(),style);
}

void page_mosaic_maker::slot_set_reps()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (!mosaic) return;

    FillData fd;
    fd.set(chkSingle->isChecked(),xRepMin->value(), xRepMax->value(), yRepMin->value(), yRepMax->value());

    CanvasSettings & cs = mosaic->getCanvasSettings();
    cs.setFillData(fd);

    emit sig_render();
}

void page_mosaic_maker::slot_setCleanse()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (!mosaic) return;

    auto map   = mosaic->getFirstExistingPrototypeMap();
    uint level = mosaic->getCleanseLevel();
    qreal sens = mosaic->getCleanseSensitivity();

    DlgCleanse dlg(map,level,sens,this);

    connect(&dlg, &DlgCleanse::sig_cleansed,this,&page_mosaic_maker::slot_cleansed, Qt::QueuedConnection);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    level = dlg.getLevel();
    sens  = dlg.getSsnsitivity();
    mosaic->setCleanseLevel(level);
    mosaic->setCleanseSensitivity(sens);

    mosaic->resetProtoMaps();

    emit sig_render();
}

void page_mosaic_maker::slot_cleansed()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (!mosaic) return;

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

    emit sig_render();
}
