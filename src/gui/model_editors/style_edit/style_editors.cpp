#include <QHeaderView>
#include <QComboBox>
#include <QCheckBox>

#include "sys/engine/image_engine.h"
#include "gui/model_editors/style_edit/style_color_fill_face.h"
#include "gui/model_editors/style_edit/style_color_fill_set.h"
#include "gui/model_editors/style_edit/style_color_fill_group.h"
#include "gui/model_editors/style_edit/style_color_fill_original.h"
#include "gui/model_editors/style_edit/style_editors.h"
#include "model/prototypes/prototype.h"
#include "sys/qt/utilities.h"
#include "sys/sys.h"
#include "gui/top/controlpanel.h"
#include "gui/panels/panel_misc.h"
#include "model/styles/colored.h"
#include "model/styles/emboss.h"
#include "model/styles/filled.h"
#include "model/styles/interlace.h"
#include "model/styles/tile_colors.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "gui/viewers/motif_view.h"
#include "gui/top/view_controller.h"
#include "gui/widgets/dlg_colorSet.h"
#include "gui/widgets/layout_sliderset.h"

#define ROW_HEIGHT 39

#if (QT_VERSION >= QT_VERSION_CHECK(6,7,0))
#define CBOX_STATECHANGE &QCheckBox::checkStateChanged
#else
#define CBOX_STATECHANGE &QCheckBox::stateChanged
#endif

StyleEditor::StyleEditor() : QWidget()
{
    setable = nullptr;

    ViewController * vcontrol = Sys::viewController;
    
    connect(this, &StyleEditor::sig_reconstructView, vcontrol,  &ViewController::slot_reconstructView);
    connect(this, &StyleEditor::sig_updateView,      Sys::view, &View::slot_update);
}

///////////////////////////////////////////////////////////////
///   Colored
///////////////////////////////////////////////////////////////

ColoredEditor::ColoredEditor(StylePtr style) : StyleEditor()
{
    auto colored = std::dynamic_pointer_cast<Colored>(style);
    wcolored     = colored;
    Q_ASSERT(wcolored.lock());

    if (!setable)
    {
        setable = new AQTableWidget(this);
        AQVBoxLayout * vbox = new AQVBoxLayout();
        vbox->addWidget(setable);
        setLayout(vbox);
    }

    TPColor tpcolor = colored->getColorSet()->getFirstTPColor();
    qreal  opacity  = tpcolor.color.alphaF();
    qDebug() << "color=" << tpcolor.color << "opacity=" << opacity;

    setable->clear();
    setable->setColumnCount(3);
    setable->setRowCount(2);
    setable->setColumnWidth(1,301);
    setable->horizontalHeader()->setVisible(false);
    setable->verticalHeader()->setVisible(false);

    rows = 0;

    // color
    QTableWidgetItem * item = new QTableWidgetItem("Color");
    setable->setItem(rows,0,item);

    colorwidget = colored->getColorSet()->createWidget();
    setable->setCellWidget(rows,1,colorwidget);

    color_button = new QPushButton("Select Color");
    setable->setCellWidget(rows,2,color_button);

    rows++;

    // opacity
    item = new QTableWidgetItem("Opacity");
    setable->setItem(rows,0,item);

    opacitySlider = new DoubleSliderSet("", opacity, 0.0, 1.0, 100);
    QWidget * widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(opacitySlider);
    setable->setCellWidget(rows,1,widget);
    setable->setRowHeight(rows,ROW_HEIGHT);
    rows++;

    connect(color_button,  &QPushButton::clicked,          this, &ColoredEditor::slot_pickColor);
    connect(opacitySlider, &DoubleSliderSet::valueChanged, this, &ColoredEditor::slot_opacityChanged);

    setable->resizeColumnsToContents();
    setable->adjustTableSize();
}

void ColoredEditor::slot_opacityChanged(qreal val)
{
    auto colored = wcolored.lock();
    if (!colored) return;

    ColorSet * cset = colored->getColorSet();
    cset->setOpacity(val);

    colorwidget = cset->createWidget();
    setable->setCellWidget(rows,1,colorwidget);

    emit sig_colorsChanged();
    emit sig_reconstructView();
}

void ColoredEditor::slot_pickColor()
{
    auto colored = wcolored.lock();
    if (!colored) return;

    DlgColorSet dlg(colored->getColorSet());

    connect(&dlg, &DlgColorSet::sig_dlg_colorsChanged, this, &ColoredEditor::slot_colorsChanged);

    dlg.exec();
}

