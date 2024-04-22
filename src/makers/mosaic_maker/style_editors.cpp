#include <QHeaderView>
#include <QComboBox>
#include <QCheckBox>

#include "engine/image_engine.h"
#include "makers/mosaic_maker//style_color_fill_face.h"
#include "makers/mosaic_maker//style_color_fill_set.h"
#include "makers/mosaic_maker/style_color_fill_group.h"
#include "makers/mosaic_maker/style_color_fill_original.h"
#include "makers/mosaic_maker/style_editors.h"
#include "makers/prototype_maker/prototype.h"
#include "misc/utilities.h"
#include "misc/sys.h"
#include "panels/controlpanel.h"
#include "panels/panel_misc.h"
#include "settings/configuration.h"
#include "style/colored.h"
#include "style/emboss.h"
#include "style/filled.h"
#include "style/interlace.h"
#include "style/tile_colors.h"
#include "tile/tile.h"
#include "tile/tiling.h"
#include "viewers/motif_view.h"
#include "viewers/view_controller.h"
#include "widgets/dlg_colorSet.h"
#include "widgets/layout_sliderset.h"

#define ROW_HEIGHT 39

StyleEditor::StyleEditor()
{
    ViewController * vcontrol = Sys::viewController;
    
    connect(this, &StyleEditor::sig_refreshView,  vcontrol, &ViewController::slot_reconstructView);
}

///////////////////////////////////////////////////////////////
///   Colored
///////////////////////////////////////////////////////////////

ColoredEditor::ColoredEditor(Colored * c, AQTableWidget * table)
{
    colored = c;
    this->table = table;

    TPColor tpcolor = colored->getColorSet()->getFirstTPColor();
    qreal  opacity  = tpcolor.color.alphaF();
    qDebug() << "color=" << tpcolor.color << "opacity=" << opacity;

    table->clear();
    table->setColumnCount(3);
    table->setRowCount(2);
    table->setColumnWidth(1,301);
    table->horizontalHeader()->setVisible(false);
    table->verticalHeader()->setVisible(false);

    rows = 0;

    // color
    QTableWidgetItem * item = new QTableWidgetItem("Color");
    table->setItem(rows,0,item);

    colorwidget = colored->getColorSet()->createWidget();
    table->setCellWidget(rows,1,colorwidget);

    color_button = new QPushButton("Select Color");
    table->setCellWidget(rows,2,color_button);

    rows++;

    // opacity
    item = new QTableWidgetItem("Opacity");
    table->setItem(rows,0,item);

    opacitySlider = new DoubleSliderSet("", opacity, 0.0, 1.0, 100);
    QWidget * widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(opacitySlider);
    table->setCellWidget(rows,1,widget);
    table->setRowHeight(rows,ROW_HEIGHT);
    rows++;

    connect(color_button,  &QPushButton::clicked,          this, &ColoredEditor::slot_pickColor);
    connect(opacitySlider, &DoubleSliderSet::valueChanged, this, &ColoredEditor::slot_opacityChanged);
}

void ColoredEditor::slot_opacityChanged(qreal val)
{
    ColorSet * cset = colored->getColorSet();
    cset->setOpacity(val);

    colorwidget = cset->createWidget();
    table->setCellWidget(rows,1,colorwidget);

    emit sig_colorsChanged();
    emit sig_refreshView();
}

void ColoredEditor::slot_pickColor()
{
    DlgColorSet dlg(colored->getColorSet());

    connect(&dlg, &DlgColorSet::sig_colorsChanged, this, &ColoredEditor::slot_colorsChanged);

    dlg.exec();
}

void ColoredEditor::slot_colorsChanged()
{
    colorwidget = colored->getColorSet()->createWidget();
    table->setCellWidget(0,1,colorwidget);
    colored->resetStyleRepresentation();
    emit sig_colorsChanged();
    emit sig_refreshView();
}

