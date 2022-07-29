#include <QHeaderView>
#include <QComboBox>
#include <QCheckBox>

#include "makers/mosaic_maker/style_editors.h"
#include "misc/utilities.h"
#include "viewers/viewcontrol.h"
#include "widgets/dlg_colorSet.h"
#include "makers/mosaic_maker/style_color_fill_group.h"
#include "makers/mosaic_maker//style_color_fill_set.h"
#include "settings/configuration.h"
#include "panels/panel.h"
#include "tile/tiling.h"
#include "tile/feature.h"
#include "widgets/layout_sliderset.h"
#include "style/colored.h"
#include "style/emboss.h"
#include "style/filled.h"
#include "style/interlace.h"
#include "style/tile_colors.h"

#define ROW_HEIGHT 39

StyleEditor::StyleEditor()
{
    ViewControl * vcontrol = ViewControl::getInstance();

    connect(this, &StyleEditor::sig_refreshView,  vcontrol, &ViewControl::slot_refreshView);
    connect(this, &StyleEditor::sig_updateView,   vcontrol, &ViewControl::slot_updateView);
}

///////////////////////////////////////////////////////////////
///   Colored
///////////////////////////////////////////////////////////////

ColoredEditor::ColoredEditor(Colored * c, AQTableWidget * table)
{
    colored = c;
    this->table = table;

    TPColor tpcolor = colored->getColorSet()->getFirstColor();
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

    outline_checkbox = new QCheckBox();
    outline_checkbox->setStyleSheet("padding-left:21px;");
    auto outline = thick->getDrawOutline();
    if (outline == OUTLINE_NONE)
        outline_checkbox->setChecked(false);
    else
        outline_checkbox->setChecked(true);
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
    connect(join_style, QOverload<int>::of(&QComboBox::currentIndexChanged), [=]()
            { thick->setJoinStyle(static_cast<Qt::PenJoinStyle>(join_style->currentData().toInt())); emit sig_refreshView(); } );
    connect(cap_style,  QOverload<int>::of(&QComboBox::currentIndexChanged), [=]()
            { thick->setCapStyle(static_cast<Qt::PenCapStyle>(cap_style->currentData().toInt())); emit sig_refreshView(); } );
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
    filled      = f;
    this->table = table;
    vbox        = parmsCtrl;
    fillSet     = nullptr;
    fillGroup   = nullptr;
    view        = ViewControl::getInstance();

    if (!filled->dcel)
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

    // clear the table
    table->clearContents();
    table->setColumnCount(2);
    table->setRowCount(1);

    int algo         = filled->getAlgorithm();
    int row          = 0;

    // algorithm
    QTableWidgetItem * item = new QTableWidgetItem("Algorithm");
    table->setItem(row,0,item);

    QComboBox * algoBox = new QComboBox();
    algoBox->addItem("Original: two face sets, two color sets",0);
    algoBox->addItem("New 1: two face sets, two color sets",1);
    algoBox->addItem("New 2: multi face sets, one color each",2);
    algoBox->addItem("New 3: multi face sets, color sets for each",3);
    table->setCellWidget(row,1,algoBox);

    int index = algoBox->findData(algo);
    algoBox->setCurrentIndex(index);
    connect(algoBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){ slot_algo(index);} );

    switch (algo)
    {
    case 0:
    case 1:
        displayParms01();
        break;
    case 2:
        displayParms2();
        break;
    case 3:
        displayParms3();
        break;
    }

    table->adjustTableSize();
    table->update();
}

