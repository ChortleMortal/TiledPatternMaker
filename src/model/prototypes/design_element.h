#pragma once
#ifndef DESIGN_ELEMENT_H
#define DESIGN_ELEMENT_H

#include <QString>
#include <QTransform>

typedef std::shared_ptr<class Motif>              MotifPtr;
typedef std::shared_ptr<class Prototype>          ProtoPtr;
typedef std::shared_ptr<class Tile>               TilePtr;
typedef std::shared_ptr<class Tiling>             TilingPtr;
typedef std::weak_ptr<class Tiling>           WeakTilingPtr;
typedef std::shared_ptr<class DesignElement>      DELPtr;
typedef std::weak_ptr<class DesignElement>    WeakDELPtr;

////////////////////////////////////////////////////////////////////////////
//
// DesignElement.java
//
// A DesignElement is the core of the process of building a finished design.
// It's a Tile together with a Motif.  The tile comes from the
// tile library and will be used to determine where to place copies of the
// Motif, which is designed by the user.

class DesignElement
{
public:
    DesignElement(const TilingPtr & tiling);
    DesignElement(const TilingPtr & tiling, const TilePtr & tile, const MotifPtr & motif);
    DesignElement(const TilingPtr & tiling, const TilePtr & tile);
    DesignElement(const TilingPtr & tiling, const DELPtr & dep);
    DesignElement(const TilingPtr & tiling, const DesignElement & other);
    ~DesignElement();

    TilePtr     getTile()   const   { return tile; }
    MotifPtr    getMotif()  const   { return motif; }
    TilingPtr   getTiling() const   { return  tiling.lock(); }

    void        replaceTile(const TilePtr & tile);
    void        exactReplaceTile(const TilePtr & tile);

    void        setMotif(const MotifPtr & motif);
    bool        validMotifRegularity();
    void        createMotifFromTile();
    void        recreateMotifWhenRgularityChanged();
    void        recreateMotifFromChangedTile();

    QString     toString();
    void        describe();

    static int  refs;

protected:
    TilePtr       tile;
    MotifPtr	  motif;
    WeakTilingPtr tiling;  // 27JAN26 - if this was done earlier would have simplified code
};

#endif

