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

#ifndef PANELPAGE_H
#define PANELPAGE_H

#include <QtGui>
#include <QtWidgets>
#include <QString>

#include "base/configuration.h"
#include "base/view.h"
#include "base/workspace.h"
#include "panels/panel_misc.h"
#include <vector>

using std::vector;

class Canvas;
class View;
class Configuration;
class Workspace;
class WorkspaceViewer;
class TiledPatternMaker;
class ControlPanel;

class panel_page : public QWidget
{
    Q_OBJECT

public:
    panel_page(ControlPanel * panel, QString name);

    virtual void	refreshPage() = 0;
    virtual void    onEnter()     = 0;
    virtual void    onExit()      = 0;

    void            leaveEvent(QEvent *event) override;
    void            enterEvent(QEvent *event) override;
    virtual void	closeEvent(QCloseEvent * event) override;

    void    setNewlySelected(bool state) { newlySelected = state; }
    bool    isNewlySelected() { return newlySelected; }
    void    setFloated(bool isFloated) { floated = isFloated; }
    void    closePage();
    void    floatMe();
    bool    wasFloated();

    QString getName() { return pageName; }
    QString addr(const void * address);
    QString addr(void * address);

    static void    adjustTableSize(QTableWidget *table);
    static void    adjustTableWidth(QTableWidget *table);
    static void    adjustTableHeight(QTableWidget *table);

    static int     getTableWidth(QTableWidget *table);
    static int     getTableHeight(QTableWidget * table);

signals:
    void    sig_render(eRender type);
    void	sig_attachMe(QString title);
    void    sig_viewWS();

public slots:

protected:
    virtual void    mouseEnter() { refresh = false; }
    virtual void    mouseLeave() { refresh = true; }

    QVBoxLayout *		vbox;
    QString				pageName;

    ControlPanel      * panel;
    Canvas            * canvas;
    Configuration     * config;
    Workspace         * workspace;
    TiledPatternMaker * maker;
    WorkspaceViewer   * viewer;

    bool    refresh;

private:
    bool                newlySelected;
    bool                floated;
};

#endif
