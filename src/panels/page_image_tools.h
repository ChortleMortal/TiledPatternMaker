#pragma once
#ifndef PAGE_IMAGETOOLS_H
#define PAGE_IMAGETOOLS_H

#include <QImage>
#include "widgets/panel_page.h"
#include "enums/ecyclemode.h"

class MemoryCombo;

class page_image_tools : public panel_page
{
    Q_OBJECT

public:
    page_image_tools(ControlPanel * cpanel);

    void    onRefresh() override;
    void    onEnter() override;
    void    onExit() override;

signals:
    void    sig_compareBMPFiles(QString,QString,bool);
    void    sig_compareBMPFilesPath(QString,QString);
    void    sig_compareBMPandLoaded(QString,bool);
    void    sig_view_image(QString file,QString file2,bool transparent,bool popup);
    void    sig_cyclerStart(eCycleMode);
    void    sig_loadMosaic(QString,bool ready);
    void    sig_worklistChanged();

public slots:
    void    slot_compareResult(QString result);
    void    slot_setImageLeftCombo(QString name);
    void    slot_setImageRightCombo(QString name);
    void    slot_deleteCurrentWLEntry(bool confirm);

private slots:
    void    slot_stopIfDiffClicked(bool enb);
    void    slot_cycleIntervalChanged(int value);
    void    selectDir0();
    void    selectDir1();
    void    swapDirs();
    void    continueCycle();
    void    slot_previous();
    void    slot_next();
    void    slot_loadFirst();
    void    slot_colorEdit();
    void    slot_useFilter(bool checked);

    void    slot_ibox0_changed(int index);
    void    slot_ibox1_changed(int index);

    void    slot_viewImageLeft();
    void    slot_viewImageRight();
    void    slot_cycleGen();
    void    slot_cycleView();
    void    slot_opendir();
    void    slot_compareImages();
    void    slot_compareCycle();
    void    slot_transparentClicked(bool checked);
    void    slot_view_transparentClicked(bool checked);
    void    slot_view_popupClicked(bool checked);
    void    slot_popupClicked(bool checked);
    void    slot_differencesClicked(bool checked);
    void    slot_use_worklist_compare(bool checked);
    void    slot_gen_worklist_compare(bool checked);
    void    slot_compareView(bool checked);
    void    slot_skipExisting(bool checked);
    
    void    slot_firstDirChanged();
    void    slot_secondDirChanged();

    void    slot_selectImage1();
    void    slot_selectImage2 ();
    void    slot_viewImage1();
    void    slot_viewImage2();
    void    slot_viewImage3();
    void    slot_viewImage4();
    void    slot_imageSelectionChanged1();
    void    slot_imageSelectionChanged2();
    void    slot_compareImages2();

    void    loadWorkListFromFile();
    void    saveWorkListToFile();
    void    editWorkList();
    void    replaceBMP();
    void    slot_loadSecond();
    void    slot_createList();

    void    slot_genTypeChanged(int id);
    void    slot_gen_selectionChanged();
    void    slot_ver_selectionChanged();

    void    slot_compareVersions();
    void    slot_cycleVersions();
    void    slot_mediaAChanged();
    void    slot_mediaBChanged();
    void    slot_nextImage();
    void    slot_quitImageCycle();

protected:
    QGroupBox   * createCycleGenBox();
    QHBoxLayout * createWorklistBox();
    QGroupBox   * createCompareImagesBox();
    QGroupBox   * createCompareVersionsBox();
    QGroupBox   * createViewImageBox();
    QGroupBox   * createTransparencyBox();

    bool viewImage(QString file, bool transparent, bool popup);

    void loadVersionCombos();
    void loadCombo(QComboBox * box, QString dir);
    void setCombo(QComboBox * box,QString name);

    void loadFileFilterCombo();
    void loadVersionFilterCombo();
    bool loadMosaic(QString name);
    void saveMosaicBitmaps();
    void saveTilingBitmaps();
    void savePixmap(QString name);
    void setImageDirectory();
    void compareNextVersions();

    QString getPixmapPath();

private:
    bool          created;

    QButtonGroup* cycleGenBtnGroup;
    QButtonGroup* cycleViewBtnGroup;

    QGroupBox   * wlistGroupBox;
    
    MemoryCombo * firstDir;
    MemoryCombo * secondDir;
    QLineEdit   * directory;
    QLineEdit   * imageCompareResult;

    MemoryCombo * viewFileCombo1;
    MemoryCombo * viewFileCombo2;

    QComboBox   * firstFileCombo;
    QComboBox   * secondFileCombo;
    QComboBox   * fileFilterCombo;
    QComboBox   * versionFilterCombo;

    QCheckBox   * use_wlistForCompareChk;
    QCheckBox   * gen_wlistChk;
    QCheckBox   * compareView;

    QLabel      * colorLabel;

    QPushButton * loadFirstBtn;
    QPushButton * loadSecondBtn;

    QComboBox   * mediaA;
    QComboBox   * mediaB;
    QComboBox   * versionsA;
    QComboBox   * versionsB;

    QCheckBox   * chkLock;
    QRadioButton* radMosaic;
    QRadioButton* radTile;
    QRadioButton* radImg;
    QRadioButton* radXML;
    QButtonGroup* mediaGroup;
    QButtonGroup* typeGroup;

    bool                  comparingVersions;
    QStringList           mediaNames;
    QString               mediaName;
    QStringList           versions;
    QStringList::iterator imgList_it;
    QStringList::iterator imgListVerA_it;
    QStringList::iterator imgListVerB_it;
    QImage                imgA;
    QImage                imgB;
};

#endif
