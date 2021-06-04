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

#ifndef PAGE_LOG_H
#define PAGE_LOG_H

#include "panels/panel_page.h"

class page_log;

class AQScrollBar : public QScrollBar
{
    Q_OBJECT

public:
    AQScrollBar(page_log *plog);
};

class page_log : public panel_page
{
    Q_OBJECT

public:
    page_log(ControlPanel * cpanel);

    void onEnter() override;
    void onExit() override {}
    void refreshPage() override;

public slots:
    void slot_actionTriggered();

private slots:
    void    slot_copyLog();
    void    slot_viewLog();
    void    slot_logToStdErr(bool enable);
    void    slot_logToDisk(bool enable);
    void    slot_logToPanel(bool enable);
    void    slot_numberLines(bool enable);
    void    slot_warningsOnly(bool enable);
    void    slot_elapsedTime(bool enable);
    void    slot_intervalTime(bool enable);

protected:

private:
    QCheckBox   * follow;
    QTextEdit   * ed;
    AQScrollBar * sbar;
    QCheckBox   * cbLogElapsedTime;
    QCheckBox   * cbLogIntervalTime;
};

#endif
