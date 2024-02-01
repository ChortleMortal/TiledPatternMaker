#pragma once
#ifndef PAGE_IMAGETOOLS_H
#define PAGE_IMAGETOOLS_H

#include <QImage>
#include "panels/panel_page.h"
#include "engine/image_engine.h"

class MemoryCombo;

enum eActionType
{
    ACT_GEN_MOSAIC_BMP,
    ACT_GEN_TILING_BMP,
    ACT_GEN_COMPARE_WLIST
};

struct sAction
{
    eActionType type;
    QString     name;
    QString     path;
    QString     path2;
};

class page_image_tools : public panel_page
{
    Q_OBJECT

public:
    page_image_tools(ControlPanel * cpanel);
    ~page_image_tools();

    void    onRefresh() override;
    void    onEnter() override;
    void    onExit() override;

    static void addToComparisonWorklist(QString name);

signals:
    void    sig_worklistChanged();

public slots:
    void    slot_compareResult(QString result);
    void    slot_setImageLeftCombo(QString name);
    void    slot_setImageRightCombo(QString name);
    void    slot_deleteCurrentWLEntry(bool confirm);

private slots:
    void    slot_stopIfDiffClicked(bool enb);
    void    slot_cycleIntervalChanged(int value);
    void    slot_selectDir0();
    void    slot_selectDir1();
    void    slor_swapDirs();
    void    slot_previous();
    void    slot_next();
    void    slot_loadFirst();
    void    slot_colorEdit();
    void    slot_useFilter(bool checked);

    void    slot_ibox0_changed(int index);
    void    slot_ibox1_changed(int index);

    void    slot_viewImageLeft();
    void    slot_viewImageRight();
    void    slot_genBMPs();
    void    slot_startStepping();
    void    slot_opendir();
    void    slot_compareDiffDirBMPs();
    void    slot_startCompare();
    void    slot_compareGen();
    void    slot_transparentClicked(bool checked);
    void    slot_popupClicked(bool checked);
    void    slot_differencesClicked(bool checked);
    void    slot_use_worklist_compare(bool checked);
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
    void    slot_compareFileBMPs();
    
    void    slot_loadWorkListFromFile();
    void    slot_saveWorkListToFile();
    void    slot_editWorkList();
    void    slot_replaceBMP();
    void    slot_loadSecond();
    void    slot_createList();

    void    slot_genTypeChanged(int id);
    void    slot_gen_selectionChanged();
    void    slot_view_selectionChanged();
    void    slot_ver_selectionChanged();

    void    slot_compareDiffVerBMPs();
    void    slot_cycleVersions();

    void    slot_nextImage();
    void    slot_viewTypeChanged(int);

    void    slot_engineComplete();
    void    slot_engineProgress(int val);

protected:
    int createWorklistBox(int row);
    int createCycleGenBox(int row);
    int createCompareImagesBox(int row);
    int createCompareVersionsBox(int row);
    int createViewImageBox(int row);

    void addHSeparator(int row);
    void bump(int row, int stretch);

    bool viewImage(QString filename, bool transparent, bool popup);

    void loadCombo(QComboBox * box, QString dir);
    void setCombo(QComboBox * box,QString name);

    void loadFileFilterCombo();
    void loadViewFilterCombo();
    void loadVersionFilterCombo();
    void setImageDirectory();

    QString getPixmapPath();

    // multi-threaded
    void setupActions();
    void processActionList(QList<sAction> & actions);
    void saveTilingBitmaps();
    void saveMosaicBitmaps();
    void createComparedWorklist();

    // comparison worklis
    static void         clearComparisonWorklist();
    static QStringList& getComparisonWorklist();

private:
    static QStringList comparisonWorklist;
    static QMutex      comparisonMutex;

    bool          created;

    QGridLayout * grid;

    QButtonGroup* genBtnGroup;
    QButtonGroup* viewBtnGroup;

    MemoryCombo * firstDir;
    MemoryCombo * secondDir;
    QLineEdit   * directory;
    QLineEdit   * imageCompareResult;

    MemoryCombo * viewFileCombo1;
    MemoryCombo * viewFileCombo2;

    QComboBox   * firstFileCombo;
    QComboBox   * secondFileCombo;

    QComboBox   * genFilterCombo;
    QComboBox   * viewFilterCombo;
    QComboBox   * versionFilterCombo;

    QCheckBox   * use_wlistForCompareChk;
    QCheckBox   * compareView;

    QLabel      * colorLabel;
    QLabel      * wlistLabel;

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

    QPushButton * generateBMPsBtn;
    QPushButton * viewImgesBtn;
    QPushButton * startBtn;
    QPushButton * versionStepBtn;

    QFutureWatcher<bool>  watcher;
    int                   totalEngineImages;

    eActionType        generatorType;

    class AQElapsedTimer* etimer;

    ImageEngine     engine;

};

#endif
