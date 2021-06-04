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
#include "base/tiledpatternmaker.h"
#include "base/utilities.h"
#include "viewers/view.h"
#include "viewers/viewcontrol.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/decoration_maker/decoration_maker.h"

panel_page::panel_page(ControlPanel * panel,  QString name) : QWidget()
{
    pageName    = name;

    this->panel     = panel;
    config          = Configuration::getInstance();
    vcontrol        = ViewControl::getInstance();
    view            = View::getInstance();
    motifMaker      = MotifMaker::getInstance();
    tilingMaker     = TilingMaker::getSharedInstance();
    decorationMaker = DecorationMaker::getInstance();

    vbox = new QVBoxLayout;
    vbox->setSizeConstraint(QLayout::SetFixedSize);

    newlySelected   = false;
    floated         = false;
    refresh         = true;
    blockCount      = 0;

    setLayout (vbox);

    connect(this,   &panel_page::sig_render,     theApp,   &TiledPatternMaker::slot_render);
    connect(this,   &panel_page::sig_refreshView,vcontrol, &ViewControl::slot_refreshView);
    connect(this,   &panel_page::sig_attachMe,   panel,    &ControlPanel::slot_attachWidget, Qt::QueuedConnection);
}

// pressing 'x' to close detached/floating page is used to re-attach
void panel_page::closeEvent(QCloseEvent *event)
{
    setParent(nullptr);

    QSettings s;
    QString name = QString("panel2/%1/pagePos").arg(pageName);
    QPoint pt = pos();
    qDebug() << "panel_page close event pos = " << pt;
    s.setValue(name,pt);

    event->setAccepted(false);

    floated = false;

    // re-attach to stack
    emit sig_attachMe(pageName);
}

// called when closing the application
void panel_page::closePage()
{
    QSettings s;
    QString name = QString("panel2/%1/floated").arg(pageName);
    if (floated)
    {
        s.setValue(name,true);

        name = QString("panel2/%1/pagePos").arg(pageName);
        QPoint pt = pos();
        qDebug() << "panel_page close event pos = " << pt;
        s.setValue(name,pt);

    }
    else
    {
        s.setValue(name,false);
    }
}

bool panel_page::wasFloated()
{
    QSettings s;
    QString name = QString("panel2/%1/floated").arg(pageName);
    bool wasFloated = s.value(name,false).toBool();
    return wasFloated;
}

void panel_page::floatMe()
{
    QSettings s;
    QString name = QString("panel2/%1/pagePos").arg(pageName);
    QPoint pt    = s.value(name).toPoint();
    qDebug() << "panel_page::floatMe()" << pageName << pt;
    move(pt);
}

QString panel_page::addr(void * address)
{
    return Utils::addr(address);
}

QString panel_page::addr(const void * address)
{
    return QString::number(reinterpret_cast<uint64_t>(address),16);
}

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
void panel_page::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event)
    mouseEnter();
}
#else
void panel_page::enterEvent(QEvent *event)
{
    Q_UNUSED(event)
    mouseEnter();
}
#endif

void panel_page::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    mouseLeave();
}

void  panel_page::blockPage(bool block)
{
    if (block)
    {
        blockCount++;
    }
    else
    {
        blockCount--;
        Q_ASSERT(blockCount >= 0);
    }
}

bool  panel_page::pageBlocked()
{
    return (blockCount != 0);
}

void panel_page::updateView()
{
    view->update();
}
