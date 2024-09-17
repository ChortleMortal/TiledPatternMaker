#pragma once
#ifndef PAGE_LOADERS_H
#define PAGE_LOADERS_H

#include "gui/panels/panel_page.h"
#include "gui/widgets/versioned_list_widget.h"
#include "sys/enums/estatemachineevent.h"
#include "sys/enums/edesign.h"
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
    void    sig_sortMosaics(bool,bool,bool);
    void    sig_sortTilings(bool,bool,bool);

public slots:
   void     slot_newTile();
   void     slot_newXML();

   void     slot_mosaicLoaded(VersionedFile file);
   void     slot_tilingLoaded(VersionedFile file);
   void     slot_loadedDesign(eDesign design);

   void     desRightClick(QPoint pos);
   void     xmlRightClick(QPoint pos);
   void     tileRightClick(QPoint pos);

   void     refillTilingsCombo();
   void     refillMosaicsCombo();

private slots:
    void    designSelected(QListWidgetItem * item, QListWidgetItem* oldItem);
    void    tilingSelected(QListWidgetItem * item, QListWidgetItem* oldItem);
    void    mosaicSelected(QListWidgetItem *item, QListWidgetItem *oldItem);
    void    designClicked(QListWidgetItem * item);
    void    tilingClicked(QListWidgetItem * item);
    void    mosaicClicked(QListWidgetItem *item);

    void    slot_mosaicItemEnteredToolTip(QListWidgetItem * item);

    void    loadShapes();
    void    slot_loadTiling();
    void    slot_loadTilingModify();
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
    void    showTilings();

    void    openTiling();
    void    rebaseTiling();
    void    renameTiling();
    void    deleteTiling();
    void    reformatTiling();

    void    slot_whereTilingUsed();
    void    slot_mosaicFilter(const QString & filter);
    void    slot_tilingFilter(const QString & filter);
    void    slot_mosaicCheck(bool check);
    void    slot_mosaicWorklistCheck(bool check);
    void    slot_tilingWorklistCheck(bool check);
    void    slot_mosOrigCheck(bool check);
    void    slot_mosNewCheck(bool check);
    void    slot_mosTestCheck(bool check);
    void    slot_mosSortCheck(bool check);
    void    slot_showWithBkgds(bool check);

    void    slot_tilingOrigCheck(bool check);
    void    slot_tilingNewCheck(bool check);
    void    slot_tilingTestCheck(bool check);
    void    slot_tilingCheck(bool check);

    void    autoLoadStylesClicked(bool enb);
    void    autoLoadTilingClicked(bool enb);
    void    autoLoadDesignsClicked(bool enb);

    void    loadTiling2();

protected:
    void    refreshPanel();

    void    makeConnections();

    void    loadTilingsCombo();
    void    loadMosaicsCombo();
    void    loadDesignCombo();

    QGroupBox * createLegacyColumn();
    QGroupBox * createMosaicColumn();
    QGroupBox * createTilingColumn();

    void    loadTiling(eTILM_Event event);

    void    putNewTilingNameIntoMosaic(VersionFileList & designs, VersionedName newTilingName);
    bool    putNewTilingNameIntoTiling(VersionedFile tiling, VersionedName newTilingName);

private:
    VersionedListWidget * tileListWidget;
    VersionedListWidget * mosaicListWidget;
    LoaderListWidget    * designListWidget;

    QThread     * thread;

    QPushButton * pbLoadShapes;
	QPushButton * pbLoadTiling;
    QPushButton * pbLoadXML;
    QPushButton * pbTilingLoadMulti;
    QPushButton * pbTilingLoadModify;

    QCheckBox   * tilingFilterCheck;
    QLineEdit   * tilingFilter;

    QCheckBox   * tilingOrigChk;
    QCheckBox   * tilingNewChk;
    QCheckBox   * tilingTestChk;
    QCheckBox   * tilingWorklistCheck;

    QCheckBox   * mosaicFilterCheck;
    QCheckBox   * mosaicWorklistCheck;
    QLineEdit   * mosaicFilter;

    QCheckBox   * mosaicOrigChk;
    QCheckBox   * mosaicNewChk;
    QCheckBox   * mosaicTestChk;
    QCheckBox   * mosaicSortChk;
    QCheckBox   * cbShowWithImages;

    eDesign       selectedDesign;
    VersionedFile selectedMosaicFile;
    VersionedFile selectedTilingFile;

    QCheckBox   * cbAutoLoadMosaics;
    QCheckBox   * cbAutoLoadTiling;
    QCheckBox   * cbAutoLoadDesigns;
};

#endif
