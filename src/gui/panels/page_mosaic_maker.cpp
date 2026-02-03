#include <QComboBox>
#include <QCheckBox>
#include <QHeaderView>

#include "gui/model_editors/style_edit/style_editors.h"
#include "gui/panels/page_mosaic_maker.h"
#include "gui/panels/panel_misc.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view_controller.h"
#include "gui/widgets/dlg_cleanse.h"
#include "gui/widgets/layout_sliderset.h"
#include "gui/widgets/smx_widget.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/prototypes/prototype.h"
#include "model/styles/style.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/map.h"
#include "sys/geometry/transform.h"

Q_DECLARE_METATYPE(WeakStylePtr)

typedef std::weak_ptr<Mosaic>          WeakMosaicPtr;

using std::make_shared;

page_mosaic_maker:: page_mosaic_maker(ControlPanel * apanel)  : panel_page(apanel,PAGE_MOSIAC_MAKER,"Mosaic Maker")
{
    setFixedWidth(PANEL_RHS_WIDTH-40);

    // top row
    QHBoxLayout * fillBox = createFillDataRow();

    // second row
    delBtn      = new QPushButton("Delete");
    upBtn       = new QPushButton("Move Up");
    downBtn     = new QPushButton("MoveDown");
    dupBtn      = new QPushButton ("Duplicate");
    chk_notrans = new QCheckBox("Hide Transform");
    chk_nolayer = new QCheckBox("Hide Layer Control");
    chk_noaddr  = new QCheckBox("Hide Addr");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(delBtn);
    hbox->addSpacing(10);
    hbox->addWidget(upBtn);
    hbox->addSpacing(10);
    hbox->addWidget(downBtn);
    hbox->addSpacing(10);
    hbox->addWidget(dupBtn);
    hbox->addStretch();
    hbox->addWidget(chk_nolayer);
    hbox->addWidget(chk_notrans);
    hbox->addWidget(chk_noaddr);

    // third row - table
    styleTable = new AQTableWidget();
    styleTable->setColumnCount(STYLE_COL_NUM_COLS);
    styleTable->setSelectionMode(QAbstractItemView::SingleSelection);
    styleTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    styleTable->setFocusPolicy(Qt::StrongFocus);
    QStringList qslH;
    qslH << "" << "Tiling" << "Style type" << "Layer Control" << "Transform" << "Style Addr";
    styleTable->setHorizontalHeaderLabels(qslH);
    styleTable->verticalHeader()->setVisible(false);

    // fourth row - style editor
    AQHBoxLayout * hbox2 = new AQHBoxLayout();
    currentEditor = new StyleEditor();
    hbox2->addWidget(currentEditor);

    lowerWidget   = new QWidget();
    lowerWidget->setContentsMargins(0,0,0,0);
    lowerWidget->setLayout(hbox2);

    QHBoxLayout * hbl = new QHBoxLayout();
    hbl->addLayout(buildDistortionsLayout());
    hbl->addStretch();
    hbl->addLayout(createBackgroundInfo());

    AQVBoxLayout * wvbox = new AQVBoxLayout;
    wvbox->addLayout(fillBox);
    wvbox->addSpacing(10);
    wvbox->addLayout(hbox);
    wvbox->addSpacing(5);
    wvbox->addWidget(styleTable);
    wvbox->addSpacing(5);
    wvbox->addWidget(lowerWidget);
    wvbox->addSpacing(5);
    wvbox->addLayout(hbl);
    wvbox->addStretch();

    vbox->addLayout(wvbox);

    connect(delBtn,      &QPushButton::clicked, this, &page_mosaic_maker::slot_deleteStyle);
    connect(upBtn,       &QPushButton::clicked, this, &page_mosaic_maker::slot_moveStyleUp);
    connect(downBtn,     &QPushButton::clicked, this, &page_mosaic_maker::slot_moveStyleDown);
    connect(dupBtn,      &QPushButton::clicked, this, &page_mosaic_maker::slot_duplicateStyle);
    connect(chk_notrans, &QCheckBox::clicked,   this, &page_mosaic_maker::slot_notrans);
    connect(chk_nolayer, &QCheckBox::clicked,   this, &page_mosaic_maker::slot_nolayer);
    connect(chk_noaddr,  &QCheckBox::clicked,   this, &page_mosaic_maker::slot_noaddr);

    QItemSelectionModel * selectModel = styleTable->selectionModel();
    connect(selectModel, &QItemSelectionModel::selectionChanged, this, &page_mosaic_maker::slot_styleSelected);
}

