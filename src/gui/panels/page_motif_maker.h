#pragma once

#include <QScrollArea>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>

#include "gui/panels/panel_page.h"

class MotifMakerWidget;
class SMXWidget;

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
    void    onExit()        override;

public slots:

private slots:
    void    replicateClicked(bool state);
    void    enlargedClicked();
    void    selectedClicked();
    void    tileUnitClicked();
    void    slot_combine();
    void    slot_saveAsTile();
    void    slot_duplicateCurrent();
    void    slot_deleteCurrent();
    void    slot_editCurrent();
    void    slot_rebuildMotif();
    void    slot_prototypeSelected(int row);
    void    slot_widthChanged(int val);

protected:
    bool    protosHaveChanged();
    void    loadProtoCombo();
    void    setPageStatusString();

private:
    MotifMakerWidget* motifMakerWidget;
    QCheckBox       * replicate;
    QComboBox       * prototypeCombo;
    QLabel          * protoLabel;
    SMXWidget       * smxWidget;
};
