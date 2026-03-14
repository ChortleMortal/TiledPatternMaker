#include "gui/model_editors/style_edit/emboss_editor.h"
#include "gui/widgets/panel_misc.h"
#include "gui/widgets/layout_sliderset.h"
#include "model/styles/emboss.h"

#define ROW_HEIGHT 39

////////////////////////////////////////////////////////////////////////////
///
///  Embossed
///
////////////////////////////////////////////////////////////////////////////
EmbossEditor::EmbossEditor(StylePtr style,eStyleType user) : ThickEditor(style,user)
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

    connect(angle_slider, &SliderSet::valueChanged, this, &EmbossEditor::slot_anlgeChanged);

    setable->resizeColumnsToContents();
    setable->adjustTableSize();
}

void EmbossEditor::slot_colorsChanged()
{
    auto emboss = wemboss.lock();
    if (!emboss) return;

    ColorSet cset = *emboss->getColorSet();  // makes a copy
    emboss->setColorSet(&cset);              // triggers writing greys

    ColoredEditor::slot_colorsChanged();
    emit sig_updateView();
}

void EmbossEditor::slot_opacityChanged(qreal val)
{
    ColoredEditor::slot_opacityChanged(val);

    auto emboss = wemboss.lock();
    if (!emboss) return;

    ColorSet cset = *emboss->getColorSet();  // makes a copy
    emboss->setColorSet(&cset);              // triggers writing greys

    emit sig_updateView();
}

void EmbossEditor::slot_anlgeChanged(int angle)
{
    auto emboss = wemboss.lock();
    if (!emboss) return;

    qDebug() << "angle=" << angle;
    emboss->setAngle( angle * M_PI / 180.0 );
    emboss->resetStyleRepresentation();
    emboss->createStyleRepresentation();
    emit sig_updateView();
}

