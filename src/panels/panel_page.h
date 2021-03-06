﻿/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
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

#ifndef PANELPAGE_H
#define PANELPAGE_H

#include <QtWidgets>
#include <QString>

typedef std::shared_ptr<class TilingMaker> TilingMakerPtr;

class panel_page : public QWidget
{
    Q_OBJECT

public:
    panel_page(class ControlPanel * panel, QString name);

    virtual void	refreshPage() = 0;
    virtual void    onEnter()     = 0;
    virtual void    onExit()      = 0;
    virtual bool    canExit() { return true; }

    void            leaveEvent(QEvent *event) override;
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    void            enterEvent(QEnterEvent *event) override;
#else
    void            enterEvent(QEvent *event) override;
#endif
    virtual void	closeEvent(QCloseEvent * event) override;

    QString getName()                    { return pageName; }
    void    setNewlySelected(bool state) { newlySelected = state; }
    bool    isNewlySelected()            { return newlySelected; }
    bool    isFloated()                  { return floated;}
    void    setFloated(bool isFloated)   { floated = isFloated; }
    void    closePage();
    void    floatMe();
    bool    wasFloated();

    QString addr(const void * address);
    QString addr(void * address);

    void    updateView();

signals:
    void    sig_render();
    void	sig_attachMe(QString title);
    void    sig_refreshView();

protected:
    virtual void    mouseEnter() { refresh = false; }
    virtual void    mouseLeave() { refresh = true; }

    void    blockPage(bool block);
    bool    pageBlocked();

    QVBoxLayout *		vbox;
    QString				pageName;

    class ControlPanel      * panel;
    class Configuration     * config;
    class View              * view;
    class ViewControl       * vcontrol;
    TilingMakerPtr            tilingMaker;
    class MotifMaker        * motifMaker;
    class DecorationMaker   * decorationMaker;

    bool                refresh;

private:
    bool                newlySelected;
    bool                floated;
    int                 blockCount;
};

#endif
