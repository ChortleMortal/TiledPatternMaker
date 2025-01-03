#pragma once
#ifndef STYLE_H
#define STYLE_H

#include "gui/viewers/layer_controller.h"
#include "sys/enums/estyletype.h"
#include "model/prototypes/prototype.h"

#if (QT_VERSION < QT_VERSION_CHECK(6,5,0))
#include <QDebug>
#endif

class GeoGraphics;
class DebugMap;

typedef std::shared_ptr<class Style>        StylePtr;
typedef std::shared_ptr<class Prototype>    ProtoPtr;
typedef std::shared_ptr<class Map>          MapPtr;
typedef std::shared_ptr<class Tiling>       TilingPtr;


////////////////////////////////////////////////////////////////////////////
//
// Style.java
//
// A style encapsulates drawing a map with some interesting style.

class QSvgGenerator;

class Style : public LayerController
{
    Q_OBJECT

public:
    // Creation.
    Style(const ProtoPtr & proto);
    Style(const StylePtr & other);

    virtual ~Style();

    // Geometry data.
    ProtoPtr    getPrototype() const {return prototype;}
    void        setPrototype(ProtoPtr pp);

    MapPtr      getExistingProtoMap();

    TilingPtr   getTiling();

    // Retrieve a name describing this style and map.


    const Xform &   getModelXform() override;
    void            setModelXform(const Xform & xf, bool update) override;

    // Overridable behaviours
    virtual void resetStyleRepresentation()  {}; // Called when the map is changed to clear any internal map representation.
    virtual void createStyleRepresentation() { prototype->getProtoMap(true); }; // Called to ensure there is an internal map representation, if needed.

    virtual QString     getStyleDesc() const = 0; // Retrieve the style description.
    virtual eStyleType  getStyleType() const = 0;

    virtual void    draw(GeoGraphics * gg)   = 0;
    virtual void    paint(QPainter *painter) override;
            void    paintToSVG();
            void    triggerPaintSVG(QSvgGenerator * generator) { this->generator = generator; paintSVG = true; }

    virtual void        dump() const = 0;
            QString     getInfo() const;
            QString     getDescription() const;
    virtual QString     getLayerName() override { return LayerController::getLayerName() + "-" + getDescription(); }

    virtual eViewType iamaLayer() override { return VIEW_MOSAIC; }
    void              iamaLayerController() override {}

    static int refs;

public slots:
    void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;
    void slot_mouseDoublePressed(QPointF spt) override;

protected:
    void   annotateEdges(MapPtr map);
    void   drawAnnotation(QPainter *painter, QTransform T);

    ProtoPtr      prototype; // Contains the map to be rendered (the input geometry)

private:
    DebugMap *    debugMap;

    bool          paintSVG;

    QSvgGenerator * generator;
};
#endif
