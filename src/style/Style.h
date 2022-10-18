#ifndef STYLE_H
#define STYLE_H

#include "misc/layer_controller.h"
#include "enums/estyletype.h"

class GeoGraphics;

typedef std::shared_ptr<class Style>        StylePtr;
typedef std::shared_ptr<class Prototype>    PrototypePtr;
typedef std::shared_ptr<class Map>          MapPtr;
typedef std::shared_ptr<class DebugMap>     DebugMapPtr;
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
    Style(const PrototypePtr & proto);
    Style(const StylePtr & other);

    ~Style() override;

    // Geometry data.
    PrototypePtr getPrototype() const {return prototype;}
    void         setPrototype(PrototypePtr pp);

    MapPtr       getMap();
    MapPtr       getExistingMap();
    void         setMap(MapPtr map) {styleMap = map; }

    TilingPtr    getTiling();

    // Retrieve a name describing this style and map.
    QString getDescription() const;
    QString getInfo() const;
    virtual QString getName() override { return LayerController::getName() + "-" + getDescription(); }

    virtual const Xform  & getCanvasXform() override;
    virtual void    setCanvasXform(const Xform & xf) override;

    // Overridable behaviours

    virtual void createStyleRepresentation() = 0; // Called to ensure there is an internal map representation, if needed.
    virtual void resetStyleRepresentation()  = 0; // Called when the map is changed to clear any internal map representation.

    virtual QString     getStyleDesc() const = 0; // Retrieve the style description.
    virtual eStyleType  getStyleType() const = 0;

    virtual void    draw(GeoGraphics * gg) = 0;
            void    paint(QPainter *painter) override;
            void    paintToSVG();

    void    triggerPaintSVG(QSvgGenerator * generator) { this->generator = generator; paintSVG = true; }

    void iamaLayer() override {}
    void iamaLayerController() override {}

    static int refs;

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseMoved(QPointF spt)         override;
    virtual void slot_mouseReleased(QPointF spt)      override;
    virtual void slot_mouseDoublePressed(QPointF spt) override;

protected:
    void   resetStyleMap();

    void   annotateEdges(MapPtr map);
    void   drawAnnotation(QPainter *painter, QTransform T);

private:
    PrototypePtr  prototype; // The input geometry to be rendered
    MapPtr        styleMap;
    DebugMapPtr   debugMap;

    bool          paintSVG;
    QSvgGenerator * generator;

    Xform         xf_canvas;
};
#endif