///////////////////////////////////////////////////////////////
///   Thick
///////////////////////////////////////////////////////////////
ThickEditor::ThickEditor(Thick * thick, AQTableWidget * table) : ColoredEditor(thick,table)
{
    this->thick = thick;
    table->setRowCount(rows + 5);

    // width
    QTableWidgetItem * item;
    item = new QTableWidgetItem("Width");
    table->setItem(rows,0,item);

    qreal width = thick->getLineWidth();
    int val = static_cast<int>(width * 100);
    width_slider = new SliderSet("", val, 1, 100);
    QWidget * widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(width_slider);
    table->setCellWidget(rows,1,widget);
    table->setRowHeight(rows,ROW_HEIGHT);

    rows++;

    // join style
    item = new QTableWidgetItem("Pen Styles");
    table->setItem(rows,0,item);

    join_style = new QComboBox();
    join_style->addItem("Bevel Join",Qt::BevelJoin);
    join_style->addItem("Mitre Join",Qt::MiterJoin);
    join_style->addItem("Round Join",Qt::RoundJoin);
    table->setCellWidget(rows,1,join_style);

    int index = join_style->findData(thick->getJoinStyle());
    join_style->setCurrentIndex(index);

    cap_style = new QComboBox();
    cap_style->addItem("Square Cap",Qt::SquareCap);
    cap_style->addItem("Flat Cap",Qt::FlatCap);
    cap_style->addItem("Round Cap",Qt::RoundCap);
    table->setCellWidget(rows,2,cap_style);

    index = cap_style->findData(thick->getCapStyle());
    cap_style->setCurrentIndex(index);

    rows++;

    // outline
    item = new QTableWidgetItem("Outline");
    table->setItem(rows,0,item);

    outline_checkbox = new AQCheckBox();
    switch (thick->getDrawOutline())
    {
    case OUTLINE_NONE:
        outline_checkbox->setChecked(false);
        break;
    case OUTLINE_DEFAULT:
    case OUTLINE_SET:
        outline_checkbox->setChecked(true);
        break;
    }
    table->setCellWidget(rows,1,outline_checkbox);

    rows++;

    // outline width
    item = new QTableWidgetItem("Outline Width");
    table->setItem(rows,0,item);

    qreal outline_width = thick->getOutlineWidth();
    val = static_cast<int>(outline_width * 100);
    outline_width_slider = new SliderSet("", val, 1, 100);
    widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(outline_width_slider);
    table->setCellWidget(rows,1,widget);
    table->setRowHeight(rows,ROW_HEIGHT);

    rows++;

    // outline color
    item = new QTableWidgetItem("Outline Color");
    table->setItem(rows,0,item);

    outline_color = new QTableWidgetItem();
    outline_color->setBackground(thick->getOutlineColor());
    table->setItem(rows,1,outline_color);

    outline_color_button = new QPushButton("Color");
    table->setCellWidget(rows,2,outline_color_button);

    rows++;

    connect(outline_checkbox,     &QCheckBox::stateChanged, this, &ThickEditor::slot_outlineChanged);
    connect(width_slider,         &SliderSet::valueChanged, this, &ThickEditor::slot_widthChanged);
    connect(outline_width_slider, &SliderSet::valueChanged, this, &ThickEditor::slot_outlineWidthChanged);
    connect(outline_color_button, &QPushButton::clicked,    this, &ThickEditor::slot_outlineColor);
    connect(join_style,           QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ThickEditor::slot_joinStyle);
    connect(cap_style,            QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ThickEditor::slot_capStyle);
}

void ThickEditor::slot_joinStyle(int index)
{
    Q_UNUSED(index);
    thick->setJoinStyle(static_cast<Qt::PenJoinStyle>(join_style->currentData().toInt()));
    emit sig_refreshView();
}

void ThickEditor::slot_capStyle(int index)
{
    Q_UNUSED(index);
    thick->setCapStyle(static_cast<Qt::PenCapStyle>(cap_style->currentData().toInt()));
    emit sig_refreshView();
}

void  ThickEditor::slot_widthChanged(int width)
{
    qreal val = width/100.0;
    thick->setLineWidth(val);
    thick->resetStyleRepresentation();
    emit sig_refreshView();
}

void  ThickEditor::slot_outlineChanged(int state)
{
    bool checked = (state == Qt::Checked);
    eDrawOutline outline = (checked) ? OUTLINE_SET : OUTLINE_NONE;
    thick->setDrawOutline(outline);
    emit sig_refreshView();
}

void ThickEditor::slot_outlineWidthChanged(int width)
{
    qreal val = width/100.0;
    thick->setOutlineWidth(val);
    auto outline = thick->getDrawOutline();
    if (outline == OUTLINE_DEFAULT)
    {
        thick->setDrawOutline(OUTLINE_SET);
    }

    emit sig_refreshView();
}

