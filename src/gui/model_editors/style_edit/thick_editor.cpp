#include "gui/model_editors/style_edit/thick_editor.h"
#include "gui/widgets/panel_misc.h"
#include "gui/widgets/layout_sliderset.h"
#include "model/styles/thick.h"

#define ROW_HEIGHT 39

///////////////////////////////////////////////////////////////
///
///   Thick
///
///////////////////////////////////////////////////////////////

ThickEditor::ThickEditor(StylePtr style, eStyleType user) : ColoredEditor(style,user)
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

    connect(outline_checkbox,     &QCheckBox::clicked,      this, &ThickEditor::slot_outlineChanged);
    connect(width_slider,         &SliderSet::valueChanged, this, &ThickEditor::slot_widthChanged);
    connect(outline_width_slider, &SliderSet::valueChanged, this, &ThickEditor::slot_outlineWidthChanged);
    connect(outline_color_button, &QPushButton::clicked,    this, &ThickEditor::slot_outlineColor);
    connect(join_style,           &QComboBox::currentIndexChanged, this, &ThickEditor::slot_joinStyle);
    connect(cap_style,            &QComboBox::currentIndexChanged, this, &ThickEditor::slot_capStyle);

    setable->resizeColumnsToContents();
    setable->adjustTableSize();
}

void ThickEditor::slot_joinStyle(int index)
{
    Q_UNUSED(index);

    auto thick = wthick.lock();
    if (!thick) return;

    thick->setJoinStyle(static_cast<Qt::PenJoinStyle>(join_style->currentData().toInt()));

    emit sig_updateView();
}

void ThickEditor::slot_capStyle(int index)
{
    Q_UNUSED(index);

    auto thick = wthick.lock();
    if (!thick) return;

    thick->setCapStyle(static_cast<Qt::PenCapStyle>(cap_style->currentData().toInt()));

    emit sig_updateView();
}

void  ThickEditor::slot_widthChanged(int width)
{
    auto thick = wthick.lock();
    if (!thick) return;

    qreal val = width/100.0;
    thick->setLineWidth(val);

    notify();
}

void  ThickEditor::slot_outlineChanged(bool checked)
{
    auto thick = wthick.lock();
    if (!thick) return;

    eDrawOutline outline = (checked) ? OUTLINE_SET : OUTLINE_NONE;
    thick->setDrawOutline(outline);

    emit sig_updateView();
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

    emit sig_updateView();
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

    emit sig_updateView();
}