void ColoredEditor::slot_colorsChanged()
{
    auto colored = wcolored.lock();
    if (!colored) return;

    colorwidget = colored->getColorSet()->createWidget();
    setable->setCellWidget(0,1,colorwidget);
    colored->resetStyleRepresentation();
    emit sig_colorsChanged();
    emit sig_reconstructView();
}

///////////////////////////////////////////////////////////////
///   Thick
///////////////////////////////////////////////////////////////
ThickEditor::ThickEditor(StylePtr style) : ColoredEditor(style)
{
    auto thick = std::dynamic_pointer_cast<Thick>(style);
    wthick = thick;
    Q_ASSERT(wthick.lock());

    if (!setable)
    {
        setable = new AQTableWidget(this);
        AQVBoxLayout * vbox = new AQVBoxLayout();
        vbox->addWidget(setable);
        setLayout(vbox);
    }

    setable->setRowCount(rows + 5);

    // width
    QTableWidgetItem * item;
    item = new QTableWidgetItem("Width");
    setable->setItem(rows,0,item);

    qreal width = thick->getLineWidth();
    int val = static_cast<int>(width * 100);
    width_slider = new SliderSet("", val, 1, 100);
    QWidget * widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(width_slider);
    setable->setCellWidget(rows,1,widget);
    setable->setRowHeight(rows,ROW_HEIGHT);

    rows++;

    // join style
    item = new QTableWidgetItem("Pen Styles");
    setable->setItem(rows,0,item);

    join_style = new QComboBox();
    join_style->addItem("Bevel Join",Qt::BevelJoin);
    join_style->addItem("Mitre Join",Qt::MiterJoin);
    join_style->addItem("Round Join",Qt::RoundJoin);
    setable->setCellWidget(rows,1,join_style);

    int index = join_style->findData(thick->getJoinStyle());
    join_style->setCurrentIndex(index);

    cap_style = new QComboBox();
    cap_style->addItem("Square Cap",Qt::SquareCap);
    cap_style->addItem("Flat Cap",Qt::FlatCap);
    cap_style->addItem("Round Cap",Qt::RoundCap);
    setable->setCellWidget(rows,2,cap_style);

    index = cap_style->findData(thick->getCapStyle());
    cap_style->setCurrentIndex(index);

    rows++;

    // outline
    item = new QTableWidgetItem("Outline");
    setable->setItem(rows,0,item);

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
    setable->setCellWidget(rows,1,outline_checkbox);

    rows++;

    // outline width
    item = new QTableWidgetItem("Outline Width");
    setable->setItem(rows,0,item);

    qreal outline_width = thick->getOutlineWidth();
    val = static_cast<int>(outline_width * 100);
    outline_width_slider = new SliderSet("", val, 1, 100);
    widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(outline_width_slider);
    setable->setCellWidget(rows,1,widget);
    setable->setRowHeight(rows,ROW_HEIGHT);

    rows++;

    // outline color
    item = new QTableWidgetItem("Outline Color");
    setable->setItem(rows,0,item);

    outline_color = new QTableWidgetItem();
    outline_color->setBackground(thick->getOutlineColor());
    setable->setItem(rows,1,outline_color);

    outline_color_button = new QPushButton("Color");
    setable->setCellWidget(rows,2,outline_color_button);

    rows++;

    connect(outline_checkbox,     CBOX_STATECHANGE,         this, &ThickEditor::slot_outlineChanged);
    connect(width_slider,         &SliderSet::valueChanged, this, &ThickEditor::slot_widthChanged);
    connect(outline_width_slider, &SliderSet::valueChanged, this, &ThickEditor::slot_outlineWidthChanged);
    connect(outline_color_button, &QPushButton::clicked,    this, &ThickEditor::slot_outlineColor);
    connect(join_style,           QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ThickEditor::slot_joinStyle);
    connect(cap_style,            QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ThickEditor::slot_capStyle);

    setable->resizeColumnsToContents();
    setable->adjustTableSize();
}

void ThickEditor::slot_joinStyle(int index)
{
    Q_UNUSED(index);

    auto thick = wthick.lock();
    if (!thick) return;

    thick->setJoinStyle(static_cast<Qt::PenJoinStyle>(join_style->currentData().toInt()));
    emit sig_reconstructView();
}

void ThickEditor::slot_capStyle(int index)
{
    Q_UNUSED(index);

    auto thick = wthick.lock();
    if (!thick) return;

    thick->setCapStyle(static_cast<Qt::PenCapStyle>(cap_style->currentData().toInt()));
    emit sig_reconstructView();
}

