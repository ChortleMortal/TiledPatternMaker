#include "widgets/panel_pagesWidget.h"
#include "widgets/panel_page.h"

PanelPagesWidget::PanelPagesWidget()
{
    //setMinimumSize(300,400);
    //setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);

    currentPage = nullptr;
}

PanelPagesWidget::~PanelPagesWidget()
{
#if 0
    for (auto page : pages.values())
    {
        if (page)
            delete page;
    }
#endif
}

void PanelPagesWidget::addWidget(panel_page * page)
{
     pages[page->getName()] = page;
}

panel_page *PanelPagesWidget::setCurrentPage(QString name)
{
    panel_page * pp = pages.value(name);
    if (pp)
    {
        setCurrentPage(pp);
    }
    return pp;
}

void PanelPagesWidget::setCurrentPage(panel_page * pp)
{
    currentPage = pp;

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
    pp->adjustSize();
    adjustSize();
    repaint();
}
