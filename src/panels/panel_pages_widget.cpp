#include "panels/panel_pages_widget.h"
#include "panels/panel_page.h"
#include "panels/panel_misc.h"

PanelPagesWidget::PanelPagesWidget()
{
    setContentsMargins(0,0,0,0);
}

PanelPagesWidget::~PanelPagesWidget()
{}

void PanelPagesWidget::setCurrentPage(panel_page * pp)
{
    AQVBoxLayout * aLayout = new AQVBoxLayout();
    aLayout->setSizeConstraint(QLayout::SetFixedSize);
    aLayout->addWidget(pp);

    QLayout * l = layout();
    if (l)
    {
        QLayoutItem * item;
        while ( (item = l->itemAt(0)) != nullptr)
        {
            QWidget * w = item->widget();
            if (w)
            {
                w->setParent(nullptr);
            }
        }
        delete l;
    }
    setLayout(aLayout);
    update();
}