void ThickEditor::slot_outlineColor()
{
    QColor color = thick->getOutlineColor();

    AQColorDialog dlg(color,table);
    dlg.setOption(QColorDialog::ShowAlphaChannel);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
    {
        return;
    }
    color = dlg.selectedColor();

    thick->setOutlineColor(color);

    outline_color->setBackground(color);  // menu

    emit sig_refreshView();
}

////////////////////////////////////////////////////////////////////////////
// Filled
////////////////////////////////////////////////////////////////////////////
FilledEditor::FilledEditor(FilledPtr f, AQTableWidget * table , QVBoxLayout *parmsCtrl) : StyleEditor()
{
    filled       = f;
    this->table  = table;
    vbox         = parmsCtrl;
    fillSet      = nullptr;
    fillGroup    = nullptr;
    fillFaces    = nullptr;
    fillOriginal = nullptr;
    panel        = Sys::controlPanel;

    auto proto = filled->getPrototype();
    if (!proto->getDCEL())
    {
        filled->createStyleRepresentation();   // builds and  cleans the  dcel
    }

    table->clear();

    table->horizontalHeader()->setVisible(false);
    table->verticalHeader()->setVisible(false);

    displayParms();
}

FilledEditor::~FilledEditor()
{
    if (fillSet)
        delete fillSet;
    if (fillGroup)
        delete fillGroup;
    if (fillFaces)
        delete fillFaces;

    panel->clearStatus();
}

void FilledEditor::displayParms()
{
    eraseLayout(dynamic_cast<QLayout*>(vbox));
    if (fillSet)
    {
        delete fillSet;
        fillSet   = nullptr;
    }
    if (fillGroup)
    {
        delete fillGroup;
        fillGroup = nullptr;
    }
    if (fillFaces)
    {
        delete fillFaces;
        fillFaces = nullptr;
    }
    if (fillOriginal)
    {
        delete fillOriginal;
        fillOriginal = nullptr;
    }

    // clear the table
    table->clearContents();
    table->setColumnCount(2);
    table->setRowCount(1);

    eFillType algo  = filled->getAlgorithm();
    int row         = 0;

    // algorithm
    QTableWidgetItem * item = new QTableWidgetItem("Algorithm");
    table->setItem(row,0,item);

    algoBox = new QComboBox();
    algoBox->addItem("Original: two face sets, two color sets",FILL_ORIGINAL);
    algoBox->addItem("New 1: two face sets, two color sets",FILL_TWO_FACE);
    algoBox->addItem("New 2: multi face sets, one color each",FILL_MULTI_FACE);
    algoBox->addItem("New 3: multi face sets, color sets for each",FILL_MULTI_FACE_MULTI_COLORS);
    algoBox->addItem("New 4: color per face",FILL_DIRECT_FACE);
    table->setCellWidget(row,1,algoBox);

    int index = algoBox->findData(algo);
    algoBox->setCurrentIndex(index);
    connect(algoBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){ slot_algo(index);} );

    switch (algo)
    {
    case FILL_ORIGINAL:
    case FILL_TWO_FACE:
        panel->clearStatus();
        displayParms1();
        break;
    case FILL_MULTI_FACE:
        panel->clearStatus();
        displayParms2();
        break;
    case FILL_MULTI_FACE_MULTI_COLORS:
        panel->clearStatus();
        displayParms3();
        break;
    case FILL_DIRECT_FACE:
        panel->setStatus("Click on palette color in table, then lef-click on faces in the Mosaic (right-click to erase");
        displayParmsPalette();
        break;
    }

    //table->updateGeometry();
    table->resizeColumnsToContents();
    table->adjustTableSize();
    table->update();
}

void FilledEditor::displayParms1()
{
    fillOriginal = new StyleColorFillOriginal(this, filled,vbox);
    fillOriginal->display();
}

void FilledEditor::displayParms2()
{
    fillSet = new StyleColorFillSet(filled,vbox);
    connect(fillSet, &StyleColorFillSet::sig_colorsChanged,     this, &FilledEditor::slot_colorsChanged,     Qt::UniqueConnection);
    fillSet->display();
}

void FilledEditor::displayParms3()
{
    fillGroup = new StyleColorFillGroup(filled,vbox);
    connect(fillGroup, &StyleColorFillGroup::sig_colorsChanged,  this, &FilledEditor::slot_colorsChanged,     Qt::UniqueConnection);
    fillGroup->display();
}

