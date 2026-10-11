#pragma once

#include <QKeyEvent>

#include "gui/top/view_accessor.h"
#include "gui/viewers/layer.h"
#include "model/settings/canvas.h"
#include "sys/geometry/xform.h"
#include "sys/qt/unique_qvector.h"

typedef std::shared_ptr<class Border>           BorderPtr;
typedef std::shared_ptr<class LegacyBorder>     LegacyBorderPtr;
typedef std::shared_ptr<class Tile>             TilePtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Layer>            LayerPtr;
typedef std::weak_ptr<class Layer>             wLayerPtr;

class ViewController : public QObject, public ViewAccessor
{
    Q_OBJECT

    friend class PanelViewSelect;

public:
    ViewController();
    virtual ~ViewController();

    void    unloadMakers();

    bool    isEnabled(Layer::Type view);  // Borders and crops are excluded
    uint    enabledViewCount()          { return enabledViewers.count(); }
    void    disableAllViews();

    static bool isGangMember(Layer::Type type);
    static QVector<Layer::Type> getGangMembers();

    void      setMostRecentGangEnable(Layer::Type view);
    Layer::Type getMostRecentGangEnable() { return _mostRecentGangEnable; }
    uint      enabledGangCount();
    void      disableGang();
    bool      isRadioGangSelection(Layer::Type type);

    void      setSelectedPrimaryLayer(LayerPtr layer);
    LayerPtr  getSelectedPrimaryLayer() { return _selectedPrimaryLayer.lock(); }

    void      setCanvas(Canvas * canvas)  { this->canvas = canvas; }
    Canvas *  getCanvas()                 { Q_ASSERT(canvas); return canvas; }

    const   Xform & getSystemModelXform();
    void    setSystemModelXform(const Xform & xform, uint sigid);     // use with care

    //void    setBkgdColor(eViewType vtype);

    QSize   viewSize();
    void    bringToPrimaryScreen();

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
    void    populateLayer(Layer::Type view);

    void    viewEnable(Layer::Type view, bool enable);

    Canvas *                     canvas = nullptr;
    Xform                       _systemModelXform;      // aka SMX
    wLayerPtr                   _selectedPrimaryLayer;
    Layer::Type                 _mostRecentGangEnable;
    UniqueQVector<Layer::Type>   enabledViewers;
    uint                         rx_sigid;
};
