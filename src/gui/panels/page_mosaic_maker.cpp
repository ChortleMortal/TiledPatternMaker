#include <QComboBox>
#include <QCheckBox>
#include <QHeaderView>

#include "gui/model_editors/style_edit/border_editor.h"
#include "gui/model_editors/style_edit/colored_editor.h"
#include "gui/model_editors/style_edit/emboss_editor.h"
#include "gui/model_editors/style_edit/fill_editor.h"
#include "gui/model_editors/style_edit/interlace_editor.h"
#include "gui/model_editors/style_edit/style_editor.h"
#include "gui/model_editors/style_edit/tile_colors_editor.h"
#include "gui/panels/page_borders.h"
#include "gui/panels/page_crop_maker.h"
#include "gui/panels/page_mosaic_maker.h"
#include "gui/top/controlpanel.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/crop_maker_view.h"
#include "gui/widgets/dlg_cleanse.h"
#include "gui/widgets/layout_sliderset.h"
#include "gui/widgets/panel_misc.h"
#include "gui/widgets/smx_widget.h"
#include "model/makers/crop_maker.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/borders/border.h"
#include "model/mosaics/mosaic.h"
#include "model/prototypes/prototype.h"
#include "model/styles/style.h"
#include "model/tilings/backgroundimage.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/map.h"

typedef std::weak_ptr<Mosaic> WeakMosaicPtr;

using std::make_shared;