void FilledEditor::displayParmsPalette()
{
    fillFaces = new StyleColorFillFace(this, filled,vbox);
    fillFaces->display();
}

void FilledEditor::slot_algo(int index)
{
    filled->setAlgorithm(eFillType(index));
    filled->resetStyleRepresentation();
    filled->createStyleRepresentation();
    displayParms();
    emit sig_refreshView();
}

void FilledEditor::slot_editB()
{
    qDebug() << "DLG B" << filled.get() << filled->getBlackColorSet();

    DlgColorSet dlg(filled->getBlackColorSet());

    connect(&dlg, &DlgColorSet::sig_colorsChanged, this, &FilledEditor::slot_colorsChanged);

    dlg.exec();

    displayParms();
}

void FilledEditor::slot_editW()
{
    qDebug() << "DLG W" << filled.get() << filled->getWhiteColorSet();

    DlgColorSet dlg(filled->getWhiteColorSet());

    connect(&dlg, &DlgColorSet::sig_colorsChanged, this, &FilledEditor::slot_colorsChanged);

    dlg.exec();

    displayParms();
}

void FilledEditor::slot_colorsChanged()
{
    displayParms();
    Sys::view->update();     // that's all
}

void FilledEditor::onEnter()
{
    connect(Sys::view,        &View::sig_mousePressed,     this, &FilledEditor::slot_mousePressed);
    connect(Sys::imageEngine, &ImageEngine::sig_colorPick, this, &FilledEditor::slot_colorPick, Qt::QueuedConnection);
}

void FilledEditor::onExit()
{
    disconnect(Sys::view,        &View::sig_mousePressed,     this, &FilledEditor::slot_mousePressed);
    disconnect(Sys::imageEngine, &ImageEngine::sig_colorPick, this, &FilledEditor::slot_colorPick);
}

void FilledEditor::onRefresh()
{
    if (filled->getAlgorithm() == FILL_DIRECT_FACE && fillFaces)
    {
        fillFaces->onRefresh();
    }
}

void FilledEditor::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
{
    QPointF mpt = filled->screenToModel(spt);
    qDebug() << "FilledEditor" << spt << mpt;

    eFillType algo = (eFillType)algoBox->currentIndex();
    switch (algo)
    {
    case FILL_ORIGINAL:
    case FILL_TWO_FACE:
        break;
    case FILL_MULTI_FACE:
        fillSet->select(mpt);
        break;
    case FILL_MULTI_FACE_MULTI_COLORS:
        fillGroup->select(mpt);
        break;
    case FILL_DIRECT_FACE:
        fillFaces->select(mpt,btn);
        break;
    }
}

void FilledEditor::slot_colorPick(QColor color)
{
    qDebug() << "slot_colorPick" << color;

    eFillType algo = (eFillType)algoBox->currentIndex();
    switch (algo)
    {
    case FILL_ORIGINAL:
    case FILL_TWO_FACE:
    case FILL_DIRECT_FACE:
        break;
    case FILL_MULTI_FACE:
        fillSet->setColor(color);
        break;
    case FILL_MULTI_FACE_MULTI_COLORS:
        fillGroup->setColor(color);
        break;
    }
}

////////////////////////////////////////////////////////////////////////////
// Embossed
////////////////////////////////////////////////////////////////////////////
EmbossEditor::EmbossEditor(Emboss * e, AQTableWidget * table) : ThickEditor(e,table)
{
    emboss = e;

    table->setRowCount(rows + 1);

    QTableWidgetItem * item;

    item = new QTableWidgetItem("Azimuth Angle");
    table->setItem(rows,0,item);

    qreal angle = e->getAngle() * 180.0 / M_PI;
    int iangle = static_cast<int>(angle);
    angle_slider = new SliderSet("", iangle, 0, 360);
    QWidget * widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(angle_slider);
    table->setCellWidget(rows,1,widget);
    table->setRowHeight(rows,ROW_HEIGHT);

    rows++;

    connect(angle_slider, &SliderSet::valueChanged,        this, &EmbossEditor::slot_anlgeChanged);
    connect(this,         &StyleEditor::sig_colorsChanged, this, &EmbossEditor::slot_colorsChanged);

}

void EmbossEditor::slot_anlgeChanged(int angle)
{
    qDebug() << "angle=" << angle;
    emboss->setAngle( angle * M_PI / 180.0 );
    emit sig_refreshView();
}

