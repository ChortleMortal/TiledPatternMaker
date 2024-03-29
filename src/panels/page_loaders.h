#pragma once
#ifndef PAGE_LOADERS_H
#define PAGE_LOADERS_H

#include "panels/panel_page.h"
#include "widgets/versioned_list_widget.h"
#include "enums/estatemachineevent.h"
#include "enums/edesign.h"

class page_loaders : public panel_page
{
    Q_OBJECT

public:
    explicit page_loaders(ControlPanel * apanel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override {}

    static int  whereTilingUsed(QString name, QStringList & results);

signals:
    void    sig_loadDesign(eDesign id);
    void    sig_buildDesign(eDesign id);

public slots:
   void     slot_newTile(QString name);
   void     slot_newXML();

   void     slot_mosaicLoaded(QString name);
   void     slot_tilingLoaded (QString name);
   void     slot_loadedDesign(eDesign design);

   void     desRightClick(QPoint pos);
   void     xmlRightClick(QPoint pos);
   void     tileRightClick(QPoint pos);

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

    void    loadMosaic();
    void    openXML();
#ifdef Q_OS_WINDOWS
    void    showXMLDir();
#endif
    void    rebaseXML();
    void    renameXML();
    void    deleteXML();
    void    addToWorklist();
    void    showTilings();

    //void    tilingLoadChanged(int id);
    void    openTiling();
    void    rebaseTiling();
    void    renameTiling();
    void    deleteTiling();

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
    void    loadMosaicCombo();
    void    loadDesignCombo();

    QGroupBox * createLegacyColumn();
    QGroupBox * createMosaicColumn();
    QGroupBox * createTilingColumn();

    void    loadTiling(eTILM_Event event);

    void    putNewTilingNameIntoDesign(QStringList & designs, QString newName);
    bool    putNewTilingNameIntoTiling(QString filename, QString newName);

    QStringList sortMosaicsByDate(const QStringList &names);
    QStringList sortTilingsByDate(const QStringList &names);

    QStringList findMosaicsWithBkgds(const QStringList & names);
    QStringList findTilingsWithBkgds(const QStringList & names);

private:
    VersionedListWidget * tileList;
    VersionedListWidget * mosaicList;
    LoaderListWidget    * designList;

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
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
    QCheckBox   * mosaicSortChk;
#endif
    QCheckBox   * cbShowWithImages;

    eDesign       selectedDesign;
    QString       selecteMosaicName;
    QString       selectedTilingName;

    QCheckBox   * cbAutoLoadMosaics;
    QCheckBox   * cbAutoLoadTiling;
    QCheckBox   * cbAutoLoadDesigns;
};

#endif
