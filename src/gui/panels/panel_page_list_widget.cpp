#include <QDebug>
#include <QMouseEvent>
#include <QMenu>
#include <QLineEdit>

#include "gui/panels/panel_page_list_widget.h"
#include "gui/top/controlpanel.h"

//////////////////////////////////////////////////////////
///
/// PageListWidget
///
//////////////////////////////////////////////////////////

PageListWidget::PageListWidget(ControlPanel * parent) : QListWidget((QWidget*)parent)
{
    controlPanel = parent;

    separators = 0;
    setSelectionMode(QAbstractItemView::SingleSelection);
    //setSizeAdjustPolicy(QListWidget::AdjustToContents);
    //setResizeMode(QListWidget::Adjust);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);



    // determine default colors
    QListWidgetItem * item = new QListWidgetItem("");
    insertItem(0,item);
    backgroundBrush = item->background();
    foregroundBrush = item->foreground();
    item = takeItem (0);
    delete item;

    // set colors for selected
    setStyleSheet("QListWidget::item:selected { background:yellow; color:red; }");
}

void PageListWidget::deletePages()
{
    //	delete pages on shutdown
    for (const PageData & pdata : std::as_const(pageData))
    {
        panel_page * page = pdata.page;
        //qDebug() << page->getName() << page->getPageType();
        delete page;
    }
}

void PageListWidget::refreshPages()
{
    auto selectedPage = getSelectedPage();

    //	Update pages
    for (const PageData & pdata : std::as_const(pageData))
    {
        if (pdata.page == selectedPage || pdata.page->getState() == PAGE_DETACHED || pdata.page->getState() == PAGE_SUB_ATTACHED)
        {
            refreshPage(pdata.page);
        }
    }
}

void PageListWidget::refreshPage(panel_page * wp)
{
    Q_ASSERT(wp != nullptr);

    if (wp->pageBlocked())
        return;

    if (wp->isNewlySelected())
    {
        wp->onEnter();
        wp->setNewlySelected(false);
        wp->repaint();
    }

    wp->onRefresh();
    wp->show();
}

bool PageListWidget::isVisiblyDetached(panel_page * page)
{
    auto selectedPage = getSelectedPage();
    if (selectedPage == page)
        return true;

    ePageState state = getPageData(page).page->getState();
    if (state == PAGE_DETACHED || state == PAGE_SUB_ATTACHED)
    {
        return true;
    }

    return false;
}