void EmbossEditor::slot_colorsChanged()
{
    emboss->resetStyleRepresentation();
}

////////////////////////////////////////////////////////////////////////////
// Interlaced
////////////////////////////////////////////////////////////////////////////

InterlaceEditor::InterlaceEditor(Interlace * i, AQTableWidget * table) : ThickEditor(i,table)
{
    interlace = i;

    table->setRowCount(rows + 4);

    QTableWidgetItem * item;

    item = new QTableWidgetItem("Gap Width");
    table->setItem(rows,0,item);

    qreal gap = i->getGap();
    gap_slider = new DoubleSliderSet("", gap, 0.0, 1.0, 100);
    QWidget * widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(gap_slider);
    table->setCellWidget(rows,1,widget);
    table->setRowHeight(rows,ROW_HEIGHT);

    rows++;
    item = new QTableWidgetItem("Shadow Width");
    table->setItem(rows,0,item);

    qreal shadow = i->getShadow();
    shadow_slider = new DoubleSliderSet("", shadow, 0.0, 0.7, 100);
    widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(shadow_slider);
    table->setCellWidget(rows,1,widget);
    table->setRowHeight(rows,ROW_HEIGHT);

    rows++;
    item = new QTableWidgetItem("Start Under");
    table->setItem(rows,0,item);

    sunder_checkbox = new AQCheckBox();
    sunder_checkbox->setChecked(i->getInitialStartUnder());
    table->setCellWidget(rows,1,sunder_checkbox);
    table->resizeColumnToContents(0);
    table->adjustTableSize();

    rows++;
    item = new QTableWidgetItem("Include Tip Vertices");
    table->setItem(rows,0,item);

    tipVert_checkbox = new AQCheckBox();
    tipVert_checkbox->setChecked(i->getIncludeTipVertices());
    table->setCellWidget(rows,1,tipVert_checkbox);
    table->resizeColumnToContents(0);
    table->adjustTableSize();

    rows++;

    connect(gap_slider,       &DoubleSliderSet::valueChanged,  this, &InterlaceEditor::slot_gapChanged);
    connect(shadow_slider,    &DoubleSliderSet::valueChanged,  this, &InterlaceEditor::slot_shadowChanged);
    connect(sunder_checkbox,  &QCheckBox::stateChanged,        this, &InterlaceEditor::slot_startUnderChanged);
    connect(tipVert_checkbox, &QCheckBox::stateChanged,        this, &InterlaceEditor::slot_includeTipVerticesChanged);
    connect(this,             &StyleEditor::sig_colorsChanged, this, &InterlaceEditor::slot_colorsChanged);
}

void InterlaceEditor::slot_gapChanged(qreal gap)
{
    interlace->setGap(gap);
    interlace->resetStyleRepresentation();
    emit sig_refreshView();
}

void InterlaceEditor::slot_shadowChanged(qreal shadow)
{
    interlace->setShadow(shadow);
    interlace->resetStyleRepresentation();
    emit sig_refreshView();
}

void InterlaceEditor::slot_startUnderChanged(int state)
{
    Q_UNUSED(state)
    interlace->setInitialStartUnder(sunder_checkbox->isChecked());
    interlace->resetStyleRepresentation();
    emit sig_refreshView();
}

void InterlaceEditor::slot_includeTipVerticesChanged(int state)
{
    Q_UNUSED(state)
    interlace->setIncludeTipVertices(tipVert_checkbox->isChecked());
    interlace->resetStyleRepresentation();
    emit sig_refreshView();
}

void InterlaceEditor::slot_colorsChanged()
{
    interlace->resetStyleRepresentation();
}

///////////////////////////////////////////////////////////////
///   TileColors
///////////////////////////////////////////////////////////////
TileColorsEditor::TileColorsEditor(TileColors * c, AQTableWidget * table, TilingPtr tiling)
{
    config  = Sys::config;
    panel   = Sys::controlPanel;

    colStyle      = c;
    this->table  = table;
    this->tiling = tiling;

    buildTable();
}

