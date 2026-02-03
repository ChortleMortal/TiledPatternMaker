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
    virtual void resetStyleRepresentation()  override    { created = false; }

    virtual MapPtr      getProtoMap()       override     { return nullptr; }
    virtual MapPtr      getStyleMap()       override     { return nullptr; }
    virtual void        setStyleMap(MapPtr map) override { Q_UNUSED(map); }

    virtual void paint(QPainter *painter)    override;
    virtual void draw(GeoGraphics * gg)      override  = 0 ;

    virtual void setModelXform(const Xform & xf, bool update, uint sigid) override;

    virtual QString     getStyleDesc() const override { return "Border"; }
    virtual eStyleType  getStyleType() const override { return STYLE_BORDER; }
    virtual void        dump()         const override {}

    eBorderType  getBorderType()       { return borderType; }
    QString      getBorderTypeString() { return sBorderType[borderType]; }
    QString      getCropTypeString()   { return sCropType[_cropType]; }
    bool         getUseViewSize()      { return _useViewSize; }

    void    setWidth(qreal width)       { borderWidth = width; resetStyleRepresentation(); }
    void    setColor(QColor color)      { this->color = color; resetStyleRepresentation(); }
    void    setColor2(QColor color)     { color2 = color;      resetStyleRepresentation(); }
    void    setUseViewSize(bool use);

    qreal   getWidth()  { return borderWidth; }
    QColor  getColor()  { return color; }
    QColor  getColor2() { return color2; }

    virtual void legacy_convertToModelUnits() = 0;

    void    setRequiresConversion(bool enb)   { _requiresConversion = enb; }
    bool    getRequiresConversion()           { return _requiresConversion; }

    void slot_mousePressed(QPointF spt, Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;

    void    setMousePos(QPointF pt);
    QPointF getMousePos() { return mousePos; }

public slots:
    virtual void viewResized(QSize oldSize, QSize newSize);
    virtual void viewMoved();

protected:
    Border(Mosaic * parent);
    Border(Mosaic * parent, const Crop & crop);

    void                  setMouseInteraction(BorderMouseActionPtr action) { mouse_interaction = action; }
    void                  resetMouseInteraction() { mouse_interaction.reset(); }
    BorderMouseActionPtr  getMouseInteraction() { return mouse_interaction; }

    void                  convertCropToModelUnits();
    void                  setBorderSize(QSize viewSize);

    eBorderType     borderType;

    qreal           borderWidth;
    QColor          color;
    QColor          color2;
    bool            _useViewSize;

    Mosaic *        parent;

private:
    bool            _requiresConversion;
    QPointF         mousePos;             // used by menu
    bool            debugMouse;

    BorderMouseActionPtr   mouse_interaction;    // used by menu
};

class BorderPlain : public Border
{
public:
    BorderPlain(Mosaic * parent, const Crop &crop, qreal width, QColor color);
    BorderPlain(Mosaic * parent, Circle c,         qreal width, QColor color);
    BorderPlain(Mosaic * parent, QRectF rect,      qreal width, QColor color);
    BorderPlain(Mosaic * parent, QPolygonF poly,   qreal width, QColor color);
    BorderPlain(Mosaic * parent, QSizeF sz,        qreal width, QColor color);

    virtual void createStyleRepresentation() override;
    virtual void draw(GeoGraphics * gg) override;

    void get(qreal & width, QColor & color);

    void legacy_convertToModelUnits() override;

    MapPtr  bmap;
};

class BorderTwoColor : public Border
{
public:
    BorderTwoColor(Mosaic * parent, QSizeF sz,   QColor color1, QColor color2, qreal width, qreal len);
    BorderTwoColor(Mosaic * parent, QRectF rect, QColor color1, QColor color2, qreal width, qreal len);

    virtual void createStyleRepresentation() override;
    virtual void draw(GeoGraphics * gg) override;

    qreal   getLength()             { return segmentLen; }
    void    setLength(qreal length) { segmentLen = length;  resetStyleRepresentation(); }

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
    BorderBlocks(Mosaic * parent, QSizeF sz,   QColor color, int rows, int cols, qreal width);
    BorderBlocks(Mosaic * parent, QRectF rect, QColor color, int rows, int cols, qreal width);

    virtual void createStyleRepresentation() override;
    virtual void draw(GeoGraphics * gg) override;

    void    get(QColor & color1, int & rows, int & cols, qreal & width);
    void    setRows(int rows)   { this->rows = rows; resetStyleRepresentation(); }
    void    setCols(int cols)   { this->cols = cols; resetStyleRepresentation(); }

    void    legacy_convertToModelUnits() override;

protected:

private:
    int         rows;
    int         cols;
    FaceSet     faces;
};
#endif

