#ifndef PAGE_MOTIF_MAKER_H
#define PAGE_MOTIF_MAKER_H

#include <QScrollArea>

#include "widgets/panel_page.h"
#include "enums/emotiftype.h"
#include "widgets/panel_misc.h"

class QComboBox;
class QCheckBox;

typedef std::shared_ptr<class Prototype>     PrototypePtr;
typedef std::shared_ptr<class Tile>          TilePtr;
typedef std::shared_ptr<class Motif>         MotifPtr;
typedef std::shared_ptr<class MotifButton>   MotifBtnPtr;

class page_motif_maker : public panel_page
{
    Q_OBJECT

public:
    page_motif_maker(ControlPanel * cpanel);

    void	onRefresh(void) override;
    void    onEnter() override;
    void    onExit() override;

    void     select(PrototypePtr prototype);
    TilePtr  getActiveTile();

public slots:
    void    slot_motifModified(MotifPtr motif);
    void    slot_motifTypeChanged(eMotifType type);
    void    slot_selectMotifButton(MotifBtnPtr fb);
    void    slot_tilingChoicesChanged();
    void    slot_tileChanged();
    void    slot_tilingChanged();

private slots:
    void    whiteClicked(bool state);
    void    replicateClicked(bool state);
    void    multiClicked(bool state);
    void    slot_combine();
    void    slot_duplicateCurrent();
    void    slot_deleteCurrent();
    void    slot_editCurrent();
    void    slot_prototypeSelected(int);
    void    slot_swapFeatureType();
    void    slot_rebuildCurrentFigure();

protected:
    AQWidget * createMotifWidget();

    void    setPrototype(PrototypePtr proto);
    void    setButtonTransforms();
    void    motifModified(MotifPtr fp);

private:
    class MotifSelector      * motifSelector;
    class MotifEditor        * motifEditor;
    class MotifMaker         * motifMaker;

    MotifBtnPtr              viewerBtn;

    QCheckBox                * whiteBackground;
    QCheckBox                * replicate;
    QComboBox                * tilingListBox;
};

#endif
