#ifndef PAGE_IMAGETOOLS_H
#define PAGE_IMAGETOOLS_H

class QGroupBox;
class QComboBox;
class QCheckBox;
class QLabel;
class QLineEdit;
class MemoryCombo;
class QButtonGroup;

#include "widgets/panel_page.h"
#include "enums/ecyclemode.h"

class page_image_tools : public panel_page
{
    Q_OBJECT

public:
    page_image_tools(ControlPanel * cpanel);

    void    onRefresh() override;
    void    onEnter() override;
    void    onExit() override;

signals:
    void    sig_compareImageFiles(QString,QString,bool);
    void    sig_view_image(QString file,QString file2,bool transparent,bool popup);
    void    sig_cyclerStart(eCycleMode);
    void    sig_loadMosaic(QString,bool ready);

public slots:
    void    slot_compareResult(QString result);
    void    slot_setImage0(QString name);
    void    slot_setImage1(QString name);
    void    slot_deleteCurrent();

private slots:
    void    slot_stopIfDiffClicked(bool enb);
    void    slot_cycleIntervalChanged(int value);
    void    selectDir0();
    void    selectDir1();
    void    swapDirs();
    void    continueCycle();
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
    void    slot_skipExisting(bool checked);

    void    slot_dir0Changed();
    void    slot_dir1Changed();

    void    slot_selectImage();
    void    slot_imageSelectionChanged();

    void    loadWorkListFromFile();
    void    saveWorkListToFile();
    void    editWorkList();
    void    replaceCurrent();
    void    loadCurrent();

protected:
    QGroupBox   * createCycleGenSection();
    QGroupBox   * createCycleViewSection();
    QGroupBox   * createWorklistSection();
    QGroupBox   * createCompareSection();
    QGroupBox   * createViewSection();
    QGroupBox   * createTransparencySection();

    void viewImage(QString file, bool transparent, bool popup);

    void loadCombo(QComboBox * box, QString dir);
    void setCombo(QComboBox * box,QString name);

    bool loadMosaic(QString name);
    void saveMosaicBitmaps();
    void saveTilingBitmaps();
    void savePixmap(QString name);

    QString getPixmapPath();

private:
    bool         localCycle;

    QButtonGroup * cycleGenBtnGroup;
    QButtonGroup * cycleViewBtnGroup;

    QLineEdit   * leftDir;
    QLineEdit   * rightDir;
    QLineEdit   * directory;

    MemoryCombo * viewFileCombo;

    QComboBox   * leftFile;
    QComboBox   * rightFile;
    QLineEdit   * imageCompareResult;

    QComboBox   * fileFilterCombo;
    QCheckBox   * use_wlistForCompareChk;
    QCheckBox   * gen_wlistChk;
    QLabel      * wlistStatus;
    QLabel      * colorLabel;
};

#endif
