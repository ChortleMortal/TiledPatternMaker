#pragma once
#ifndef PAGE_MOTIF_MAKER_H
#define PAGE_MOTIF_MAKER_H

#include <QScrollArea>

#include "panels/panel_page.h"

class PrototypeData;
class MotifMakerWidget;

typedef std::shared_ptr<class Prototype>     ProtoPtr;
typedef std::shared_ptr<class Tile>          TilePtr;
typedef std::shared_ptr<class Motif>         MotifPtr;

class page_motif_maker : public panel_page
{
    Q_OBJECT

public:
    page_motif_maker(ControlPanel * cpanel);

    void	onRefresh(void) override;
    void    onEnter()       override;
    void    onExit()        override {}
    QString getPageStatus() override;

public slots:

private slots:
    void    replicateClicked(bool state);
    void    multiClicked(bool state);
    void    slot_combine();
    void    slot_duplicateCurrent();
    void    slot_deleteCurrent();
    void    slot_editCurrent();
    void    slot_swapTileRegularity();
    void    slot_rebuildMotif();
    void    slot_prototypeSelected(int row);
    void    slot_widthChanged(int val);
    void    slot_enlarge(bool checked);
    void    slot_actual(bool checked);

protected:
    void    loadProtoCombo();

private:
    MotifMakerWidget* motifMakerWidget;
    PrototypeData   * protoMakerData;
    QCheckBox       * replicate;
    QComboBox       * prototypeCombo;
    QLabel          * protoLabel;
};

#endif
