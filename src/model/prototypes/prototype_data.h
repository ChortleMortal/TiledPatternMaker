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

/*
 * The Prototype Maker puts information into the ProtoTypeData which is
 * read (shared) directly by both the PrototypeMakerView and the Motif Maker View.
 * The controlling menus for these makers use specific indices into the data
 * MVD_DELEM the Motif (designEelement) view and MVP_PROT for the proto view)
 *
 */

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

class ProtoMakerData
{
public:
    ProtoMakerData();

    void setPrototypes(QVector<ProtoPtr> & protos);

    void erase();
    void select(  eMVDType type, ProtoPtr proto,                 bool multi, bool hidden = false);
    void select(  eMVDType type, DesignElementPtr designElement, bool multi, bool hidden = false);
    void deselect(eMVDType type, DesignElementPtr designElement, bool multi, bool hidden = false);

    void hide(    eMVDType type,ProtoPtr proto,       bool hide);
    void hide(    eMVDType type,DesignElementPtr del, bool hide);

    bool isHidden(eMVDType type,ProtoPtr proto);
    bool isHidden(eMVDType type,DesignElementPtr del);
    bool isHidden(eMVDType type,MotifPtr motif);

    bool isSelected(DesignElementPtr designElement)     { return (designElement == selectedDesignElement.lock()); }
    bool isSelected(ProtoPtr proto)                     { return (proto == selectedPrototype.lock()); }

    ProtoPtr                    getSelectedPrototype()  { return selectedPrototype.lock(); }
    void                        remove(TilingPtr tiling);

    ProtoPtr                    getPrototype(TilingPtr tiling);
    const QVector<ProtoPtr>     getSelectedPrototypes(eMVDType type);
    const QVector<ProtoPtr>     getPrototypes();

    DesignElementPtr            getSelectedDEL()        { return selectedDesignElement.lock(); }
    QVector<DesignElementPtr>   getSelectedDELs(eMVDType type);
    QVector<DesignElementPtr>   getDELs();

    void                        remove(ProtoPtr proto);
    void                        add(ProtoPtr proto);

    DesignElementPtr            getDesignElement(TilePtr tile);

    void                        rebuildCurrentMotif();                 // used only for debug

    void setWidget(MotifMakerWidget * widget) { _motifMakerWidget = widget; }
    MotifMakerWidget * getWidget()            { return _motifMakerWidget; }

    void                        dumpData(eMVDType type);

private:
    MotifMakerWidget *          _motifMakerWidget;

    WeakProtoPtr                selectedPrototype;
    QVector<ProtoInfo>          prototypes;
    WeakDELPtr                  selectedDesignElement;
    QVector<DelInfo>            designElements;
};

#endif
