#pragma once
#ifndef PROTOTYPE_DATA_H
#define PROTOTYPE_DATA_H

#include <QVector>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif
typedef std::shared_ptr<class Motif>            MotifPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Tile>             TilePtr;

typedef std::weak_ptr<class Prototype>          WeakProtoPtr;
typedef std::weak_ptr<class DesignElement>      WeakDELPtr;

class MotifMakerWidget;

#define NUM_MVD 2
enum eMVDType
{
    MVD_DELEM = 0,
    MVD_PROTO = 1
};

class ProtoInfo
{
public:
    ProtoInfo() { show[MVD_DELEM] = false; show[MVD_PROTO] = false; }
    ProtoInfo(WeakProtoPtr wpp, bool showMotif, bool showProto)
            { wproto = wpp; show[MVD_DELEM] = showMotif; show[MVD_PROTO] = showProto; }
    ProtoInfo(const ProtoInfo & other)
            { wproto = other.wproto; show[MVD_DELEM] = other.show[MVD_DELEM]; show[MVD_PROTO] = other.show[MVD_PROTO];}

    ProtoInfo operator=(const ProtoInfo& other) { return ProtoInfo(other); }

    WeakProtoPtr wproto;
    bool         show[NUM_MVD];
};

class DelInfo
{
public:
    DelInfo() { qCritical("DelInfo default constructor"); }
    DelInfo(WeakDELPtr wDEL, bool showMotif, bool showProto)
            { wdel = wDEL; show[MVD_DELEM] = showMotif; show[MVD_PROTO] = showProto; }

    WeakDELPtr wdel;
    bool       show[NUM_MVD];
};

class PrototypeData
{
public:
    PrototypeData();

    void setWidget(MotifMakerWidget * widget);

    void setPrototypes(QVector<ProtoPtr> & protos);

    void erase();
    void select(eMVDType type, ProtoPtr proto,                 bool multi, bool hidden = false);
    void select(eMVDType type, DesignElementPtr designElement, bool multi, bool hidden = false);

    void hide(eMVDType type,ProtoPtr proto,       bool hide);
    void hide(eMVDType type,DesignElementPtr del, bool hide);

    bool isHidden(eMVDType type,ProtoPtr proto);
    bool isHidden(eMVDType type,DesignElementPtr del);
    bool isHidden(eMVDType type,MotifPtr motif);

    ProtoPtr                    getSelectedPrototype()  { return selectedPrototype.lock(); }
    QVector<ProtoPtr>           getSelectedPrototypes(eMVDType type);
    QVector<ProtoPtr>           getPrototypes();

    DesignElementPtr            getSelectedDEL()        { return selectedDesignElement.lock(); }
    QVector<DesignElementPtr>   getSelectedDELs(eMVDType type);
    QVector<DesignElementPtr>   getDELs();

    void                        remove(ProtoPtr proto);
    void                        remove(TilingPtr tiling);
    void                        add(ProtoPtr proto);

    ProtoPtr                    getPrototype(TilingPtr tiling);
    DesignElementPtr            getDesignElement(TilePtr tile);

    void                        rebuildCurrentMotif();                 // used only for debug

private:
    MotifMakerWidget *          motifMakerWidget;

    WeakProtoPtr                selectedPrototype;
    QVector<ProtoInfo>          prototypes;
    WeakDELPtr                  selectedDesignElement;
    QVector<DelInfo>            designElements;
};

#endif
