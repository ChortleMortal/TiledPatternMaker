#include "gui/panels/page_backgrounds.h"
#include "gui/panels/page_borders.h"
#include "gui/panels/page_config.h"
#include "gui/panels/page_crop_maker.h"
#include "gui/panels/page_debug.h"
#include "gui/panels/page_grid.h"
#include "gui/panels/page_image_tools.h"
#include "gui/panels/page_layers.h"
#include "gui/panels/page_layer_alignmentl.h"
#include "gui/panels/page_loaders.h"
#include "gui/panels/page_log.h"
#include "gui/panels/page_map_editor.h"
#include "gui/panels/page_modelSettings.h"
#include "gui/panels/page_mosaic_info.h"
#include "gui/panels/page_mosaic_maker.h"
#include "gui/panels/page_motif_maker.h"
#include "gui/panels/page_prototype_info.h"
#include "gui/panels/page_save.h"
#include "gui/panels/page_system_info.h"
#include "gui/panels/page_tiling_maker.h"
#include "gui/panels/panel_page_controller.h"
#include "gui/panels/panel_pages_widget.h"
#include "gui/top/controlpanel.h"
#include "gui/top/split_screen.h"
#include "model/settings/configuration.h"
#include "sys/engine/image_engine.h"
#include "sys/tiledpatternmaker.h"

Q_DECLARE_METATYPE(panel_page *)

PanelPageController::PanelPageController(PageListWidget * pageListWidget, PanelPagesWidget * panelPagesWidget)
{
    pageList     = pageListWidget;
    pages        = panelPagesWidget;
    panel        = Sys::controlPanel;
    config       = Sys::config;
    updateLocked = false;

    connect (pageListWidget, &PageListWidget::sig_detachWidget, this,	&PanelPageController::slot_detachWidget, Qt::QueuedConnection);
}

PanelPageController::~PanelPageController()
{
    pageList->deletePages();
    pageList->clear();
}

void PanelPageController::populatePages()
{
    qDebug() << "PanelPageController::populatePages";

    panel_page * wp;

    wp = new page_loaders(panel);
    pageList->addPage(wp);

    wp = new page_save(panel);
    pageList->addPage(wp);

    pageList->addSeparator();

    wp = new page_mosaic_maker(panel);
    pageList->addPage(wp);

    pageList->addSeparator();

    wp = new page_motif_maker(panel);
    pageList->addPage(wp);

    pageList->addSeparator();

    wp = new page_tiling_maker(panel);
    pageList->addPage(wp);

    pageList->addSeparator();

    wp = new page_borders(panel);
    pageList->addPage(wp);

    wp = new page_crop_maker(panel);
    pageList->addPage(wp);

    wp = new page_backgrounds(panel);
    pageList->addPage(wp);

    wp = new page_modelSettings(panel);
    pageList->addPage(wp);

    wp = new page_grid(panel);
    pageList->addPage(wp);

    wp = new page_layer_algnment(panel);
    pageList->addPage(wp);

    if (config->insightMode)
    {
        pageList->addSeparator();

        wp = new page_map_editor(panel);
        pageList->addPage(wp);

        page_map_editor * wp_med = dynamic_cast<page_map_editor*>(wp);
        connect(panel,   &ControlPanel::sig_reload,  wp_med,    &page_map_editor::slot_mosaicChanged);

        pageList->addSeparator();

        wp = new page_mosaic_info(panel);
        pageList->addPage(wp);

        wp = new page_prototype_info(panel);
        pageList->addPage(wp);

        wp = new page_layers(panel);
        pageList->addPage(wp);

        wp = new page_system_info(panel);
        pageList->addPage(wp);
    }

    pageList->addSeparator();

    wp = new page_config(panel);
    pageList->addPage(wp);

    if (config->insightMode)
    {
        wp = new page_image_tools(panel);
        pageList->addPage(wp);

        wp = new page_log(panel);
        pageList->addPage(wp);

        page_log * pageLog = dynamic_cast<page_log*>(wp);
        connect(panel, &ControlPanel::sig_saveLog, pageLog, &page_log::slot_copyLog);

        wp = new page_debug(panel);
        pageList->addPage(wp);
    }

    pageList->adjustSize();

    connect(pageList, &PageListWidget::itemClicked,       this, &PanelPageController::slot_selectPanelPage);
    connect(pageList, &PageListWidget::itemDoubleClicked, this, &PanelPageController::slot_itemDetachPanelPage);

    pageList->establishSize();

    qDebug() << "PanelPageController::populatePages" << "- END";
}


void PanelPageController::refreshPages()
{
    if (updateLocked)
    {
        return;
    }

    updateLocked = true;
    pageList->refreshPages();
    updateLocked = false;
}

