#pragma once
#ifndef PAGE_LOADERS_H
#define PAGE_LOADERS_H

#include "gui/panels/panel_page.h"
#include "gui/widgets/versioned_list_widget.h"
#include "legacy/design_maker.h"
#include "sys/enums/edesign.h"
#include "sys/enums/estatemachineevent.h"
#include "sys/sys/versioning.h"

class page_loaders : public panel_page
{
    Q_OBJECT

public:
    explicit page_loaders(ControlPanel * apanel);
    ~page_loaders();

    void    onRefresh() override;
    void    onEnter() override;
    void    onExit() override {}

signals:
    void    sig_loadDesign(eDesign id);
    void    sig_buildDesign(eDesign id);
    void    sig_sortMosaics();
    void    sig_sortTilings();
    void    sig_setTilingUses();

public slots:
   void     slot_newTiling();
   void     slot_newMosaic();

   void     slot_mosaicLoaded(VersionedFile file);
   void     slot_tilingLoaded(VersionedFile file);
   void     slot_loadedDesign(eDesign design);

   void     desRightClick(QPoint pos);
   void     xmlRightClick(QPoint pos);
   void     tileRightClick(QPoint pos);

   void     refillTilingsCombo();
   void     refillMosaicsCombo();

private slots:
    void    mosaicSelected(QListWidgetItem *item, QListWidgetItem *oldItem);
    void    mosaicClicked(QListWidgetItem *item);
    void    slot_mosaicActivated(QListWidgetItem * item);
    void    slot_mosaicTextChanged(const QString & currentText);

    void    tilingSelected(QListWidgetItem * item, QListWidgetItem* oldItem);
    void    tilingClicked(QListWidgetItem * item);
    void    slot_tilingActivated(QListWidgetItem * item);
    void    slot_tilingTextChanged(const QString & currentText);

    void    designSelected(QListWidgetItem * item, QListWidgetItem* oldItem);
    void    designClicked(QListWidgetItem * item);

    void    slot_mosaicItemEnteredToolTip(QListWidgetItem * item);

    void    loadShapes();
    void    slot_loadTiling();
    void    slot_loadTilingReplace();
    void    slot_loadTilingMulti();

    void    openXML();
#ifdef Q_OS_WINDOWS
    void    showXMLDir();
#endif

    void    loadMosaic();
    void    rebaseMosaic();
    void    renameMosaic();
    void    deleteMosaic();
    void    reformatMosaic();
    void    addToWorklist();
    void    removeFromWorklist();

    void    showTilings();
    void    genMosaicBMP();


    void    openTiling();
    void    rebaseTiling();
    void    renameTiling();
    void    deleteTiling();
    void    reformatTiling();
    void    addTilingToWorklist();
    void    removeTilingFromWorklist();

    void    slot_whereTilingUsed();
    void    slot_mosaicFilter(const QString & filter);
    void    slot_tilingFilter(const QString & filter);
    void    slot_mosaicCheck(bool check);
    void    slot_mosaicWorklistCheck(bool check);
    void    slot_tilingWorklistCheck(bool check);
    void    slot_mosaicWorklistXCheck(bool check);
    void    slot_tilingWorklistXCheck(bool check);
    void    slot_mosOrigCheck(bool check);
    void    slot_mosNewCheck(bool check);
    void    slot_mosTestCheck(bool check);
    void    slot_mosSortCheck(bool check);
    void    slot_showWithBkgds(bool check);

    void    slot_tilingOrigCheck(bool check);
    void    slot_tilingNewCheck(bool check);
    void    slot_tilingTestCheck(bool check);
    void    slot_tilingCheck(bool check);

    void    autoLoadLastClicked(bool enb);

    void    slot_kbdModeChanged(int row);
    void    slot_kbdMode(eLegacyMode mode);

    void    loadTiling2();

protected:
    void    refreshPanel();

    void    makeConnections();

    void    loadTilingsCombo();
    void    loadMosaicsCombo();
    void    loadDesignCombo();

    QGroupBox * createGeneralColumn();
    QGroupBox * createLegacyColumn();
    QGroupBox * createMosaicColumn();
    QGroupBox * createTilingColumn();

    void    loadTiling(eTILM_Event event);

    void    putNewTilingNameIntoMosaic(VersionFileList & designs, VersionedName newTilingName);
    bool    putNewTilingNameIntoTiling(VersionedFile tiling, VersionedName newTilingName);

    void    setupKbdModeCombo();

private:
    VersionedListWidget * tileListWidget;
    VersionedListWidget * mosaicListWidget;
    LoaderListWidget    * designListWidget;

    QThread     * thread;

    QPushButton * pbLoadShapes;
	QPushButton * pbLoadTiling;
    QPushButton * pbLoadXML;
    QPushButton * pbTilingLoadMulti;
    QPushButton * pbTilingLoadReplace;

    QCheckBox   * tilingFilterCheck;
    QLineEdit   * tilingFilter;

    QCheckBox   * tilingOrigChk;
    QCheckBox   * tilingNewChk;
    QCheckBox   * tilingTestChk;
    QCheckBox   * tilingWorklistCheck;
    QCheckBox   * tilingWorklistXCheck;

    QCheckBox   * mosaicFilterCheck;
    QCheckBox   * mosaicWorklistCheck;
    QCheckBox   * mosaicWorklistXCheck;
    QLineEdit   * mosaicFilter;

    QCheckBox   * mosaicOrigChk;
    QCheckBox   * mosaicNewChk;
    QCheckBox   * mosaicTestChk;
    QCheckBox   * cbMosaicSortChk;
    QCheckBox   * cbShowWithImages;

    eDesign       selectedDesign;
    VersionedFile selectedMosaicFile;
    VersionedFile selectedTilingFile;

    QCheckBox   * cbAutoLoadLast;

    QComboBox   * kbdModeCombo;

};

#endif
