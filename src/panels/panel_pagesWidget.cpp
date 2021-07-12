/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "panels/panel_pagesWidget.h"
#include "panels/panel_page.h"

PanelPagesWidget::PanelPagesWidget()
{
    //setMinimumSize(300,400);
    //setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);

    currentPage = nullptr;
}

PanelPagesWidget::~PanelPagesWidget()
{
#if 0
    while (mpStackedWidget->count())
    {
        mpStackedWidget->removeWidget (mpStackedWidget->widget(0));
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
