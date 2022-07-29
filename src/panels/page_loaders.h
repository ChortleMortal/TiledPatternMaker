#ifndef PAGE_LOADERS_H
#define PAGE_LOADERS_H

class QCheckBox;
class QPushButton;

#include "widgets/panel_page.h"
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
    void    sig_loadTiling(QString,eSM_Event);
    void    sig_loadMosaic(QString name, bool ready);
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

    void    slot_itemEnteredToolTip(QListWidgetItem * item);

    void    loadShapes();
    void    loadTiling();

    void    loadXML();
    void    openXML();
    void    rebaseXML();
    void    renameXML();
    void    deleteXML();
    void    showTilings();

    //void    tilingLoadChanged(int id);
    void    openTiling();
    void    rebaseTiling();
    void    renameTiling();
    void    deleteTiling();
    void    slot_whereTilingUsed();

    void    loadTilingsCombo();
    void    loadMosaicCombo();
    void    loadDesignCombo();

    void    slot_mosaicFilter(const QString & filter);
    void    slot_tilingFilter(const QString & filter);
    void    slot_mosaicCheck(bool check);
    void    slot_mosaicWorklistCheck(bool check);
    void    slot_mosOrigCheck(bool check);
    void    slot_mosNewCheck(bool check);
    void    slot_mosTestCheck(bool check);
    void    slot_tilingCheck(bool check);

    void    autoLoadStylesClicked(bool enb);
    void    autoLoadTilingClicked(bool enb);
    void    autoLoadDesignsClicked(bool enb);

protected:
    void    setupUI();
    void    refreshPanel();
    void    makeConnections();
    void    putNewTilingNameIntoDesign(QStringList & designs, QString newName);
    bool    putNewTilingNameIntoTiling(QString filename, QString newName);

private:
    VersionedListWidget * tileList;
    VersionedListWidget * mosaicList;
    LoaderListWidget    * designList;

    QPushButton * pbLoadShapes;
	QPushButton * pbLoadTiling;
    QPushButton * pbLoadXML;

    QCheckBox   * cbLoadMulti;
    QCheckBox   * cbLoadModify;

    QCheckBox   * mosaicFilterCheck;
    QCheckBox   * mosaicWorklistCheck;
    QLineEdit   * mosaicFilter;

    QCheckBox   * mosOrigChk;
    QCheckBox   * mosNewChk;
    QCheckBox   * mosTestChk;

    QCheckBox   * tilingFilterCheck;

    QLineEdit   * tilingFilter;

    eDesign     selectedDesign;
    QString     selectedXMLName;
    QString     selectedTilingName;

    QCheckBox    * cbAutoLoadMosaics;
    QCheckBox    * cbAutoLoadTiling;
    QCheckBox    * cbAutoLoadDesigns;
};

#endif
