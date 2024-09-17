#pragma once

#ifndef BORDER_H
#define BORDER_H

#include "sys/geometry/crop.h"
#include "sys/geometry/faces.h"
#include "model/styles/style.h"

typedef std::shared_ptr<class MouseEditBorder> BorderMouseActionPtr;

typedef std::shared_ptr<class Map> MapPtr;

class Border : public Style, public Crop
{
    Q_OBJECT

public:
    virtual void createStyleRepresentation() override  = 0;
    virtual void resetStyleRepresentation()  override  = 0; // Called when the map is changed to clear any internal map representation.
    virtual void paint(QPainter *painter)    override;

    virtual void draw(GeoGraphics * gg)      override  = 0 ;

    virtual QString     getStyleDesc() const override { return "Border"; }
    virtual eStyleType  getStyleType() const override { return STYLE_BORDER; }
    virtual void        dump()         const override {}

    eBorderType  getBorderType()       { return borderType; }
    QString      getBorderTypeString() { return sBorderType[borderType]; }
    QString      getCropTypeString()   { return sCropType[_cropType]; }
    bool         getUseViewSize()      { return _useViewSize; }

    void    setWidth(qreal width)       { borderWidth = width; setRequiresConstruction(true); }
    void    setColor(QColor color)      { this->color = color; setRequiresConstruction(true); }
    void    setColor2(QColor color)     { color2 = color;      setRequiresConstruction(true); }
    void    setUseViewSize(bool use);

    qreal   getWidth()  { return borderWidth; }
    QColor  getColor()  { return color; }
    QColor  getColor2() { return color2; }

    virtual void legacy_convertToModelUnits() = 0;

    void    setRequiresConversion(bool enb)   { _requiresConversion = enb; }
    bool    getRequiresConversion()           { return _requiresConversion; }

    void    setRequiresConstruction(bool enb) { _requiresConstruction = enb; }
    bool    getRequiresConstruction()         { return _requiresConstruction; }

    void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseTranslate(QPointF pt)      override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;

    void    setMousePos(QPointF pt);
    QPointF getMousePos() { return mousePos; }

public slots:
    virtual void viewResized(QSize oldSize, QSize newSize);
    virtual void viewMoved();

protected:
    Border(ProtoPtr proto);
    Border(ProtoPtr proto, const Crop & crop);

    void                  setMouseInteraction(BorderMouseActionPtr action) { mouse_interaction = action; }
    void                  resetMouseInteraction() { mouse_interaction.reset(); }
    BorderMouseActionPtr  getMouseInteraction() { return mouse_interaction; }

    void            convertCropToModelUnits();
    void            setBorderSize(QSize viewSize);

    eBorderType     borderType;

    qreal           borderWidth;
    QColor          color;
    QColor          color2;
    bool            _useViewSize;

private:
    bool            _requiresConversion;
    bool            _requiresConstruction;
    QPointF         mousePos;             // used by menu
    bool            debugMouse;

    BorderMouseActionPtr   mouse_interaction;    // used by menu
};

class BorderPlain : public Border
{
public:
    BorderPlain(ProtoPtr proto, const Crop &crop, qreal width, QColor color);
    BorderPlain(ProtoPtr proto, Circle c,         qreal width, QColor color);
    BorderPlain(ProtoPtr proto, QRectF rect,      qreal width, QColor color);
    BorderPlain(ProtoPtr proto, QPolygonF poly,   qreal width, QColor color);
    BorderPlain(ProtoPtr proto, QSizeF sz,        qreal width, QColor color);

    virtual void createStyleRepresentation() override;
    virtual void resetStyleRepresentation() override {}
    virtual void draw(GeoGraphics * gg) override;

    void get(qreal & width, QColor & color);

    void legacy_convertToModelUnits() override;

    MapPtr  bmap;
};

class BorderTwoColor : public Border
{
public:
    BorderTwoColor(ProtoPtr proto, QSizeF sz,   QColor color1, QColor color2, qreal width, qreal len);
    BorderTwoColor(ProtoPtr proto, QRectF rect, QColor color1, QColor color2, qreal width, qreal len);

    virtual void createStyleRepresentation() override;
    virtual void resetStyleRepresentation() override {}
    virtual void draw(GeoGraphics * gg) override;

    qreal   getLength()             { return segmentLen; }
    void    setLength(qreal length) { segmentLen = length;  createStyleRepresentation(); }

    void    get(QColor & color1, QColor & color2, qreal & width, qreal &length);

    void    legacy_convertToModelUnits() override;

protected:
    QColor  nextBorderColor();
    void    addSegment(qreal x, qreal y, qreal width, qreal height);

    qreal   segmentLen;

private:
    FaceSet faces;
};

class BorderBlocks : public Border
{
public:
    BorderBlocks(ProtoPtr proto, QSizeF sz,   QColor color, int rows, int cols, qreal width);
    BorderBlocks(ProtoPtr proto, QRectF rect, QColor color, int rows, int cols, qreal width);

    virtual void createStyleRepresentation() override;
    virtual void resetStyleRepresentation() override {}
    virtual void draw(GeoGraphics * gg) override;

    void    get(QColor & color1, int & rows, int & cols, qreal & width);
    void    setRows(int rows)   { this->rows = rows; createStyleRepresentation(); }
    void    setCols(int cols)   { this->cols = cols; createStyleRepresentation(); }

    void    legacy_convertToModelUnits() override;

protected:

private:
    int         rows;
    int         cols;
    FaceSet     faces;
};
#endif

