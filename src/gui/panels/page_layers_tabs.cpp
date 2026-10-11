#include "gui/panels/page_layers_tabs.h"
#include "gui/panels/page_layer_alignmentl.h"
#include "gui/panels/page_layers.h"
#include "gui/top/controlpanel.h"
#include "gui/widgets/floatable_tab.h"
#include "model/settings/configuration.h"
#include "sys/sys.h"

page_layers_tabs::page_layers_tabs(ControlPanel * cpanel) : panel_page(cpanel,PAGE_LAYERS,"Layers")
{
    alignPage = new page_layer_algnment(cpanel);
    alignTab  = createTab(alignPage);

    infoPage  = nullptr;
    infoTab   = nullptr;
    if (config->insightMode)
    {
        infoPage = new page_layers(cpanel);
        infoTab  = createTab(infoPage);
    }

    setMinimumWidth(PANEL_RHS_WIDTH);

    tabWidget = new QTabWidget;
    if (infoTab)
        tabWidget->addTab(infoTab, "Layer Info");
    tabWidget->addTab(alignTab, "Layer Align");

    vbox->addWidget(tabWidget);
    vbox->addStretch();

    connect(tabWidget, &QTabWidget::tabBarDoubleClicked, this, &page_layers_tabs::slot_detach);

    QSettings s = Sys::settings();

    // detach in descending tab-index order so removing one tab doesn't shift the index of another not-yet-processed tab
    QString name = QString("panel2/%1/float_align_tab").arg(pageName);
    if (s.value(name,false).toBool())
        slot_detach(tabWidget->indexOf(alignTab));

    name = QString("panel2/%1/float_info_tab").arg(pageName);
    if (infoTab && s.value(name,false).toBool())
        slot_detach(0);
}

page_layers_tabs::~page_layers_tabs()
{
    QSettings s = Sys::settings();

    QString name = QString("panel2/%1/float_align_tab").arg(pageName);
    s.setValue(name,alignTab->floating);
    if (alignTab->floating)
        alignTab->savePosition();
    alignTab->floating = false;   // so it closes correctly

    if (infoTab)
    {
        name = QString("panel2/%1/float_info_tab").arg(pageName);
        s.setValue(name,infoTab->floating);
        if (infoTab->floating)
            infoTab->savePosition();
        infoTab->floating = false;   // so it closes correctly
    }
}

FloatableTab * page_layers_tabs::createTab(panel_page * page)
{
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(page);
    layout->addStretch();

    FloatableTab * tab = new FloatableTab(this);
    tab->setLayout(layout);
    return tab;
}

void page_layers_tabs::slot_detach(int index)
{
    QWidget * widget = tabWidget->widget(index);
    QString title    = tabWidget->tabText(index);

    tabWidget->removeTab(index);

    FloatableTab * floater = dynamic_cast<FloatableTab*>(widget);
    Q_ASSERT(floater);
    floater->detach(tabWidget,title);
}

void page_layers_tabs::onEnter()
{
    alignPage->onEnter();
    if (infoPage)
        infoPage->onEnter();
}

void page_layers_tabs::onExit()
{
    alignPage->onExit();
    if (infoPage)
        infoPage->onExit();
}

void page_layers_tabs::onRefresh()
{
    alignPage->onRefresh();
    if (infoPage)
        infoPage->onRefresh();
}
