#ifndef DESIGN_ELEMENT_H
#define DESIGN_ELEMENT_H

#include <QString>
#include <QTransform>

typedef std::shared_ptr<class Motif>          MotifPtr;
typedef std::shared_ptr<class Tile>         TilePtr;
typedef std::shared_ptr<class DesignElement>   DesignElementPtr;

////////////////////////////////////////////////////////////////////////////
//
// DesignElement.java
//
// A DesignElement is the core of the process of building a finished design.
// It's a Tile together with a Motif.  The tile comes from the
// tile library and will be used to determine where to place copies of the
// MOtif, which is designed by the user.

class DesignElement
{
public:

    DesignElement();
    DesignElement(const TilePtr & tile, const MotifPtr & motif);
    DesignElement(const TilePtr & tiling);
    DesignElement(const DesignElementPtr & dep);
    DesignElement(const DesignElement & other);
    ~DesignElement();

    TilePtr     getTile() const;
    void        replaceTile(const TilePtr & tile);


    MotifPtr    getMotif() const;
    void        setMotif(const MotifPtr & motif);
    bool        validMotif();
    void        createMotif();

    QString     toString();
    void        describe();

    static int  refs;

protected:
    TilePtr     tile;
    MotifPtr	motif;
};

#endif