QHBoxLayout * page_mosaic_maker::createFillDataRow()
{
    const int rmin = -99;
    const int rmax =  99;

    chkSingle = new QCheckBox("Singleton");

    xRepMin = new SpinSet("xMin ",0,rmin,rmax);
    xRepMax = new SpinSet("xMax ",0,rmin,rmax);
    yRepMin = new SpinSet("yMin ",0,rmin,rmax);
    yRepMax = new SpinSet("yMax ",0,rmin,rmax);

    QPushButton * pbRender = new QPushButton("Render Styles");
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
    hbox->addSpacing(7);
    hbox->addWidget(cleanseStatus);

    connect(chkSingle,&QCheckBox::clicked,   this, &page_mosaic_maker::singleton_changed);
    connect(xRepMin, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(xRepMax, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(yRepMin, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(yRepMax, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(pbRender,&QPushButton::clicked,  this, [] {Sys::render(RENDER_RESET_STYLES);} );
    connect(pbClean, &QPushButton::clicked,  this, &page_mosaic_maker::slot_setCleanse);

    return hbox;
}

QHBoxLayout *page_mosaic_maker::createBackgroundInfo()
{
    pbExam  = new QPushButton("Background Image");  // label is overwritten by status

    QHBoxLayout * hb1 = new QHBoxLayout();
    hb1->addWidget(pbExam);

    connect (pbExam, &QPushButton::pressed, this, [this] { panel->setCurrentPage("Backgrounds");} );
    return hb1;
}

QHBoxLayout * page_mosaic_maker::buildDistortionsLayout()
{
    chkDistort = new QCheckBox("Distort");

    xBox = new DoubleSpinSet("X:",1.0,-9.99,9.99);
    xBox->setSingleStep(0.01);
    yBox = new DoubleSpinSet("Y:",1.0,-9.99,9.99);
    yBox->setSingleStep(0.01);

    connect(xBox, &DoubleSpinSet::valueChanged, this, &page_mosaic_maker::setDistortion);
    connect(yBox, &DoubleSpinSet::valueChanged, this, &page_mosaic_maker::setDistortion);
    connect(chkDistort, &QCheckBox::clicked,    this, &page_mosaic_maker::enbDistortion);

    QHBoxLayout * dist = new QHBoxLayout;
    dist->addWidget(chkDistort);
    dist->addLayout(xBox);
    dist->addSpacing(5);
    dist->addLayout(yBox);

    return dist;
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

    if (mosaic->getBkgdImage())
    {
        pbExam->setText("Examine Background Image");
    }
    else
    {
        pbExam->setText("Add Background Image");
    }

    StylePtr style = getStyleRow(styleTable->currentRow());
    if (style)
    {
        ProtoPtr proto = style->getPrototype();
        if (proto)
        {
            uint level = proto->getCleanseLevel();
            if (level > 0)
            {
                pbClean->setText("Cleanse : ON ");
                qreal sensitivity = proto->getCleanseSensitivity();
                QString str = QString("0x%1 : %2").arg(QString::number(level,16)).arg(sensitivity);
                cleanseStatus->setText(str);
            }
            else
            {
                pbClean->setText("Cleanse : OFF");
                cleanseStatus->setText(" ");
            }


        }
        else
        {
            pbClean->setText("Cleanse : OFF");
            cleanseStatus->setText(" ");
        }
    }
    else
    {
        pbClean->setText("Cleanse : OFF");
        cleanseStatus->setText(" ");
    }

    ProtoPtr proto = getCurrentPrototype();
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

    if (mosaic == wmp.lock())
    {
        int row = 0;
        const StyleSet & sset = mosaic->getStyleSet();
        for (auto & style : std::as_const(sset))
        {
            QTableWidgetItem * item = styleTable->item(row,STYLE_COL_TRANSFORM);
            if (item)
            {
                Xform xf = style->getModelXform();
                item->setText(xf.info(8));
            }

            QWidget * w = styleTable->cellWidget(row,STYLE_COL_LAYER_CONTROL);
            SMXWidget * smx = dynamic_cast<SMXWidget*>(w);
            if (smx)
                smx->refresh();

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
    displayStyles();
    displayStyleParams();
    lowerWidget->setFocus();
}

void page_mosaic_maker::displayStyles()
{
    blockPage(true);

    int sel = styleTable->currentRow();
    styleTable->clearContents();
    styleTable->setRowCount(0);

    int row = 0;
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (mosaic && mosaic->hasContent())
    {
        const QVector<TilingPtr> & tilings = tilingMaker->getTilings();  // this is tilings to choose from not tilings used

        const StyleSet & sset = mosaic->getStyleSet();
        for (auto & style : std::as_const(sset))
        {
            // build structure
            styleTable->setRowCount(row+1);

            // enable
            QCheckBox * cbShow = new QCheckBox();
            styleTable->setCellWidget(row,STYLE_COL_ENABLE,cbShow);
            styleTable->setColumnWidth(STYLE_COL_ENABLE,25);

            cbShow->setChecked(style->isVisible());

            // style type
            QComboBox * qcbStyle = new QComboBox();
            qcbStyle->setEditable(false);
            qcbStyle->setFrame(false);
            styleTable->setCellWidget(row,STYLE_COL_STYLE_TYPE,qcbStyle);

            qcbStyle->addItem("Sketched",STYLE_SKETCHED);
            qcbStyle->addItem("Plain",STYLE_PLAIN);
            qcbStyle->addItem("Thick Lines",STYLE_THICK);
            qcbStyle->addItem("Outlined",STYLE_OUTLINED);
            qcbStyle->addItem("Embossed",STYLE_EMBOSSED);
            qcbStyle->addItem("Interlaced",STYLE_INTERLACED);
            qcbStyle->addItem("Filled",STYLE_FILLED);
            qcbStyle->addItem("Tile Colors",STYLE_TILECOLORS);
            qcbStyle->addItem("Border",STYLE_BORDER);

            QString stylename = style->getStyleDesc();
            int index = qcbStyle->findText(stylename);
            Q_ASSERT(index != -1);
            qcbStyle->setCurrentIndex(index);

            // tiling
            QComboBox * qcbTiling = new QComboBox();
            styleTable->setCellWidget(row,STYLE_COL_TILING,qcbTiling);

            for (auto & tiling : std::as_const(tilings))
            {
                qcbTiling->addItem(tiling->getVName().get());
            }

            ProtoPtr pp  = style->getPrototype();
            if (pp)
            {
                TilingPtr tp = pp->getTiling();
                if (tp)
                {
                    int index = qcbTiling->findText(tp->getVName().get());
                    qcbTiling->setCurrentIndex(index);
                }
            }

            // style address
            QTableWidgetItem * addrtxt = new QTableWidgetItem(addr(style.get()));
            styleTable->setItem(row,STYLE_COL_STYLE_ADDR,addrtxt);
            addrtxt->setData(Qt::UserRole,QVariant::fromValue(WeakStylePtr(style)));     // tiling name also stores Style address

            // style transform
            QTableWidgetItem * xftext = new QTableWidgetItem("Xform");
            styleTable->setItem(row,STYLE_COL_TRANSFORM,xftext);
            
            Xform xf = style->getModelXform();
            xftext->setText(xf.info(8));

            // layer control
            SMXWidget * smx = new SMXWidget(style.get(),true,false);
            styleTable->setCellWidget(row,STYLE_COL_LAYER_CONTROL,smx);

            // these three connects all pas the row not the index
            connect(qcbStyle,  &QComboBox::currentIndexChanged, this,  [this,row] { styleChanged(row); });
            connect(qcbTiling, &QComboBox::currentIndexChanged, this,  [this,row] { tilingChanged(row); });
            connect(cbShow,    &QCheckBox::clicked,             this,  [this,row] (bool checked){ styleVisibilityChanged(row, checked); });

            row++;
        }
    }

    styleTable->resizeColumnsToContents();
    styleTable->adjustTableSize();

    if (sel == -1)
        styleTable->selectRow(0);
    else
        styleTable->selectRow(sel);

    blockPage(false);
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
    int row = styleTable->currentRow();  // can be -1
    //qDebug() << "displayStyleParams row =" << row;

    StylePtr style = getStyleIndex(row);
    if (!style)
        return;

    if (currentEditor)
        currentEditor->onExit();

    // set current editor
    StyleEditor * newEditor = nullptr;
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
    case STYLE_BORDER:
        newEditor = new StyleEditor();  // does nothing
        break;
    case STYLE_STYLE:
        qCritical("unexpected style");
        break;
    }

    lowerWidget->layout()->replaceWidget(currentEditor,newEditor);
    if (currentEditor)
        delete currentEditor;

    currentEditor = newEditor;
    if (currentEditor)
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
    if (style)
    {
        ProtoPtr pp = style->getPrototype();
        if (pp)
            pp->replaceTiling(tp);
    }
    emit sig_reconstructView();

    reEnter();
}

void page_mosaic_maker::styleVisibilityChanged(int row, bool checked)
{
    qDebug() << "visibility changed: row=" << row;

    StylePtr style = getStyleRow(row);
    Q_ASSERT(style);
    style->setVisible(checked);
    emit sig_reconstructView();
}

void page_mosaic_maker::styleChanged(int row)
{
    qDebug() << "style changed: row=" << row;

    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        QComboBox * qcb = dynamic_cast<QComboBox*>(styleTable->cellWidget(row,STYLE_COL_STYLE_TYPE));
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

    QTableWidgetItem * twi = styleTable->item(row,STYLE_COL_STYLE_ADDR);
    if (!twi)
    {
        return sp;
    }

    QVariant var = twi->data(Qt::UserRole);
    if (var.canConvert<WeakStylePtr>())
    {
        WeakStylePtr wsp = var.value<WeakStylePtr>();
        sp = wsp.lock();
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

    if (row > 0)
        row--;
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

    int rows = styleTable->rowCount();
    if (row < (rows-1))
        row++;
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

    Sys::render(RENDER_RESET_PROTOTYPES);
}

void page_mosaic_maker::slot_setCleanse()
{
    StylePtr style = getStyleRow(styleTable->currentRow());
    if (!style) return;
    ProtoPtr proto = style->getPrototype();
    if (!proto) return;
    uint level     = proto->getCleanseLevel();
    qreal sens     = proto->getCleanseSensitivity();
    auto map       = proto->getExistingProtoMap();
    DlgCleanse dlg(map,level,sens,this);

    connect(&dlg, &DlgCleanse::sig_cleansed,this,&page_mosaic_maker::slot_cleansed, Qt::QueuedConnection);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    level = dlg.fromCheckboxes();
    sens  = dlg.getSsnsitivity();
    proto->setCleanseLevel(level);
    proto->setCleanseSensitivity(sens);

    Sys::render(RENDER_RESET_PROTOTYPES);
}

void page_mosaic_maker::slot_cleansed()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (!mosaic) return;

    Sys::render(RENDER_RESET_PROTOTYPES);
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

    Sys::render(RENDER_RESET_PROTOTYPES);
}

void page_mosaic_maker::slot_noaddr(bool checked)
{
    styleTable->setColumnHidden(STYLE_COL_STYLE_ADDR,checked);
}

void page_mosaic_maker::slot_notrans(bool checked)
{
    styleTable->setColumnHidden(STYLE_COL_TRANSFORM,checked);
}

void page_mosaic_maker::slot_nolayer(bool checked)
{
    styleTable->setColumnHidden(STYLE_COL_LAYER_CONTROL,checked);
}
void page_mosaic_maker::setDistortion()
{
    // distortion is used to force desings into a fixed space
    // by defornming regular polygons by transforming the map

    // distortion is applied to prototypes used by styles

    ProtoPtr proto = getCurrentPrototype();
    if (!proto) return;

    qreal x = xBox->value();
    qreal y = yBox->value();
    QTransform t = QTransform::fromScale(x,y);
    qDebug() << "scale x" << t.m11() << "y" << t.m22();
    proto->setDistortion(t);

    if (proto->getDistortionEnable())
    {
        auto mosaic = mosaicMaker->getMosaic();
        mosaic->resetProtoMaps();   // really is wipeout
        mosaic->resetStyleMaps();
        emit sig_reconstructView();
    }
}

void page_mosaic_maker::enbDistortion(bool checked)
{
    ProtoPtr proto = getCurrentPrototype();
    if (!proto) return;

    proto->enableDistortion(checked);

    auto mosaic = mosaicMaker->getMosaic();
    mosaic->resetProtoMaps();   // really is wipeout
    mosaic->resetStyleMaps();
    emit sig_reconstructView();
}

ProtoPtr page_mosaic_maker::getCurrentPrototype()
{
    ProtoPtr proto;

    int row = styleTable->currentRow();  // can be -1
    if (row >= 0)
    {
        //qDebug() << "displayStyleParams row =" << row;
        StylePtr style = getStyleIndex(row);
        if (style)
        {
            proto = style->getPrototype();
        }
    }
    if (!proto)
    {
        // try another way
        auto mosaic = mosaicMaker->getMosaic();
        proto = mosaic->getPrototypes().first();
    }

    return proto;
}