bool PageListWidget::isVisiblyDetached(ePanelPage page)
{
    for (const auto & data : std::as_const(pageData))
    {
        if (data.page->getPageType() == page)
        {
            ePageState state = data.page->getState();
            if (state == PAGE_DETACHED || state == PAGE_SUB_ATTACHED)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

void PageListWidget::closePages()
{
    for (const PageData & pdata : std::as_const(pageData))
    {
        pdata.page->closePage();
    }
}

panel_page * PageListWidget::getPage(QString name)
{
    PageData & data = getPageData(name);
    return data.page;
}

panel_page * PageListWidget::getSelectedPage()
{
    for (auto & pdata : std::as_const(pageData))
    {
        if (pdata.selected)
        {
            return pdata.page;
        }
    }
    return nullptr;
}

void PageListWidget::addPage(panel_page * page)
{
    QListWidgetItem * item = new QListWidgetItem(page->getName());
    addItem(item);
    PageData pd(page,item);
    pageData.push_back(pd);
}

void PageListWidget::setState(QString name, ePageState state)
{
    PageData & data = getPageData(name);
    qDebug().noquote() << name << sPageState[state];

    data.page->setState(state);

    auto item = data.pageItem;
    switch (state)
    {
    case PAGE_DETACHED:
    case PAGE_SUB_ATTACHED:
        item->setBackground(QBrush(Qt::gray));
        item->setForeground(QBrush(Qt::yellow));
        break;

    case PAGE_ATTACHED:
        data.page->setParent(controlPanel);          // re-parent the widget

        item->setBackground(backgroundBrush);
        item->setForeground(foregroundBrush);
        break;
    }
}

void PageListWidget::mousePressEvent(QMouseEvent * event)
{
    //qDebug() << "PanelListWidget::mousePressEvent";
    if (event->button() == Qt::RightButton)
    {
        QPoint pt    = event->position().toPoint();
        auto item    = itemAt(pt);
        pageToDetach = item->text();
        if (pageToDetach.isEmpty())
        {
            // must be a separator
            return;
        }

        QMenu menu(this);
        menu.addSection(pageToDetach);
        menu.addAction("Float",this,&PageListWidget::slot_floatAction);
        menu.exec(event->globalPosition().toPoint());
    }
    else
    {
        QListWidget::mousePressEvent(event);
    }
}

QStringList PageListWidget::wereFloated()
{
    QStringList names;
    for (const PageData & pdata : std::as_const(pageData))
    {
        if (pdata.page->wasFloated())
        {
            qDebug() << "floating:" << pdata.pageName;
            names.push_back(pdata.pageName);
        }
    }
    return names;
}

QString PageListWidget::wasSubAttached()
{
    for (const PageData & pdata : std::as_const(pageData))
    {
        if (pdata.page->wasSubAttached())
        {
            qDebug() << "sub-attached" << pdata.pageName;
            return pdata.pageName;
        }
    }
    return QString();
}

void PageListWidget::slot_floatAction()
{
    qDebug() << "trigger detach" << pageToDetach;
    emit sig_detachWidget(pageToDetach);
}

void PageListWidget::setCurrentRow(QString name)
{
    PageData & pdata = getPageData(name);
    setCurrentItem(pdata.pageItem);
}

void PageListWidget::addSeparator()
{
    QListWidgetItem * item = new QListWidgetItem();
    item->setSizeHint(QSize(20,10));
    item->setFlags(Qt::NoItemFlags);
    addItem(item);

    QFrame * frame = new QFrame();
    frame->setFrameShape(QFrame::HLine);

    setItemWidget(item,frame);

    separators++;
}

void PageListWidget::establishSize()
{
    int rowCount = count() - separators;
    //setFixedWidth(sizeHintForColumn(0) + (2 * frameWidth()));
    setFixedHeight((sizeHintForRow(0) * rowCount) + (10 * separators) + (2 * frameWidth()) + 5);   // 5 is a little pad
}

PageData & PageListWidget::getPageData(QString name)
{
    for (auto & pdata : pageData)
    {
        if (pdata.pageName == name)
        {
            return pdata;
        }
    }

    qWarning() << "page" << name << "not found - defaulting";
    return pageData[0];
}

PageData & PageListWidget::getPageData(panel_page * page)
{
    for (auto & pdata : pageData)
    {
        if (pdata.page == page)
        {
            return pdata;
        }
    }

    qWarning() << "pagenot found - defaulting";
    return pageData[0];
}

ePageState PageListWidget::getState(panel_page * page)
{
    PageData & data = getPageData(page);
    return data.page->getState();
}

int  PageListWidget::getIndex(panel_page * page)
{
    int index = 0;
    for (const PageData & pdata : std::as_const(pageData))
    {
        if (pdata.page == page)
        {
            return index;
        }
        index++;
    }
    return -1;
}

PageData::PageData(panel_page * ppage, QListWidgetItem * item)
{
    page        = ppage;
    page->setState(PAGE_ATTACHED);
    pageItem    = item;
    pageName    = page->getName();
    selected    = false;
}

PageData::PageData(const PageData & other)
{
    page        = other.page;
    pageItem    = other.pageItem;
    pageName    = other.pageName;
    selected    = other.selected;
}

PageData & PageData::operator=(const PageData & other)
{
    page        = other.page;
    pageItem    = other.pageItem;
    pageName    = other.pageName;
    selected    = other.selected;
    return *this;
}