void  ThickEditor::slot_widthChanged(int width)
{
    auto thick = wthick.lock();
    if (!thick) return;

    qreal val = width/100.0;
    thick->setLineWidth(val);
    thick->resetStyleRepresentation();
    emit sig_reconstructView();
}

void  ThickEditor::slot_outlineChanged(int state)
{
    auto thick = wthick.lock();
    if (!thick) return;

    bool checked = (state == Qt::Checked);
    eDrawOutline outline = (checked) ? OUTLINE_SET : OUTLINE_NONE;
    thick->setDrawOutline(outline);
    emit sig_reconstructView();
}

void ThickEditor::slot_outlineWidthChanged(int width)
{
    auto thick = wthick.lock();
    if (!thick) return;

    qreal val = width/100.0;
    thick->setOutlineWidth(val);
    auto outline = thick->getDrawOutline();
    if (outline == OUTLINE_DEFAULT)
    {
        thick->setDrawOutline(OUTLINE_SET);
    }

    emit sig_reconstructView();
}

void ThickEditor::slot_outlineColor()
{
    auto thick = wthick.lock();
    if (!thick) return;

    QColor color = thick->getOutlineColor();

    AQColorDialog dlg(color,setable);
    dlg.setOption(QColorDialog::ShowAlphaChannel);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
    {
        return;
    }
    color = dlg.selectedColor();

    thick->setOutlineColor(color);

    outline_color->setBackground(color);  // menu

    emit sig_reconstructView();
}

////////////////////////////////////////////////////////////////////////////
// Filled
////////////////////////////////////////////////////////////////////////////
FilledEditor::FilledEditor(StylePtr style) : StyleEditor()
{
    auto filled  = std::dynamic_pointer_cast<Filled>(style);
    wfilled      = filled;
    Q_ASSERT(wfilled.lock());

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

    // GUI
    vbox    = new QVBoxLayout();
    hbox    = new QHBoxLayout();

    QLabel * label = new QLabel("Algorithm");
    label->setMinimumHeight(31);

    algoBox = new QComboBox();
    algoBox->setMinimumHeight(31);
    algoBox->addItem("Original: two face sets, two color sets",FILL_ORIGINAL);
    algoBox->addItem("New 1: two face sets, two color sets",FILL_TWO_FACE);
    algoBox->addItem("New 2: multi face sets, one color each",FILL_MULTI_FACE);
    algoBox->addItem("New 3: multi face sets, color sets for each",FILL_MULTI_FACE_MULTI_COLORS);
    algoBox->addItem("New 4: color per face",FILL_DIRECT_FACE);

    eFillType algo  = filled->getAlgorithm();
    int index       = algoBox->findData(algo);
    algoBox->setCurrentIndex(index);

    connect(algoBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){ slot_algo(index);} );

    hbox->addStretch();
    hbox->addWidget(label);
    hbox->addWidget(algoBox);
    hbox->addStretch();

    QVBoxLayout * vbox2 = new QVBoxLayout();
    vbox2->addLayout(hbox);
    vbox2->addLayout(vbox);
    this->setLayout(vbox2);

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

    auto filled = wfilled.lock();
    if (!filled) return;

    eFillType algo  = filled->getAlgorithm();
    int index       = algoBox->findData(algo);
    algoBox->setCurrentIndex(index);

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
        panel->setStatus("Click palette color in table, then lef-click mosaic faces (right-click to erase)");
        displayParmsPalette();
        break;
    }

    updateGeometry();
}

void FilledEditor::displayParms1()
{
    auto filled = wfilled.lock();
    if (!filled) return;

    fillOriginal = new StyleColorFillOriginal(this, filled,vbox);
    fillOriginal->display();
}

void FilledEditor::displayParms2()
{
    auto filled = wfilled.lock();
    if (!filled) return;

    fillSet = new StyleColorFillSet(filled,vbox);
    connect(fillSet, &StyleColorFillSet::sig_colorsChanged,     this, &FilledEditor::slot_colorsChanged,     Qt::UniqueConnection);
    fillSet->display();
}

void FilledEditor::displayParms3()
{
    auto filled = wfilled.lock();
    if (!filled) return;

    fillGroup = new StyleColorFillGroup(filled,vbox);
    connect(fillGroup, &StyleColorFillGroup::sig_colorsChanged,  this, &FilledEditor::slot_colorsChanged,     Qt::UniqueConnection);
    fillGroup->display();
}

