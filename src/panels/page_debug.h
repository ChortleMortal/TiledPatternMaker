#pragma once
#ifndef PAGE_DEBUG_H
#define PAGE_DEBUG_H

class QGroupBox;

#include "misc/qtapplog.h"
#include "widgets/panel_page.h"

typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Mosaic>  MosaicPtr;

class page_debug : public panel_page
{
    Q_OBJECT

public:
    page_debug(ControlPanel * cpanel);

    void    onRefresh() override;
    void    onEnter() override;
    void    onExit() override;

private slots:
    void    slot_reformatDesignXML();
    void    slot_reprocessDesignXML();
    void    slot_examineMosaic();
    void    slot_examineAllMosaics();

    void    slot_verifyProtosClicked(bool enb);
    void    slot_verifyMapsClicked(bool enb);
    void    slot_verifyDumpClicked(bool enb);
    void    slot_verifypopupClicked(bool enb);
    void    slot_verifyVerboseClicked(bool enb);

    void    slot_reprocessTilingXML();
    void    slot_reformatTilingXML();
    void    slot_verifyTilingNames();
    void    slot_verifyTiling();
    void    slot_verifyAllTilings();

    void    slot_testA();
    void    slot_testB();
    void    slot_dontTrapLog(bool dont);

protected:
    bool    verifyTiling(TilingPtr tiling);
    void    identifyDuplicateTiles(TilingPtr tiling);
    void    examineMosaic(MosaicPtr mosaic);

    QGroupBox   * createDebugSection();
    QGroupBox   * createVerifyMaps();

private:
    qtAppLog    * log;
};

#endif