page_mosaic_maker:: page_mosaic_maker(ControlPanel * apanel)  : panel_page(apanel,PAGE_MOSIAC_MAKER,"Mosaic Maker")
{
    setFixedWidth(PANEL_RHS_WIDTH);

    // top row
    QHBoxLayout * fillBox = createFillDataRow();

    // second row
    delBtn      = new QPushButton("Delete");
    upBtn       = new QPushButton("Move Up");
    downBtn     = new QPushButton("MoveDown");
    dupBtn      = new QPushButton ("Duplicate");
    chk_nolayer = new QCheckBox("Hide Layer Control");
    chk_noaddr  = new QCheckBox("Hide Address");

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
    hbox->addWidget(chk_noaddr);

    // third row - table
    styleTable = new AQTableWidget();
    styleTable->setColumnCount(STYLE_COL_NUM_COLS);
    styleTable->setSelectionMode(QAbstractItemView::SingleSelection);
    styleTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    styleTable->setFocusPolicy(Qt::StrongFocus);
    QStringList qslH;
    qslH << "" << "Tiling" << "Component Type" << "Layer Control" << "Info" << "Address";
    styleTable->setHorizontalHeaderLabels(qslH);
    styleTable->verticalHeader()->setVisible(false);

    styleTable->setRowCount(1);

    connect(styleTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this](const QItemSelection &sel, const QItemSelection &desel)
            {
                // Newly selected cells
                for (const QModelIndex &idx : sel.indexes())
                {
                    if (auto *w = styleTable->cellWidget(idx.row(), idx.column()))
                    {
                        if (auto *cw = qobject_cast<SMXWidget*>(w))
                            cw->setSelected(true);
                    }
                }

                // Newly deselected cells
                for (const QModelIndex &idx : desel.indexes())
                {
                    if (auto *w = styleTable->cellWidget(idx.row(), idx.column()))
                    {
                        if (auto *cw = qobject_cast<SMXWidget*>(w))
                            cw->setSelected(false);
                    }
                }
            });


    componentsBox = new QGroupBox("Mosaic Components");
    AQVBoxLayout * componentsLayout = new AQVBoxLayout;
    componentsLayout->addLayout(hbox);
    componentsLayout->addSpacing(10);
    componentsLayout->addWidget(styleTable);
    componentsBox->setLayout(componentsLayout);

    // fourth row - style editor
    AQHBoxLayout * hbox2 = new AQHBoxLayout();
    StylePtr sp;
    currentEditor = new StyleEditor(sp,STYLE_NONE);
    hbox2->addWidget(currentEditor);

    lowerWidget   = new QWidget();
    lowerWidget->setContentsMargins(0,0,0,0);
    lowerWidget->setLayout(hbox2);

    styleEditorBox = new QGroupBox();
    AQVBoxLayout * styleEditorLayout = new AQVBoxLayout;
    styleEditorLayout->addWidget(lowerWidget);
    styleEditorBox->setLayout(styleEditorLayout);

    QHBoxLayout * hbl = new QHBoxLayout();
    hbl->addLayout(buildDistortionsLayout());
    hbl->addStretch();
    hbl->addWidget(pbClean);

    AQVBoxLayout * wvbox = new AQVBoxLayout;
    wvbox->addLayout(fillBox);
    wvbox->addSpacing(10);
    wvbox->addLayout(hbl);
    wvbox->addSpacing(10);
    wvbox->addWidget(componentsBox);
    wvbox->addSpacing(10);
    wvbox->addWidget(styleEditorBox);
    wvbox->addStretch();

    vbox->addLayout(wvbox);

    connect(delBtn,      &QPushButton::clicked, this, &page_mosaic_maker::slot_deleteStyle);
    connect(upBtn,       &QPushButton::clicked, this, &page_mosaic_maker::slot_moveStyleUp);
    connect(downBtn,     &QPushButton::clicked, this, &page_mosaic_maker::slot_moveStyleDown);
    connect(dupBtn,      &QPushButton::clicked, this, &page_mosaic_maker::slot_duplicateStyle);
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

    pbClean       = new QPushButton("Prototype Cleanse : OFF");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(chkSingle);
    hbox->addSpacing(3);
    hbox->addLayout(xRepMin);
    hbox->addLayout(xRepMax);
    hbox->addLayout(yRepMin);
    hbox->addLayout(yRepMax);
    hbox->addSpacing(9);
    hbox->addWidget(pbRender);

    connect(chkSingle,&QCheckBox::clicked,   this, &page_mosaic_maker::singleton_changed);
    connect(xRepMin, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(xRepMax, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(yRepMin, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(yRepMax, &SpinSet::valueChanged, this, &page_mosaic_maker::slot_set_reps);
    connect(pbRender,&QPushButton::clicked,  this, [] {Sys::render(RENDER_RESET_STYLES);} );
    connect(pbClean, &QPushButton::clicked,  this, &page_mosaic_maker::slot_setCleanse);

    return hbox;
}

void page_mosaic_maker::slot_gotoBackgrounds()
{
    panel->setCurrentPage("Backgrounds");
}

void page_mosaic_maker::slot_gotoCropMaker()
{
    panel->setCurrentPage("Crop Maker");
}

void page_mosaic_maker::slot_gotoBorderMaker()
{
    auto border = std::dynamic_pointer_cast<Border>(getStyleRow(styleTable->currentRow()));
    auto * pb   = dynamic_cast<page_borders*>(panel->getPage("Border Maker"));
    if (border && pb)
    {
        pb->setCurrentBorder(border);
    }
    panel->setCurrentPage("Border Maker");
}

void page_mosaic_maker::slot_showMosaicCrop(bool checked)
{
    auto * pcm = dynamic_cast<page_crop_maker*>(panel->getPage("Crop Maker"));
    if (pcm)
        pcm->setShowMosaicCrop(checked);
}

void page_mosaic_maker::slot_showPainterCrop(bool checked)
{
    auto * pcm = dynamic_cast<page_crop_maker*>(panel->getPage("Crop Maker"));
    if (pcm)
        pcm->setShowPainterCrop(checked);
}

QHBoxLayout * page_mosaic_maker::buildDistortionsLayout()
{
    distortCombo = new AQComboBox();
    distortCombo->addItem("No Distortion",     DISTORT_NONE);
    distortCombo->addItem("Canvas Distortion", DISTORT_CANVAS);
    distortCombo->addItem("Tile Distortion",   DISTORT_TILE);

    xBox = new DoubleSpinSet("X:",1.0,-9.99,9.99);
    xBox->setSingleStep(0.01);
    yBox = new DoubleSpinSet("Y:",1.0,-9.99,9.99);
    yBox->setSingleStep(0.01);

    connect(xBox, &DoubleSpinSet::valueChanged, this, &page_mosaic_maker::setDistortion);
    connect(yBox, &DoubleSpinSet::valueChanged, this, &page_mosaic_maker::setDistortion);
    connect(distortCombo, &QComboBox::currentIndexChanged, this, &page_mosaic_maker::setDistortion);

    QHBoxLayout * dist = new QHBoxLayout;
    dist->addWidget(distortCombo);
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
    const FillData & fd = mosaic->getMosaicCanvas()->getFillData();
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

    ProtoPtr proto = getCurrentPrototype();
    bool cleanseOn = proto && proto->getCleanseLevel() > 0;
    pbClean->setText(cleanseOn ? "Prototype Cleanse : ON " : "Prototype Cleanse : OFF");

    eDistortionMode distortMode; qreal distortX,distortY;
    mosaic->getMosaicCanvas()->getDistortion().get(distortMode,distortX,distortY);
    distortCombo->select(distortMode);
    xBox->setValue(distortX);
    yBox->setValue(distortY);

    if (mosaic == wmp.lock())
    {
        // same mosaic as before
        const StyleSet & sset = mosaic->getStyleSet();
        for (int row = 0; row < sset.size(); row++)
        {
            QWidget * w = styleTable->cellWidget(row,STYLE_COL_LAYER_CONTROL);
            SMXWidget * smx = static_cast<SMXWidget*>(w);
            smx->refresh();
        }

        if (cbShowBkgd)
        {
            // background row is always the last row when present - same lookup as a style row's
            int bkgdRow = styleTable->rowCount()-1;
            QWidget * w = styleTable->cellWidget(bkgdRow,STYLE_COL_LAYER_CONTROL);
            SMXWidget * smx = static_cast<SMXWidget*>(w);
            smx->refresh();

            // reflects enables that can also be changed elsewhere (LHS panel, Backgrounds page)
            cbShowBkgd->blockSignals(true);
            cbShowBkgd->setChecked(viewControl->isEnabled(Layer::Type::BkgdImage));
            cbShowBkgd->blockSignals(false);
        }

        if (cbShowCrop)
        {
            // reflects show-state that can also be changed on the Crop Maker page
            cbShowCrop->blockSignals(true);
            cbShowCrop->setChecked(Sys::cropMakerView->getShowCrop(CM_MOSAIC));
            cbShowCrop->blockSignals(false);
        }

        if (cbShowPainterCrop)
        {
            cbShowPainterCrop->blockSignals(true);
            cbShowPainterCrop->setChecked(Sys::cropMakerView->getShowCrop(CM_PAINTER));
            cbShowPainterCrop->blockSignals(false);
        }

        styleTable->resizeColumnsToContents();
        styleTable->adjustTableSize();
    }
    else
    {
        // mosiac has changed, start again
        wmp = mosaic;
        reEnter();
    }

    if (currentEditor)
    {
        currentEditor->onRefresh();
    }
}

void  page_mosaic_maker::onEnter()
{
    reEnter();
}

void  page_mosaic_maker::reEnter()
{
    displayStyles();
    displayStyleParams();
    updateStyleButtons(styleTable->currentRow());
    lowerWidget->setFocus();
}

void page_mosaic_maker::displayStyles()
{
    blockPage(true);

    int sel = styleTable->currentRow();
    styleTable->clearContents();
    styleTable->setRowCount(0);
    // clearContents() just deleted the old cell widgets
    cbShowBkgd        = nullptr;
    cbShowCrop        = nullptr;
    cbShowPainterCrop = nullptr;

    int row = 0;
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (mosaic && mosaic->hasContent())
    {
        const QVector<TilingPtr> & tilings = tilingMaker->getTilings();  // this is tilings to choose from not tilings used

        const StyleSet & sset = mosaic->getStyleSet();
        // the last style in the set is painted last (on top), so display it first - topmost visual layer at the top of the table
        for (auto it = sset.crbegin(); it != sset.crend(); ++it)
        {
            const StylePtr & style = *it;

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

            // layer control
            SMXWidget * smx = new SMXWidget(style.get(),true,false);
            styleTable->setCellWidget(row,STYLE_COL_LAYER_CONTROL,smx);

            // these three connects all pas the row not the index
            connect(qcbStyle,  &QComboBox::currentIndexChanged, this,  [this,row] { styleChanged(row); });
            connect(qcbTiling, &QComboBox::currentIndexChanged, this,  [this,row] { tilingChanged(row); });
            connect(cbShow,    &QCheckBox::clicked,             this,  [this,row] (bool checked){ styleVisibilityChanged(row, checked); });

            row++;
        }

        CropPtr crop = mosaic->getCrop();
        if (crop)
        {
            QPushButton * btn = addInfoRow(row, "Mosaic Crop", crop->getContentString(), addr(crop.get()));
            btn->setStyleSheet("text-align: left; padding-left: 5px;");
            connect(btn, &QPushButton::clicked, this, &page_mosaic_maker::slot_gotoCropMaker);

            cbShowCrop = new QCheckBox();
            cbShowCrop->setChecked(Sys::cropMakerView->getShowCrop(CM_MOSAIC));
            styleTable->setCellWidget(row,STYLE_COL_ENABLE,cbShowCrop);
            connect(cbShowCrop, &QCheckBox::clicked, this, &page_mosaic_maker::slot_showMosaicCrop);

            row++;
        }

        CropPtr painterCrop = mosaic->getPainterCrop();
        if (painterCrop)
        {
            QPushButton * btn = addInfoRow(row, "Painter Crop", painterCrop->getContentString(), addr(painterCrop.get()));
            btn->setStyleSheet("text-align: left; padding-left: 5px;");
            connect(btn, &QPushButton::clicked, this, &page_mosaic_maker::slot_gotoCropMaker);

            cbShowPainterCrop = new QCheckBox();
            cbShowPainterCrop->setChecked(Sys::cropMakerView->getShowCrop(CM_PAINTER));
            styleTable->setCellWidget(row,STYLE_COL_ENABLE,cbShowPainterCrop);
            connect(cbShowPainterCrop, &QCheckBox::clicked, this, &page_mosaic_maker::slot_showPainterCrop);

            row++;
        }

        // the one background image - it is what Save Mosaic writes
        BkgdImagePtr bip = Sys::backgroundImage;
        if (bip->isLoaded())
        {
            QPushButton * btn = addInfoRow(row, "Background Image", bip->getTitle(), addr(bip.get()));
            connect(btn, &QPushButton::clicked, this, &page_mosaic_maker::slot_gotoBackgrounds);

            cbShowBkgd = new QCheckBox();
            cbShowBkgd->setChecked(viewControl->isEnabled(Layer::Type::BkgdImage));
            styleTable->setCellWidget(row,STYLE_COL_ENABLE,cbShowBkgd);
            connect(cbShowBkgd, &QCheckBox::clicked, this, [this](bool checked){ panel->userDelegateView(Layer::Type::BkgdImage,checked); });

            SMXWidget * smx = new SMXWidget(bip.get(),true,false);
            styleTable->setCellWidget(row,STYLE_COL_LAYER_CONTROL,smx);

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
    updateStyleButtons(styleTable->currentRow());
}

void page_mosaic_maker::displayStyleParams()
{
    int row = styleTable->currentRow();  // can be -1
    //qDebug() << "displayStyleParams row =" << row;

    StylePtr style = getStyleRow(row);

    if (currentEditor)
        currentEditor->onExit();

    // set current editor
    StyleEditor * newEditor = nullptr;

    if (style)
    {
        eStyleType stype = style->getStyleType();

        switch (stype)
        {
        case STYLE_PLAIN:
            newEditor = new ColoredEditor(style,stype);
            break;
        case STYLE_THICK:
            newEditor = new ThickEditor(style,stype);
            break;
        case STYLE_FILLED:
            newEditor = new FilledEditor(style,stype);
            break;
        case STYLE_EMBOSSED:
            newEditor = new EmbossEditor(style,stype);
            break;
        case STYLE_INTERLACED:
            newEditor = new InterlaceEditor(style,stype);
            break;
        case STYLE_OUTLINED:
            newEditor = new ThickEditor(style,stype);
            break;
        case STYLE_SKETCHED:
            newEditor = new ColoredEditor(style,stype);
            break;
        case STYLE_TILECOLORS:
            newEditor = new TileColorsEditor(style,stype);
            break;
        case STYLE_BORDER:
        {
            auto * borderEditor = new BorderEditor(style,stype);
            connect(borderEditor, &BorderEditor::sig_editBorder, this, &page_mosaic_maker::slot_gotoBorderMaker);
            newEditor = borderEditor;
        }   break;
        case STYLE_NONE:
            qCritical("unexpected style");
            break;
        }
    }
    else
    {
        // Crop/Background info row - nothing to edit
        newEditor = new StyleEditor(style,STYLE_NONE);
    }

    Q_ASSERT(newEditor);

    lowerWidget->layout()->replaceWidget(currentEditor,newEditor);
    if (currentEditor)
        delete currentEditor;

    currentEditor = newEditor;
    currentEditor->onEnter();

    styleEditorBox->setTitle(getRowLabel(row));
}

QString page_mosaic_maker::getRowLabel(int row)
{
    QWidget * w = styleTable->cellWidget(row,STYLE_COL_STYLE_TYPE);
    if (auto * btn = qobject_cast<QPushButton*>(w))
        return btn->text();
    if (auto * cb = qobject_cast<QComboBox*>(w))
        return cb->currentText();
    return QString();
}

QPushButton * page_mosaic_maker::addInfoRow(int row, QString label, QString info, QString address)
{
    // a non-style info row (crop / background image) - label is a button navigating to its owning page
    styleTable->setRowCount(row+1);

    QPushButton * labelBtn = new QPushButton(label);
    styleTable->setCellWidget(row,STYLE_COL_STYLE_TYPE,labelBtn);

    QTableWidgetItem * infoItem = new QTableWidgetItem(info);
    styleTable->setItem(row,STYLE_COL_INFO,infoItem);

    QTableWidgetItem * addrItem = new QTableWidgetItem(address);
    styleTable->setItem(row,STYLE_COL_STYLE_ADDR,addrItem);

    return labelBtn;
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
        QComboBox * qcb    = static_cast<QComboBox*>(styleTable->cellWidget(row,STYLE_COL_STYLE_TYPE));
        eStyleType newType = static_cast<eStyleType>(qcb->currentData().toUInt());
        StylePtr style     = getStyleRow(row);
        StylePtr newStyle  = mosaicMaker->makeStyle(style,newType);

        mosaic->replaceStyle(style,newStyle);
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

void page_mosaic_maker::updateStyleButtons(int row)
{
    // Move Up/Down/Duplicate act on styles - meaningless for the Crop/Painter
    // Crop/Background Image info rows appended after the styles
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    bool isStyleRow  = mosaic && row >= 0 && row < mosaic->getStyleSet().size();

    upBtn->setEnabled(isStyleRow);
    downBtn->setEnabled(isStyleRow);
    dupBtn->setEnabled(isStyleRow);
    // delBtn stays enabled on the info rows too - it deletes the crop/background they represent

    // Cleanse operates on the style's prototype map - e.g. a Border style has no prototype
    StylePtr style = getStyleRow(row);
    pbClean->setEnabled(style && style->getPrototype());
}

void page_mosaic_maker::slot_deleteStyle()
{
    int row = styleTable->currentRow();
    if (row == -1) return;

    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if  (!mosaic) return;

    StylePtr style = getStyleRow(row);
    if (style)
    {
        mosaic->deleteStyle(style);
    }
    else
    {
        // an info row (Mosaic Crop / Painter Crop / Background Image) - delete what it represents
        QPushButton * labelBtn = qobject_cast<QPushButton*>(styleTable->cellWidget(row,STYLE_COL_STYLE_TYPE));
        QString label = labelBtn ? labelBtn->text() : QString();

        if (label == "Mosaic Crop")
            mosaic->resetCrop();
        else if (label == "Painter Crop")
            mosaic->resetPainterCrop();
        else if (label == "Background Image")
        {
            Sys::backgroundImage->unload();
            mosaic->removeBkgdRecord();
            emit sig_reconstructView();
        }
    }

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
    if (!style) return;    // row is a Crop/Background info row, not a style - button should be disabled

    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if  (!mosaic) return;

    // the table shows the topmost-painted style first, so moving a row up in the table
    // means painting it later, i.e. later in the style set
    mosaic->moveDown(style);
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
    if (!style) return;    // row is a Crop/Background info row, not a style - button should be disabled

    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if  (!mosaic) return;

    // see slot_moveStyleUp() - the table order is reversed relative to the style set order
    mosaic->moveUp(style);

    reEnter();

    emit sig_reconstructView();

    int styleCount = mosaic->getStyleSet().size();
    if (row < (styleCount-1))
        row++;
    styleTable->selectRow(row);
    styleTable->setFocus();
}

void  page_mosaic_maker::slot_duplicateStyle()
{
    int row = styleTable->currentRow();
    if (row == -1) return;

    StylePtr style   = getStyleRow(row);
    if (!style) return;    // row may be a Crop/Background info row, not a style

    StylePtr style2  = mosaicMaker->makeStyle(style, style->getStyleType());
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    mosaic->addStyle(style2);

    reEnter();

    emit sig_reconstructView();

    styleTable->selectRow(row);
    styleTable->setFocus();
}

void page_mosaic_maker::slot_set_reps()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (!mosaic) return;

    FillData fd;
    fd.set(chkSingle->isChecked(),xRepMin->value(), xRepMax->value(), yRepMin->value(), yRepMax->value());

    Canvas * cs = mosaic->getMosaicCanvas();
    cs->setFillData(fd);

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

    Canvas * cs = mosaicMaker->getMosaic()->getMosaicCanvas();
    cs->setFillData(fd);

    Sys::render(RENDER_RESET_PROTOTYPES);
}

void page_mosaic_maker::slot_noaddr(bool checked)
{
    styleTable->setColumnHidden(STYLE_COL_STYLE_ADDR,checked);
}

void page_mosaic_maker::slot_nolayer(bool checked)
{
    styleTable->setColumnHidden(STYLE_COL_LAYER_CONTROL,checked);
}
void page_mosaic_maker::setDistortion()
{
    // distortion now bakes into the prototype map itself (Prototype::_buildPrototypeMap),
    // not just the display - hence the RENDER_RESET_PROTOTYPES below

    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (!mosaic) return;

    Distortion d;
    d.set(static_cast<eDistortionMode>(distortCombo->currentData().toInt()),xBox->value(),yBox->value());

    Canvas * cs = mosaic->getMosaicCanvas();
    cs->setDistortion(d);

    Sys::render(RENDER_RESET_PROTOTYPES);
}

ProtoPtr page_mosaic_maker::getCurrentPrototype()
{
    ProtoPtr proto;

    int row = styleTable->currentRow();  // can be -1
    if (row >= 0)
    {
        //qDebug() << "displayStyleParams row =" << row;
        StylePtr style = getStyleRow(row);
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