void FilledEditor::displayParmsPalette()
{
    auto filled = wfilled.lock();
    if (!filled) return;

    fillFaces = new StyleColorFillFace(this, filled,vbox);
    fillFaces->display();
}

void FilledEditor::slot_algo(int index)
{
    auto filled = wfilled.lock();
    if (!filled) return;

    filled->setAlgorithm(eFillType(index));
    filled->resetStyleRepresentation();
    filled->createStyleRepresentation();
    displayParms();
    emit sig_reconstructView();
}

void FilledEditor::slot_editB()
{
    auto filled = wfilled.lock();
    if (!filled) return;

    qDebug() << "DLG B" << filled.get() << filled->getBlackColorSet();

    DlgColorSet dlg(filled->getBlackColorSet());

    connect(&dlg, &DlgColorSet::sig_dlg_colorsChanged, this, &FilledEditor::slot_colorsChanged, Qt::QueuedConnection);

    dlg.exec();

    displayParms();
}

void FilledEditor::slot_editW()
{
    auto filled = wfilled.lock();
    if (!filled) return;

    qDebug() << "DLG W" << filled.get() << filled->getWhiteColorSet();

    DlgColorSet dlg(filled->getWhiteColorSet());

    connect(&dlg, &DlgColorSet::sig_dlg_colorsChanged, this, &FilledEditor::slot_colorsChanged, Qt::QueuedConnection);

    dlg.exec();

    displayParms();
}

void FilledEditor::slot_colorsChanged()
{
    displayParms();
    emit sig_updateView();     // that's all
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
    auto filled = wfilled.lock();
    if (!filled) return;

    if (filled->getAlgorithm() == FILL_DIRECT_FACE && fillFaces)
    {
        fillFaces->onRefresh();
    }
}

