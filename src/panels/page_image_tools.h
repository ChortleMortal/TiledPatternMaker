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

#ifndef PAGE_IMAGETOOLS_H
#define PAGE_IMAGETOOLS_H

#include "panels/panel_page.h"
#include "settings/configuration.h"

class page_image_tools : public panel_page
{
    Q_OBJECT

public:
    page_image_tools(ControlPanel * cpanel);

    void    refreshPage() override;
    void    onEnter() override;
    void    onExit() override;

signals:
    void    sig_compareImageFiles(QString,QString,bool);
    void    sig_view_image(QString file,QString file2,bool transparent,bool popup);
    void    sig_cyclerStart(eCycleMode);
    void    sig_loadMosaic(QString);

public slots:
    void    slot_compareResult(QString result);
    void    slot_setImage0(QString name);
    void    slot_setImage1(QString name);

private slots:
    void    slot_stopIfDiffClicked(bool enb);
    void    slot_cycleModeChanged(int id);
    void    slot_cycleIntervalChanged(int value);
    void    selectDir0();
    void    selectDir1();
    void    swapDirs();
    void    slot_previous();
    void    slot_next();
    void    slot_load();
    void    slot_colorEdit();
    void    slot_useFilter(bool checked);

    void    slot_ibox0_changed(int index);
    void    slot_ibox1_changed(int index);
    void    slot_viewImage();
    void    slot_viewImageLeft();
    void    slot_viewImageRight();
    void    slot_cycle();
    void    slot_opendir();
    void    slot_compareImages();
    void    slot_compareCycle();
    void    slot_transparentClicked(bool checked);
    void    slot_view_transparentClicked(bool checked);
    void    slot_view_popupClicked(bool checked);
    void    slot_popupClicked(bool checked);
    void    slot_differencesClicked(bool checked);
    void    slot_use_worklist_compare(bool checked);
    void    slot_use_worklist_generate(bool checked);
    void    slot_gen_worklist(bool checked);
    void    slot_skipExisting(bool checked);

    void    slot_dir0Changed();
    void    slot_dir1Changed();

    void    slot_selectImage();

    void    loadWorkListFromFile();
    void    saveWorkListToFile();
    void    editWorkList();

protected:
    QGroupBox   * createCycleSection();
    QGroupBox   * createWorklistSection();
    QGroupBox   * createCompareSection();
    QGroupBox   * createViewSection();
    QGroupBox   * createTransparencySection();

    void viewImage(QString file, bool transparent, bool popup);

    void loadCombo(QComboBox * box, QString dir);
    void setCombo(QComboBox * box,QString name);

    void saveMosaicBitmaps();
    void saveTilingBitmaps();
    void savePixmap(QString name);

    QString getPixmapPath();


private:
    QLineEdit   * leftDir;
    QLineEdit   * rightDir;
    QLineEdit   * directory;
    QLineEdit   * viewFile;

    QComboBox   * leftFile;
    QComboBox   * rightFile;
    QLineEdit   * imageCompareResult;

    QCheckBox   * use_wlistForCompareChk;
    QCheckBox   * use_wlistForGenerateChk;
    QCheckBox   * gen_wlistChk;
    QLabel      * wlistStatus;
    QLabel      * colorLabel;
};

#endif
