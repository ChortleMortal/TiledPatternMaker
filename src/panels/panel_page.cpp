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

#include "panels/panel_page.h"
#include "panels/panel.h"
#include "viewers/workspaceviewer.h"
#include "base/tiledpatternmaker.h"
#include "base/canvas.h"
#include "base/utilities.h"

panel_page::panel_page(ControlPanel * panel,  QString name) : QWidget()
{
    this->panel = panel;
    maker     = panel->getMaker();
    pageName  = name;

    config    = Configuration::getInstance();
    canvas    = Canvas::getInstance();
    workspace = Workspace::getInstance();
    viewer    = WorkspaceViewer::getInstance();

    vbox      = new QVBoxLayout;
    vbox->setSizeConstraint(QLayout::SetFixedSize);

    newlySelected   = false;
    floated         = false;

    //setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    setLayout (vbox);
    //setAcceptDrops (true);

    setObjectName("panel_page");

    connect(this,   &panel_page::sig_render,                maker,  &TiledPatternMaker::slot_render);
    connect(this,   &panel_page::sig_viewWS,                viewer, &WorkspaceViewer::slot_viewWorkspace);
    connect(this,   &panel_page::sig_updateDesignInfo,      viewer, &WorkspaceViewer::slot_updateDesignInfo);
    connect(this,   &panel_page::sig_attachMe,              panel,  &ControlPanel::slot_attachWidget,       Qt::QueuedConnection);
}

// pressing 'x' to close detached/floating page is used to re-attach
void panel_page::closeEvent(QCloseEvent *event)
{
    setParent(nullptr);

    QSettings s;
    QString name = QString("pane2/%1/pagePos").arg(pageName);
    s.setValue(name,pos());

    event->setAccepted(false);

    floated = false;

    // re-attach to stack
    emit sig_attachMe(pageName);
}

// called when closing the application
void panel_page::closePage()
{
    QSettings s;
    QString name = QString("pane2/%1/floated").arg(pageName);
    if (floated)
    {
        s.setValue(name,true);
    }
    else
    {
        s.setValue(name,false);
    }
}

bool panel_page::wasFloated()
{
    QSettings s;
    QString name = QString("pane2/%1/floated").arg(pageName);
    bool wasFloated = s.value(name,false).toBool();
    return wasFloated;
}

void panel_page::floatMe()
{
    QSettings s;
    QString name = QString("pane2/%1/pagePos").arg(pageName);
    move(s.value(name).toPoint());
}

int panel_page::getTableWidth(QTableWidget *t)
{
    int w = t->verticalHeader()->width() + (t->frameWidth() * 2); // +4 seems to be needed
    for (int i = 0; i < t->columnCount(); i++)
    {
        w += t->columnWidth(i);
    }
    return w;
}

int panel_page::getTableHeight(QTableWidget *t)
{
    int h = t->horizontalHeader()->height() + (t->frameWidth() * 2);
    for (int i = 0; i < t->rowCount(); i++)
    {
        h += t->rowHeight(i);
    }
    return h;
}

void panel_page::adjustTableSize(QTableWidget *table)
{
    int w = getTableWidth(table);
    int h = getTableHeight(table);
    QSize size(w,h);
    table->setMaximumSize(size);
    table->setMinimumSize(size);
    updateGeometry();
}

void panel_page::adjustTableWidth(QTableWidget *table)
{
    int w = getTableWidth(table);
    table->setMaximumWidth(w);
    table->setMinimumWidth(w);
    updateGeometry();
}

void panel_page::adjustTableHeight(QTableWidget *table)
{
    int h = getTableHeight(table);
    table->setMaximumHeight(h);
    table->setMinimumHeight(h);
    updateGeometry();
}

QString panel_page::addr(void * address)
{
    return Utils::addr(address);
}

QString panel_page::addr(const void * address)
{
    return QString::number(reinterpret_cast<uint64_t>(address),16);
}