void TileColorsEditor::buildTable()
{
    qDebug() << "TileColorsEditor::buildTable()";

    colStyle->createStyleRepresentation();  // this sets up the colors if needed

    table->clear();
    table->setColumnCount(4);
    table->setColumnWidth(TILE_COLORS_ADDR,  100);
    table->setColumnWidth(TILE_COLORS_SIDES, 130);
    table->setColumnWidth(TILE_COLORS_BTN,   100);
    table->setColumnWidth(TILE_COLORS_COLORS,400);
    table->setSelectionMode(AQTableWidget::NoSelection);

    QStringList qslH;
    qslH << "Tile" << "Sides" << "Btn" << "Colors" ;
    table->setHorizontalHeaderLabels(qslH);
    table->verticalHeader()->setVisible(false);

    QVector<TilePtr> tiles = tiling->getUniqueTiles();
    for (auto & tile : std::as_const(tiles))
    {
        uniqueTiles.push_back(tile);
    }

    table->setRowCount(uniqueTiles.size() + 1);

    QColor color;
    int   width;;
    bool outline = colStyle->getOutline(color,width);

    int row = 0;

    outline_checkbox = new AQCheckBox("Outline");
    outline_checkbox->setChecked(outline);
    table->setCellWidget(row,0,outline_checkbox);

    color_button = new QPushButton("Color");
    table->setCellWidget(row,2,color_button);

    colorItem = new QTableWidgetItem();
    colorItem->setBackground(color);
    table->setItem(row,1,colorItem);

    width_slider = new SliderSet("Width", width, 1, 10);
    QWidget * widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(width_slider);
    table->setCellWidget(row,3,widget);
    table->setRowHeight(row,ROW_HEIGHT);

    row++;

    connect(outline_checkbox,   &QCheckBox::stateChanged,   this, &TileColorsEditor::slot_outlineChanged);
    connect(color_button,       &QPushButton::clicked,      this, &TileColorsEditor::slot_outline_color);
    connect(width_slider,       &SliderSet::valueChanged,   this, &TileColorsEditor::slot_widthChanged);

    for (auto & tile : std::as_const(tiles))
    {
        QTableWidgetItem * twi = new QTableWidgetItem(Utils::addr(tile.get()));
        table->setItem(row,TILE_COLORS_ADDR,twi);

        QString str = QString("%1 %2 num=%3").arg(tile->numPoints()).arg((tile->isRegular()) ? "Regular" : "Not-regular").arg(tiling->numPlacements(tile));
        twi = new QTableWidgetItem(str);
        table->setItem(row,TILE_COLORS_SIDES,twi);

        QPushButton * btn = new QPushButton("Edit");
        table->setCellWidget(row,TILE_COLORS_BTN,btn);
        connect(btn, &QPushButton::clicked, this, &TileColorsEditor::slot_edit);

        ColorSet * cset = colStyle->getColorSet(tile);
        QWidget * widget = cset->createWidget();
        table->setCellWidget(row,TILE_COLORS_COLORS,widget);

        row++;
    }
    table->adjustTableSize();
    //table->selectRow(0);
}

void TileColorsEditor::slot_edit()
{
    int row = table->currentRow();
    qInfo() << "current row" << row;
    row--;  // first row is outline

    if (row < 0 || row >= (uniqueTiles.size() ))
        return;

    TilePtr tile = uniqueTiles[row].lock();
    if (!tile)
    {
        qWarning() << "tile not found";
        return;
    }

    DlgColorSet dlg(colStyle->getColorSet(tile),table);
    connect(&dlg, &DlgColorSet::sig_colorsChanged, this, &::TileColorsEditor::slot_colors_changed);

    dlg.exec();

    buildTable();
}

void  TileColorsEditor::slot_colors_changed()
{
    emit panel->sig_render();
    buildTable();
}

void TileColorsEditor::slot_outlineChanged(int state)
{
     bool checked = (state == Qt::Checked);

     QColor color;
     int width;

     colStyle->getOutline(color,width);
     colStyle->setOutline(checked,color,width);

     emit sig_refreshView();
}

void  TileColorsEditor::slot_outline_color()
{
    QColor color;
    int width;
    bool outlineEnb = colStyle->getOutline(color,width);

    AQColorDialog dlg(color,table);
    dlg.setOption(QColorDialog::ShowAlphaChannel);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
    {
        return;
    }
    color = dlg.selectedColor();

    colStyle->setOutline(outlineEnb,color,width);

    colorItem->setBackground(color);

    emit sig_refreshView();

}

void TileColorsEditor::slot_widthChanged(int val)
{
    QColor color;
    int width;
    bool outlineEnb = colStyle->getOutline(color,width);
    colStyle->setOutline(outlineEnb,color,val);

    emit sig_refreshView();
}