void FilledEditor::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
{
    auto filled = wfilled.lock();
    if (!filled) return;

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
EmbossEditor::EmbossEditor(StylePtr style) : ThickEditor(style)
{
    auto emboss = std::dynamic_pointer_cast<Emboss>(style);
    wemboss     = emboss;
    Q_ASSERT(wemboss.lock());

    if (!setable)
    {
        setable = new AQTableWidget(this);
        AQVBoxLayout * vbox = new AQVBoxLayout();
        vbox->addWidget(setable);
        setLayout(vbox);
    }

    setable->setRowCount(rows + 1);

    QTableWidgetItem * item;

    item = new QTableWidgetItem("Azimuth Angle");
    setable->setItem(rows,0,item);

    qreal angle = emboss->getAngle() * 180.0 / M_PI;
    int iangle = static_cast<int>(angle);
    angle_slider = new SliderSet("", iangle, 0, 360);
    QWidget * widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(angle_slider);
    setable->setCellWidget(rows,1,widget);
    setable->setRowHeight(rows,ROW_HEIGHT);

    rows++;

    connect(angle_slider, &SliderSet::valueChanged,        this, &EmbossEditor::slot_anlgeChanged);
    connect(this,         &StyleEditor::sig_colorsChanged, this, &EmbossEditor::slot_colorsChanged);

    setable->resizeColumnsToContents();
    setable->adjustTableSize();
}

void EmbossEditor::slot_anlgeChanged(int angle)
{
    auto emboss = wemboss.lock();
    if (!emboss) return;

    qDebug() << "angle=" << angle;
    emboss->setAngle( angle * M_PI / 180.0 );
    emit sig_reconstructView();
}

void EmbossEditor::slot_colorsChanged()
{
    auto emboss = wemboss.lock();
    if (!emboss) return;

    emboss->resetStyleRepresentation();
}

////////////////////////////////////////////////////////////////////////////
// Interlaced
////////////////////////////////////////////////////////////////////////////

InterlaceEditor::InterlaceEditor(StylePtr style) : ThickEditor(style)
{
    auto interlace =  std::dynamic_pointer_cast<Interlace>(style);
    winterlace     = interlace;
    Q_ASSERT(winterlace.lock());

    if (!setable)
    {
        setable = new AQTableWidget(this);
        AQVBoxLayout * vbox = new AQVBoxLayout();
        vbox->addWidget(setable);
        setLayout(vbox);
    }

    setable->setRowCount(rows + 4);

    QTableWidgetItem * item;

    item = new QTableWidgetItem("Gap Width");
    setable->setItem(rows,0,item);

    qreal gap = interlace->getGap();
    gap_slider = new DoubleSliderSet("", gap, 0.0, 1.0, 100);
    QWidget * widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(gap_slider);
    setable->setCellWidget(rows,1,widget);
    setable->setRowHeight(rows,ROW_HEIGHT);

    rows++;
    item = new QTableWidgetItem("Shadow Width");
    setable->setItem(rows,0,item);

    qreal shadow = interlace->getShadow();
    shadow_slider = new DoubleSliderSet("", shadow, 0.0, 0.7, 100);
    widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(shadow_slider);
    setable->setCellWidget(rows,1,widget);
    setable->setRowHeight(rows,ROW_HEIGHT);

    rows++;
    item = new QTableWidgetItem("Start Under");
    setable->setItem(rows,0,item);

    sunder_checkbox = new AQCheckBox();
    sunder_checkbox->setChecked(interlace->getInitialStartUnder());
    setable->setCellWidget(rows,1,sunder_checkbox);
    setable->resizeColumnToContents(0);
    setable->adjustTableSize();

    rows++;
    item = new QTableWidgetItem("Include Tip Vertices");
    setable->setItem(rows,0,item);

    tipVert_checkbox = new AQCheckBox();
    tipVert_checkbox->setChecked(interlace->getIncludeTipVertices());
    setable->setCellWidget(rows,1,tipVert_checkbox);
    setable->resizeColumnToContents(0);
    setable->adjustTableSize();

    rows++;

    connect(gap_slider,       &DoubleSliderSet::valueChanged,  this, &InterlaceEditor::slot_gapChanged);
    connect(shadow_slider,    &DoubleSliderSet::valueChanged,  this, &InterlaceEditor::slot_shadowChanged);
    connect(sunder_checkbox,  CBOX_STATECHANGE,                this, &InterlaceEditor::slot_startUnderChanged);
    connect(tipVert_checkbox, CBOX_STATECHANGE,                this, &InterlaceEditor::slot_includeTipVerticesChanged);
    connect(this,             &StyleEditor::sig_colorsChanged, this, &InterlaceEditor::slot_colorsChanged);

    setable->resizeColumnsToContents();
    setable->adjustTableSize();
}

void InterlaceEditor::slot_gapChanged(qreal gap)
{
    auto interlace = winterlace.lock();
    if (!interlace) return;

    interlace->setGap(gap);
    interlace->resetStyleRepresentation();
    emit sig_reconstructView();
}

void InterlaceEditor::slot_shadowChanged(qreal shadow)
{
    auto interlace = winterlace.lock();
    if (!interlace) return;

    interlace->setShadow(shadow);
    interlace->resetStyleRepresentation();
    emit sig_reconstructView();
}

void InterlaceEditor::slot_startUnderChanged(int state)
{
    Q_UNUSED(state)

    auto interlace = winterlace.lock();
    if (!interlace) return;

    interlace->setInitialStartUnder(sunder_checkbox->isChecked());
    interlace->resetStyleRepresentation();
    emit sig_reconstructView();
}

void InterlaceEditor::slot_includeTipVerticesChanged(int state)
{
    Q_UNUSED(state)

    auto interlace = winterlace.lock();
    if (!interlace) return;

    interlace->setIncludeTipVertices(tipVert_checkbox->isChecked());
    interlace->resetStyleRepresentation();
    emit sig_reconstructView();
}

void InterlaceEditor::slot_colorsChanged()
{
    auto interlace = winterlace.lock();
    if (!interlace) return;

    interlace->resetStyleRepresentation();
}

///////////////////////////////////////////////////////////////
///   TileColors
///////////////////////////////////////////////////////////////
TileColorsEditor::TileColorsEditor(StylePtr style) : StyleEditor()
{
    auto tileColors = std::dynamic_pointer_cast<TileColors>(style);
    wtilecolors     = tileColors;
    Q_ASSERT(wtilecolors.lock());

    wtiling = tileColors->getTiling();

    config  = Sys::config;
    panel   = Sys::controlPanel;

    if (!setable)
    {
        setable = new AQTableWidget(this);
        AQVBoxLayout * vbox = new AQVBoxLayout();
        vbox->addWidget(setable);
        setLayout(vbox);
    }

    buildTable();
}

void TileColorsEditor::buildTable()
{
    qDebug() << "TileColorsEditor::buildTable()";

    auto tileColors = wtilecolors.lock();
    if (!tileColors) return;

    tileColors->createStyleRepresentation();  // this sets up the colors if needed

    setable->clear();
    setable->setColumnCount(4);
    setable->setColumnWidth(TILE_COLORS_ADDR,  100);
    setable->setColumnWidth(TILE_COLORS_SIDES, 130);
    setable->setColumnWidth(TILE_COLORS_BTN,   100);
    setable->setColumnWidth(TILE_COLORS_COLORS,400);
    setable->setSelectionMode(AQTableWidget::NoSelection);

    QStringList qslH;
    qslH << "Tile" << "Sides" << "Btn" << "Colors" ;
    setable->setHorizontalHeaderLabels(qslH);
    setable->verticalHeader()->setVisible(false);

    QVector<TilePtr> tiles;
    auto tiling = wtiling.lock();
    if (tiling)
    {
        tiles = tiling->getUniqueTiles();
        for (auto & tile : std::as_const(tiles))
        {
            uniqueTiles.push_back(tile);
        }

        setable->setRowCount(uniqueTiles.size() + 1);
    }

    QColor color;
    int   width;;
    bool outline = tileColors->getOutline(color,width);

    int row = 0;

    outline_checkbox = new AQCheckBox("Outline");
    outline_checkbox->setChecked(outline);
    setable->setCellWidget(row,0,outline_checkbox);

    color_button = new QPushButton("Color");
    setable->setCellWidget(row,2,color_button);

    colorItem = new QTableWidgetItem();
    colorItem->setBackground(color);
    setable->setItem(row,1,colorItem);

    width_slider = new SliderSet("Width", width, 1, 10);
    QWidget * widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(width_slider);
    setable->setCellWidget(row,3,widget);
    setable->setRowHeight(row,ROW_HEIGHT);

    row++;

    connect(outline_checkbox,   CBOX_STATECHANGE,           this, &TileColorsEditor::slot_outlineChanged);
    connect(color_button,       &QPushButton::clicked,      this, &TileColorsEditor::slot_outline_color);
    connect(width_slider,       &SliderSet::valueChanged,   this, &TileColorsEditor::slot_widthChanged);

    for (auto & tile : std::as_const(tiles))
    {
        QTableWidgetItem * twi = new QTableWidgetItem(Utils::addr(tile.get()));
        setable->setItem(row,TILE_COLORS_ADDR,twi);

        QString str = QString("%1 %2 num=%3").arg(tile->numPoints()).arg((tile->isRegular()) ? "Regular" : "Not-regular").arg(tiling->numPlacements(tile));
        twi = new QTableWidgetItem(str);
        setable->setItem(row,TILE_COLORS_SIDES,twi);

        QPushButton * btn = new QPushButton("Edit");
        setable->setCellWidget(row,TILE_COLORS_BTN,btn);
        connect(btn, &QPushButton::clicked, this, &TileColorsEditor::slot_edit);

        ColorSet * cset = tileColors->getColorSet(tile);
        QWidget * widget = cset->createWidget();
        setable->setCellWidget(row,TILE_COLORS_COLORS,widget);

        row++;
    }

    setable->resizeColumnsToContents();
    setable->adjustTableSize();
    //table->selectRow(0);
}

void TileColorsEditor::slot_edit()
{
    auto tileColors = wtilecolors.lock();
    if (!tileColors) return;

    int row = setable->currentRow();
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

    DlgColorSet dlg(tileColors->getColorSet(tile),setable);
    connect(&dlg, &DlgColorSet::sig_dlg_colorsChanged, this, &::TileColorsEditor::slot_colors_changed);

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
    auto tileColors = wtilecolors.lock();
    if (!tileColors) return;

     bool checked = (state == Qt::Checked);

     QColor color;
     int width;

     tileColors->getOutline(color,width);
     tileColors->setOutline(checked,color,width);

     emit sig_reconstructView();
}

void  TileColorsEditor::slot_outline_color()
{
    auto tileColors = wtilecolors.lock();
    if (!tileColors) return;

    QColor color;
    int width;
    bool outlineEnb = tileColors->getOutline(color,width);

    AQColorDialog dlg(color,setable);
    dlg.setOption(QColorDialog::ShowAlphaChannel);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
    {
        return;
    }
    color = dlg.selectedColor();

    tileColors->setOutline(outlineEnb,color,width);

    colorItem->setBackground(color);

    emit sig_reconstructView();

}

void TileColorsEditor::slot_widthChanged(int val)
{
    auto tileColors = wtilecolors.lock();
    if (!tileColors) return;

    QColor color;
    int width;
    bool outlineEnb = tileColors->getOutline(color,width);
    tileColors->setOutline(outlineEnb,color,val);

    emit sig_reconstructView();
}
