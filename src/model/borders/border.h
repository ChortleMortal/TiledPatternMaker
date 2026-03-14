#pragma once
#ifndef BORDER_H
#define BORDER_H

#include "sys/geometry/crop.h"
#include "model/styles/style.h"

typedef std::shared_ptr<class MouseEditBorder> BorderMouseActionPtr;

typedef std::shared_ptr<class Map> MapPtr;

class Border : public Style, public Crop
{
    Q_OBJECT

public:
    virtual void        createStyleRepresentation() override  = 0;
    virtual void        resetStyleRepresentation()  override { styled = false; }

    virtual MapPtr      getProtoMap()       override     { return nullptr; }
    virtual MapPtr      getStyleMap()       override     { return nullptr; }
    virtual void        setStyleMap(MapPtr map) override { Q_UNUSED(map); }

    virtual void        paint(QPainter *painter)    override;
    virtual void        draw(GeoGraphics * gg)      override  = 0 ;

    virtual void        setModelXform(const Xform & xf, bool update, uint sigid) override;

    virtual void        legacy_convertToModelUnits() = 0;

    virtual QString     getStyleDesc() const override { return "Border"; }
    virtual eStyleType  getStyleType() const override { return STYLE_BORDER; }
    virtual void        dump()         const override {}

    eBorderType         getBorderType()       { return borderType; }
    QString             getBorderTypeString() { return sBorderType[borderType]; }
    QString             getCropTypeString()   { return sCropType[_cropType]; }

    void                setUseViewSize(bool set);
    bool                getUseViewSize()            { return _useViewSize; }

    void                setWidth(qreal width)       { borderWidth = width; resetStyleRepresentation(); }
    void                setColor(QColor color)      { this->color = color; resetStyleRepresentation(); }
    void                setColor2(QColor color)     { color2 = color;      resetStyleRepresentation(); }

    qreal               getWidth()  { return borderWidth; }
    QColor              getColor()  { return color; }
    QColor              getColor2() { return color2; }

    void                setRequiresConversion(bool enb)   { _requiresConversion = enb; }
    bool                getRequiresConversion()           { return _requiresConversion; }

    void                slot_mousePressed(QPointF spt, Qt::MouseButton btn) override;
    void                slot_mouseDragged(QPointF spt)       override;
    void                slot_mouseMoved(QPointF spt)         override;
    void                slot_mouseReleased(QPointF spt)      override;

    void                setMousePos(QPointF pt);
    QPointF             getMousePos() { return mousePos; }

public slots:
    virtual void        viewResized(QSize oldSize, QSize newSize);
    virtual void        viewMoved();

protected:
    Border(Mosaic * parent);
    Border(Mosaic * parent, const Crop & crop);

    void                  setMouseInteraction(BorderMouseActionPtr action) { mouse_interaction = action; }
    void                  resetMouseInteraction() { mouse_interaction.reset(); }
    BorderMouseActionPtr  getMouseInteraction() { return mouse_interaction; }

    void                  convertCropToModelUnits();
    void                  setBorderSize(QSize viewSize);

    eBorderType borderType;

    qreal       borderWidth;
    QColor      color;
    QColor      color2;
    bool        _useViewSize;

    Mosaic *    parent;

private:
    bool        _requiresConversion;
    QPointF     mousePos;             // used by menu
    bool        debugMouse;

    BorderMouseActionPtr   mouse_interaction;    // used by menu
};

#endif