void FilledEditor::displayParms01()
{
    int row = 0;

    table->setRowCount(3);
    table->setColumnCount(4);

    QTableWidgetItem * item = new QTableWidgetItem(QString("Number"));
    table->setItem(row,3,item);
    row++;

    // whites
    outside_checkbox = new QCheckBox("Inside (Whites)");   // is truth but does not match code
    outside_checkbox->setStyleSheet("padding-left:3px;");
    outside_checkbox->setChecked(filled->getDrawOutsideWhites());
    table->setCellWidget(row,0,outside_checkbox);

    item = new QTableWidgetItem(QString("%1 faces").arg(filled->whiteFaces.size()));
    table->setItem(row,3,item);

    ColorSet * colorSetW    = filled->getWhiteColorSet();
    AQWidget * widget       = colorSetW->createWidget();
    table->setCellWidget(row,1,widget);

    QPushButton * btnW = new QPushButton("Edit");
    table->setCellWidget(row,2,btnW);

    row++;

    // blacks
    inside_checkbox = new QCheckBox("Outside (Blacks)");   // is truth but does not match code
    inside_checkbox->setStyleSheet("padding-left:3px;");
    inside_checkbox->setChecked(filled->getDrawInsideBlacks());
    table->setCellWidget(row,0,inside_checkbox);

    item = new QTableWidgetItem(QString("%1 faces").arg(filled->blackFaces.size()));
    table->setItem(row,3,item);

    ColorSet * colorSetB    = filled->getBlackColorSet();
    widget                  = colorSetB->createWidget();
    table->setCellWidget(row,1,widget);

    QPushButton * btnB = new QPushButton("Edit");
    table->setCellWidget(row,2,btnB);

    connect(inside_checkbox,  &QCheckBox::stateChanged, this, &FilledEditor::slot_insideChanged);
    connect(outside_checkbox, &QCheckBox::stateChanged, this, &FilledEditor::slot_outsideChanged);
    connect(btnB,             &QPushButton::clicked,    this, &FilledEditor::slot_editB);
    connect(btnW,             &QPushButton::clicked,    this, &FilledEditor::slot_editW);

    table->setColumnWidth(0,131);
    table->setColumnWidth(1,410);
    table->setColumnWidth(2,120);
    table->setColumnWidth(3,120);
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

void FilledEditor::slot_insideChanged(int state)
{
    bool checked = (state == Qt::Checked);
    filled->setDrawInsideBlacks(checked);

    slot_colorsChanged();
}

void FilledEditor::slot_outsideChanged(int state)
{
    bool checked = (state == Qt::Checked);
    filled->setDrawOutsideWhites(checked);

    slot_colorsChanged();
}

void FilledEditor::slot_algo(int index)
{
    filled->setAlgorithm(index);
    filled->resetStyleRepresentation();
    filled->createStyleRepresentation();
    displayParms();
    emit sig_updateView();
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
    view->update();     // that's all
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

    sunder_checkbox = new QCheckBox();
    sunder_checkbox->setStyleSheet("padding-left:21px;");
    sunder_checkbox->setChecked(i->getInitialStartUnder());
    table->setCellWidget(rows,1,sunder_checkbox);
    table->resizeColumnToContents(0);
    table->adjustTableSize();

    rows++;
    item = new QTableWidgetItem("Include Tip Vertices");
    table->setItem(rows,0,item);

    tipVert_checkbox = new QCheckBox();
    tipVert_checkbox->setStyleSheet("padding-left:21px;");
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
    config = Configuration::getInstance();
    panel   = ControlPanel::getInstance();

    colored     = c;
    this->table = table;
    this->tiling = tiling;
    buildTable();
}

void TileColorsEditor::buildTable()
{
    table->clear();
    table->setColumnCount(4);
    table->setColumnWidth(TILE_COLORS_ADDR,  100);
    table->setColumnWidth(TILE_COLORS_SIDES, 130);
    table->setColumnWidth(TILE_COLORS_BTN,   100);
    table->setColumnWidth(TILE_COLORS_COLORS,400);

    QStringList qslH;
    qslH << "Feature" << "Sides" << "Btn" << "Colors" ;
    table->setHorizontalHeaderLabels(qslH);
    table->verticalHeader()->setVisible(false);

    qlfp = tiling->getUniqueFeatures();
    table->setRowCount(qlfp.size() + 1);

    QColor color;
    int width;;
    bool outline = colored->getOutline(color,width);

    int row = 0;

    outline_checkbox = new QCheckBox("Outline");
    outline_checkbox->setStyleSheet("padding-left:21px;");
    outline_checkbox->setChecked(outline);
    table->setCellWidget(row,0,outline_checkbox);

    color_button = new QPushButton("Color");
    table->setCellWidget(row,2,color_button);

    colorItem = new QTableWidgetItem();
    colorItem->setBackground(color);
    table->setItem(row,1,colorItem);

    width_slider = new SliderSet("Width", width, 1, 10);
    AQWidget * widget = new AQWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(width_slider);
    table->setCellWidget(row,3,widget);
    table->setRowHeight(row,ROW_HEIGHT);

    row++;

    connect(outline_checkbox,   &QCheckBox::stateChanged,   this, &TileColorsEditor::slot_outlineChanged);
    connect(color_button,       &QPushButton::clicked,      this, &TileColorsEditor::slot_outline_color);
    connect(width_slider,       &SliderSet::valueChanged,   this, &TileColorsEditor::slot_widthChanged);

    for (auto it = qlfp.begin(); it != qlfp.end(); it++)
    {
        FeaturePtr fp = *it;

        QTableWidgetItem * twi = new QTableWidgetItem(Utils::addr(fp.get()));
        table->setItem(row,TILE_COLORS_ADDR,twi);

        QString str = QString("%1 %2 num=%3").arg(fp->numPoints()).arg((fp->isRegular()) ? "Regular" : "Not-regular").arg(tiling->numPlacements(fp));
        twi = new QTableWidgetItem(str);
        table->setItem(row,TILE_COLORS_SIDES,twi);

        QPushButton * btn = new QPushButton("Edit");
        table->setCellWidget(row,TILE_COLORS_BTN,btn);
        connect(btn, &QPushButton::clicked, this, &TileColorsEditor::slot_edit);

        AQWidget * widget = fp->getFeatureColors()->createWidget();
        table->setCellWidget(row,TILE_COLORS_COLORS,widget);

        row++;
    }
    table->adjustTableSize();
    table->selectRow(0);
}

void TileColorsEditor::slot_edit()
{
    int row = table->currentRow();
    row--;  // first row is outline

    if (row < 0 || row >= (qlfp.size() ))
        return;

    FeaturePtr fp = qlfp[row];
    DlgColorSet dlg(fp->getFeatureColors(),table);

    connect(&dlg, &DlgColorSet::sig_colorsChanged, this, &TileColorsEditor::slot_colors_changed);

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

     colored->getOutline(color,width);
     colored->setOutline(checked,color,width);

     emit sig_refreshView();
}

void  TileColorsEditor::slot_outline_color()
{
    QColor color;
    int width;
    bool outlineEnb = colored->getOutline(color,width);

    AQColorDialog dlg(color,table);
    dlg.setOption(QColorDialog::ShowAlphaChannel);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
    {
        return;
    }
    color = dlg.selectedColor();

    colored->setOutline(outlineEnb,color,width);

    colorItem->setBackground(color);

    emit sig_refreshView();

}

void TileColorsEditor::slot_widthChanged(int val)
{
    QColor color;
    int width;
    bool outlineEnb = colored->getOutline(color,width);
    colored->setOutline(outlineEnb,color,val);

    emit sig_refreshView();
}
