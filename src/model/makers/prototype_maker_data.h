#pragma once
#ifndef PROTOTYPE_DATA_H
#define PROTOTYPE_DATA_H

#include <QVector>

typedef std::shared_ptr<class Motif>            MotifPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Tile>             TilePtr;

typedef std::weak_ptr<class Prototype>          WeakProtoPtr;
typedef std::weak_ptr<class DesignElement>      WeakDELPtr;

typedef QVector<DesignElementPtr>   DesignUnit;

class MotifMakerWidget;
class PrototypeMaker;

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
    const QVector<ProtoPtr>     getPrototypes();
    DesignUnit                  getAllDELs();

    ProtoPtr                    getSelectedPrototype()  { return selectedPrototype.lock(); }
    DesignElementPtr            getSelectedDEL()        { return selectedDesignElement.lock(); }
    DesignUnit                  getSelectedDELs(eMVDType type);

    bool isHidden(eMVDType type,ProtoPtr proto);
    bool isHidden(eMVDType type,DesignElementPtr del);
    bool isHidden(eMVDType type,MotifPtr motif);

    void select(  eMVDType type, ProtoPtr proto,                 bool multi, bool hidden = false);
    void select(  eMVDType type, DesignElementPtr designElement, bool multi, bool hidden = false);
    void deselect(eMVDType type, DesignElementPtr designElement, bool multi, bool hidden = false);

    bool isSelected(DesignElementPtr designElement)     { return (designElement == selectedDesignElement.lock()); }
    bool isSelected(ProtoPtr proto)                     { return (proto == selectedPrototype.lock()); }

    void                        dumpData(eMVDType type);
    void                        rebuildCurrentMotif();                 // used only for debug

protected:
    ProtoMakerData();

    void                        initData(PrototypeMaker * parent) { this->parent = parent; }

    void                        setPrototypes(QVector<ProtoPtr> & protos);
    void                        setPrototype(ProtoPtr proto);

    ProtoPtr                    getPrototype(TilingPtr tiling);
    const QVector<ProtoPtr>     getSelectedPrototypes(eMVDType type);

    void                        erase();
    void                        add(ProtoPtr proto);
    ProtoPtr                    remove(TilingPtr tiling);
    void                        remove(ProtoPtr proto);

    DesignElementPtr            getDesignElement(TilePtr tile);

    void                        hide(eMVDType type,ProtoPtr proto,       bool hide);
    void                        hide(eMVDType type,DesignElementPtr del, bool hide);

private:
    PrototypeMaker *    parent;
    WeakProtoPtr        selectedPrototype;
    QVector<ProtoInfo>  prototypes;
    WeakDELPtr          selectedDesignElement;
    QVector<DelInfo>    designElements;
};

#endif
