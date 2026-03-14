#include "gui/widgets/colorset_widget.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view_controller.h"
#include "gui/model_editors/style_edit/style_editor.h"
#include "gui/widgets/panel_misc.h"
#include "model/styles/colorset.h"
#include "sys/sys.h"

////////////////////////////////////////////////////////
///
///  Color Set Widget
///
////////////////////////////////////////////////////////

ColorSetWidget::ColorSetWidget(StyleEditor * parent, ColorSet * cset)
{
    group        = nullptr;
    this->cset   = cset;
    this->parent = parent;

    hbox         = new AQHBoxLayout;
    setLayout(hbox);

    connect(this, &ColorSetWidget::sig_updateView, Sys::viewController, &SystemViewController::slot_updateView);
    updateFromColorSet();
}

void ColorSetWidget::updateFromColorSet()
{
    if (!group)
    {
        buildGroup();
    }
    else if (group->buttons().size() != cset->size())
    {
        buildGroup();
    }
    else
    {
        refreshGroup();
    }
}

void ColorSetWidget::buildGroup()
{
    if (group)
    {
        for (QAbstractButton* b : group->buttons())
        {
            delete b;  // removes from layout automatically
        }

        delete group;
    }

    group = new QButtonGroup(this);
    connect(group, &QButtonGroup::idClicked, this, &ColorSetWidget::colorButtonClicked);

    int index = 0;
    for (auto & tpcolor :  *cset)
    {
        QColor color     = tpcolor.color;
        QColor fullColor = color;
        fullColor.setAlpha(255);

        QPushButton * btn = new QPushButton();
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        QVariant variant = fullColor;
        QString colcode  = variant.toString();
        btn->setStyleSheet("QPushButton { background-color :"+colcode+" ; border: 1px solid black;}");

        hbox->addWidget(btn);
        group->addButton(btn,index);

        index++;
    }
}

void ColorSetWidget::refreshGroup()
{
    int index = 0;
    for (auto & tpcolor :  *cset)
    {
        QColor color     = tpcolor.color;
        QColor fullColor = color;
        fullColor.setAlpha(255);

        auto * btn = group->button(index);

        QVariant variant = fullColor;
        QString colcode  = variant.toString();
        btn->setStyleSheet("QPushButton { background-color :"+colcode+" ; border: 1px solid black;}");

        index++;
    }
}

void ColorSetWidget::colorButtonClicked(int id)
{
    // changes a single colour
    TPColor * tpc = cset->data() + id;

    AQColorDialog dlg(tpc->color,Sys::controlPanel);
    dlg.setCurrentColor(tpc->color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    QColor acolor = dlg.selectedColor();
    if (acolor.isValid())
    {
        tpc->color = acolor;
        parent->notify();
    }
}
