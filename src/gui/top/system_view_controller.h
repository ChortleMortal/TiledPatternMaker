#pragma once
#ifndef VIEW_CONTROL_H
#define VIEW_CONTROL_H

#include <QKeyEvent>

#include "gui/top/system_view_accessor.h"
#include "model/settings/canvas.h"
#include "sys/enums/eviewtype.h"
#include "sys/geometry/xform.h"
#include "sys/qt/unique_qvector.h"

typedef std::shared_ptr<class Border>           BorderPtr;
typedef std::shared_ptr<class LegacyBorder>     LegacyBorderPtr;
typedef std::shared_ptr<class Tile>             TilePtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Layer>            LayerPtr;
typedef std::weak_ptr<class Layer>             wLayerPtr;

class SystemViewController : public QObject, public SystemViewAccessor
{
    Q_OBJECT

    friend class PanelViewSelect;

public:
    SystemViewController();
    virtual ~SystemViewController();

    void    attach(SystemView * view);
    bool    isAttached()    { return (theView != nullptr) ;}

    void    unloadMakers();

    bool    isEnabled(eViewType view);  // Borders and crops are excluded
    uint    enabledViewCount()          { return enabledViewers.count(); }
    void    disableAllViews();

    static bool isGangMember(eViewType viewType);
    static QVector<eViewType> getGangMembers();

    void      setMostRecentGangEnable(eViewType view);
    eViewType getMostRecentGangEnable() { return _mostRecentGangEnable; }
    uint      enabledGangCount();
    void      disableGang();
    bool      isRadioGangSelection(eViewType);

    void      setSelectedPrimaryLayer(LayerPtr layer);
    LayerPtr  getSelectedPrimaryLayer() { return _selectedPrimaryLayer.lock(); }

    const   Xform & getSystemModelXform();
    void    setSystemModelXform(const Xform & xform, uint sigid);     // use with care

    Canvas& getCanvas()                 { return canvas; }
    QSize   getCanvasViewSize()         { return canvas.getViewSize(); }

    QString getBackgroundColor(eViewType vtype);
    void    setBackgroundColor(eViewType vtype, QColor color);
    void    setBackgroundColor(eViewType vtype);

    QSize   viewSize();

signals:
    void    sig_resetLayers();          // clears breakaways, solos, and locls
    void    sig_lock(Layer * l, bool set);
    void    sig_solo(Layer * l, bool set);
    void    sig_breakaway(Layer * l, bool set);
    void    sig_unloaded();

public slots:
    void    slot_updateView();
    void    slot_reconstructView();
    void    slot_unloadView();
    void    slot_unloadAll();

private:
    void    reconstructView();
    void    enableLayer(eViewType view);

    void    viewEnable(eViewType view, bool enable);

    Canvas                      canvas;
    Xform                       _systemModelXform;      // aka SMX
    wLayerPtr                   _selectedPrimaryLayer;
    eViewType                   _mostRecentGangEnable;
    UniqueQVector<eViewType>    enabledViewers;
    uint                        rx_sigid;
};

#endif
