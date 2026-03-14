#include <QHeaderView>
#include <QComboBox>
#include <QCheckBox>

#include "gui/model_editors/style_edit/colored_editor.h"
#include "gui/widgets/colorset_widget.h"
#include "gui/widgets/panel_misc.h"
#include "gui/widgets/dlg_colorSet.h"
#include "gui/widgets/layout_sliderset.h"
#include "model/styles/colored.h"

#define ROW_HEIGHT 39

///////////////////////////////////////////////////////////////
///
///   Colored
///
///////////////////////////////////////////////////////////////
ColoredEditor::ColoredEditor(StylePtr style, eStyleType user) : StyleEditor(style, user)
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

    colorwidget = new ColorSetWidget(this,colored->getColorSet());
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

    connect(color_button,  &QPushButton::clicked,          this, &ColoredEditor::slot_selectColor);
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

    colorwidget = new ColorSetWidget(this,cset);
    setable->setCellWidget(rows,1,colorwidget);

    notify();
}

void ColoredEditor::slot_selectColor()
{
    auto colored = wcolored.lock();
    if (!colored) return;

    DlgColorSet dlg(colored->getColorSet(),setable);

    connect(&dlg, &DlgColorSet::sig_dlg_colorsChanged, this, &ColoredEditor::slot_colorsChanged);

    dlg.exec();
}

void ColoredEditor::slot_colorsChanged()
{
    auto colored = wcolored.lock();
    if (!colored) return;

    colorwidget = new ColorSetWidget(this,colored->getColorSet());
    setable->setCellWidget(0,1,colorwidget);

    notify();
}


