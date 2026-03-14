#include "gui/model_editors/style_edit/interlace_editor.h"
#include "gui/widgets/panel_misc.h"
#include "gui/widgets/layout_sliderset.h"
#include "model/styles/interlace.h"

#define ROW_HEIGHT 39

////////////////////////////////////////////////////////////////////////////
///
///   Interlaced
///
////////////////////////////////////////////////////////////////////////////

InterlaceEditor::InterlaceEditor(StylePtr style, eStyleType user) : ThickEditor(style,user)
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
    connect(sunder_checkbox,  &QCheckBox::clicked,             this, &InterlaceEditor::slot_startUnderChanged);
    connect(tipVert_checkbox, &QCheckBox::clicked,             this, &InterlaceEditor::slot_includeTipVerticesChanged);

    setable->resizeColumnsToContents();
    setable->adjustTableSize();
}

void InterlaceEditor::slot_gapChanged(qreal gap)
{
    auto interlace = winterlace.lock();
    if (!interlace) return;

    interlace->setGap(gap);
    interlace->resetStyleRepresentation();
    interlace->createStyleRepresentation();
    emit sig_updateView();
}

void InterlaceEditor::slot_shadowChanged(qreal shadow)
{
    auto interlace = winterlace.lock();
    if (!interlace) return;

    interlace->setShadow(shadow);
    interlace->resetStyleRepresentation();
    interlace->createStyleRepresentation();
    emit sig_updateView();
}

void InterlaceEditor::slot_startUnderChanged(bool checked)
{
    auto interlace = winterlace.lock();
    if (!interlace) return;

    interlace->setInitialStartUnder(checked);
    interlace->resetStyleRepresentation();
    interlace->createStyleRepresentation();
    emit sig_updateView();
}

void InterlaceEditor::slot_includeTipVerticesChanged(bool checked)
{
    auto interlace = winterlace.lock();
    if (!interlace) return;

    interlace->setIncludeTipVertices(checked);
    interlace->resetStyleRepresentation();
    interlace->createStyleRepresentation();
    emit sig_updateView();
}

