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

#ifndef PAGE_DEBUG_H
#define PAGE_DEBUG_H

#include "panels/panel_page.h"
#include "base/configuration.h"

class page_debug : public panel_page
{
    Q_OBJECT

public:
    page_debug(ControlPanel * cpanel);

    void    refreshPage() override;
    void    onEnter() override;
    void    onExit() override;

signals:
    void    sig_compareImageFiles(QString,QString,bool);
    void    sig_view_image(QString file);
    void    sig_cyclerStart(eCycleMode);

public slots:
    void    slot_compareResult(QString result);
    void    slot_setImage0(QString name);
    void    slot_setImage1(QString name);

private slots:
    void    slot_verifyTilingNames();
    void    slot_reformatDesignXML();
    void    slot_reformatTilingXML();
    void    slot_reprocessDesignXML();
    void    slot_reprocessTilingXML();

    void    slot_stopIfDiffClicked(bool enb);
    void    slot_cycleModeChanged(int id);
    void    slot_cycleIntervalChanged(int value);
    void    selectDir0();
    void    selectDir1();
    void    swapDirs();
    void    slot_previous();
    void    slot_next();

    void    slot_ibox0_changed(int index);
    void    slot_ibox1_changed(int index);
    void    slot_viewImage0();
    void    slot_viewImage1();
    void    slot_cycle();
    void    slot_opendir();
    void    slot_compareImages();
    void    slot_compareCycle();
    void    slot_transparentClicked(bool checked);
    void    slot_differencesClicked(bool checked);
    void    slot_ping_pongClicked(bool checked);
    void    slot_side_by_sideClicked(bool checked);
    void    slot_use_badlist(bool checked);
    void    slot_use_badlist2(bool checked);

    void    slot_dir0Changed();
    void    slot_dir1Changed();

    void    slot_verifyMapsClicked(bool enb);
    void    slot_verifyDumpClicked(bool enb);
    void    slot_verifyVerboseClicked(bool enb);

protected:
    QGroupBox   * createDebugSection();
    QGroupBox   * createCycleSection();
    QGroupBox   * createImagesSection();
    QGroupBox   * createVerifyMaps();

    void viewImage(QString file);

    void loadCombo(QComboBox * box, QString dir);
    void setCombo(QComboBox * box,QString name);

    void saveMosaicBitmaps();
    void saveTilingBitmaps();
    void savePixmap(QString name);

    QString getPixmapPath();

private:
    QLineEdit   * dir0;
    QLineEdit   * dir1;
    QLineEdit   * directory;

    QComboBox   * ibox0;
    QComboBox   * ibox1;
    QLineEdit   * imageCompareResult;
};

#endif