void PanelPageController::slot_floatPages()
{
    if (!Sys::enableDetachedPages)
    {
        return;
    }

    QStringList names = pageList->wereFloated();
    if (!names.isEmpty())
    {
        for (const auto & pagename : std::as_const(names))
        {
            slot_detachWidget(pagename);
        }
    }
}

void PanelPageController::slot_subAttachPage()
{
    QString pagename = pageList->wasSubAttached();
    if (!pagename.isEmpty())
    {
        slot_detachWidget(pagename);
    }
}

panel_page * PanelPageController::getPage(QString name)
{
    return pageList->getPage(name);
}

panel_page * PanelPageController::getCurrentPage()
{
    return pageList->getSelectedPage();
}

void  PanelPageController::setCurrentPage(QString name)
{
    qDebug().noquote() << "PanelPageController::setCurrentPage" << name;

    panel_page * oldpage = pageList->getSelectedPage();
    panel_page * newpage = pageList->getPage(name);

    // unselect previous
    if (oldpage)
    {
        PageData & data = pageList->getPageData(oldpage);
        data.selected = false;
    }

    // select new
    config->pageName = name;
    newpage->setNewlySelected(true);
    pages->setCurrentPage(newpage);
    pageList->setCurrentRow(name);
    PageData & data2 = pageList->getPageData(newpage);
    data2.selected = true;
}

void PanelPageController::closePages()
{
    pageList->closePages();
}

void PanelPageController::slot_detachWidget(QString name)
{
    qDebug() << "slot_detachWidget" << name;

    panel_page * page = pageList->getPage(name);
    if (page == nullptr)
    {
        qWarning("Page not found to detach");
        return;
    }

    page->clearPageStatus();

    updateLocked = true;

    if (config->splitScreen && config->bigScreen)
    {
        // this sub-attaches in the splitter
        Q_ASSERT(Sys::splitter);
        auto oldDetachPage = Sys::splitter->getFloater();
        if (oldDetachPage)
        {
            // replace the old sub_attach
            if (oldDetachPage->canExit())
            {
                oldDetachPage->onExit();
            }
            Sys::splitter->removeFloater();
            reAttachPage(oldDetachPage);
        }

        Sys::splitter->addFloater(page);
        page->setNewlySelected(true);
        pageList->setState(name,PAGE_SUB_ATTACHED);
    }
    else
    {
        // this fully detaches
        pageList->setState(name,PAGE_DETACHED);

        // change status
        page->setNewlySelected(true);

        // float
        page->detach(name);
    }

    updateLocked = false;
}

void PanelPageController::reAttachPage(panel_page * page)
{
    QString name = page->getName();
    qInfo() << "PanelPageController::reAttachPage" << name;

    updateLocked = true;

    pageList->setState(name,PAGE_ATTACHED);

    if (page == getCurrentPage())
    {
        setCurrentPage(name);
    }

    updateLocked = false;
}

void PanelPageController::slot_selectPanelPage(QListWidgetItem * item)
{
    if (item->text().isEmpty())
    {
        return;     // must be a separator
    }

    //qInfo() << "PanelPageController::slot_selectPanelPage:" << item->text();

    // exit previous page
    panel_page * currentPage = pageList->getSelectedPage();
    if (currentPage)
    {
        QString currentName = currentPage->getName();
        qDebug() << "current page" << currentName;
        if (currentPage->canExit())
        {
            currentPage->onExit();
        }
        else
        {
            // if cannot exit, reselect the current
            pageList->blockSignals(true);
            pageList->setCurrentRow(currentName);
            pageList->blockSignals(false);
            return;
        }
    }

    // select new page
    QString newName         = item->text();
    panel_page * newPage    = pageList->getPage(newName);
    qDebug().noquote() << "newpage" << newName;

    switch(pageList->getState(newPage))
    {
    case PAGE_DETACHED:
        reAttachPage(newPage);
        break;

    case PAGE_SUB_ATTACHED:
        Q_ASSERT(Sys::splitter);
        Sys::splitter->removeFloater();
        reAttachPage(newPage);
        break;

    case PAGE_ATTACHED:
        break;
    }

    setCurrentPage(newName);

    panel->completePageSelection();
}

void  PanelPageController::slot_itemDetachPanelPage(QListWidgetItem * item)
{
    QString name = item->text();
    if (name.isEmpty())
    {
        // must be a separator
        return;
    }

    qInfo() << "PanelPageController::slot_itemDetachPanelPage" << name;
    slot_detachWidget(name);
}

bool PanelPageController::isVisiblePage(panel_page * page)
{
    if ((pageList->getSelectedPage() == page) || pageList->isVisiblyDetached(page))
    {
        return true;
    }
    return false;
}

bool PanelPageController::isVisiblePage(ePanelPage page)
{
    if (pageList->getSelectedPage()->getPageType() == page)
    {
        return true;
    }
    return (pageList->isVisiblyDetached(page));
}

