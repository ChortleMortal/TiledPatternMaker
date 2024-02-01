#include "misc/border.h"
#include "geometry/crop.h"
#include "geometry/map.h"
#include "qmath.h"
#include "misc/tile_color_defs.h"
#include "misc/geo_graphics.h"
#include "viewers/border_view.h"
#include "viewers/view.h"
#include <QPainter>
#include "geometry/transform.h"

/*
 * A border (defined in model units) is both a a layer, and a crop
 * The crop defines its dimensions and type
 *
 * ViewControl adds a border as a layer
 */

Border::Border()
{
    borderType            = BORDER_NONE;
    _cropType             = CROP_UNDEFINED;
    _useViewSize          = false;
    _requiresConversion   = false;
    _requiresConstruction = true;

    connect(Sys::view, &View::sig_viewSizeChanged, this, &Border::viewResized);
    connect(Sys::view, &View::sig_viewMoved,       this, &Border::viewMoved);

}

Border::Border(CropPtr crop) : Crop(crop)
{
    connect(Sys::view, &View::sig_viewSizeChanged, this, &Border::viewResized);
    connect(Sys::view, &View::sig_viewMoved,       this, &Border::viewMoved);
}

void Border::viewMoved()
{
    if (_useViewSize)
    {
        setBorderSize(Sys::view->getCurrentSize());
        _requiresConstruction = true;
    }
}

void Border::viewResized(QSize oldSize, QSize newSize)
{
    qDebug() << "Border::viewResized" << newSize;
    Q_UNUSED(oldSize);
    Q_UNUSED(newSize);
    viewMoved();
}

void Border::setBorderSize(QSize viewSize)
{
    qDebug() << "Border::setBorderSize" << viewSize;

    switch(_cropType)
    {
    case CROP_RECTANGLE:
    {
        QRectF arect((QPointF()),viewSize.toSizeF());
        auto borderView = BorderView::getInstance();
        QRectF brect =  borderView->screenToWorld(arect);
        setRect(brect);
    }   break;

    case CROP_CIRCLE:
        break;
    case CROP_POLYGON:
    case CROP_UNDEFINED:
        break;
    }
}

void Border::convertCropToModelUnits()
{
    auto borderView = BorderView::getInstance();

    switch (_cropType)
    {
    case CROP_RECTANGLE:
    {
        QRectF rect = getRect();
        rect = borderView->screenToWorld(rect);
        setRect(rect);
    }   break;

    case CROP_CIRCLE:
    {
        Circle c = getCircle();
        c = borderView->screenToWorld(c);
        setCircle(c);
    }   break;

    case CROP_POLYGON:
    {
        auto poly = borderView->screenToWorld(getPolygon());
        setPolygon(poly);
    }   break;

    case CROP_UNDEFINED:
        break;
    }
}

////////////////////////////////////////////////
///
/// BorderPlain
///
////////////////////////////////////////////////

BorderPlain::BorderPlain(QSizeF sz, qreal width, QColor color) : Border()
{
    borderType   = BORDER_PLAIN;
    borderWidth  = width;
    this->color  = color;

    QRectF rect(QPointF(),sz);
    setRect(rect);
}

BorderPlain::BorderPlain(QRectF rect, qreal width, QColor color)
{
    borderType   = BORDER_PLAIN;
    borderWidth  = width;
    this->color  = color;

    setRect(rect);
}

BorderPlain::BorderPlain(Circle c, qreal width, QColor color)
{
    borderType   = BORDER_PLAIN;
    borderWidth  = width;
    this->color  = color;

    setCircle(c);
}

BorderPlain::BorderPlain(QPolygonF p, qreal width, QColor color)
{
    borderType   = BORDER_PLAIN;
    borderWidth  = width;
    this->color  = color;

    setPolygon(p);
}

BorderPlain::BorderPlain(CropPtr crop, qreal width, QColor color) : Border(crop)
{
    // some defaults
    borderType   = BORDER_PLAIN;
    borderWidth  = width;
    this->color  = color;
}

void BorderPlain::construct()
{
    bmap = std::make_shared<Map>("Border");

    QPen pen(color,borderWidth);
    pen.setJoinStyle(Qt::MiterJoin);

    switch (_cropType)
    {
    case CROP_RECTANGLE:
        bmap = std::make_shared<Map>("Border",QPolygonF(getRect()));
        break;

    case CROP_POLYGON:
        bmap = std::make_shared<Map>("Border",getPolygon());
        break;

    case CROP_CIRCLE:
    {
        EdgePoly ep(getCircle());
        bmap = std::make_shared<Map>("Border",ep);
    }   break;

    default:
        qWarning("Not implemented yet");
        break;
    }

    setRequiresConstruction(false);

    Sys::view->update();
}

void  BorderPlain::draw(QPainter * painter, QTransform t)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (getRequiresConstruction())
        construct();

    GeoGraphics gg(painter,t);

    QPen pen(color,borderWidth);
    for (const auto & edge : std::as_const(bmap->getEdges()))
    {
        gg.drawEdge(edge,pen);
    }
}

void BorderPlain::get(qreal & width, QColor & color)
{
    width = borderWidth;
    color = this->color;
}

void BorderPlain::legacy_convertToModelUnits()
{
    convertCropToModelUnits();
    setRequiresConstruction(true);
}

////////////////////////////////////////////////
///
/// BorderTwoColor
/// An outer border with alternating tiles
///
////////////////////////////////////////////////

BorderTwoColor::BorderTwoColor(QSizeF sz, QColor color1, QColor color2, qreal width, qreal len) : Border()
{
    borderType   = BORDER_TWO_COLOR;
    color        = color1;
    this->color2 = color2;
    borderWidth  = width;
    segmentLen   = len;
    QRectF rect(QPointF(),sz);
    setRect(rect);
}

BorderTwoColor::BorderTwoColor(QRectF rect, QColor color1, QColor color2, qreal width, qreal len) : Border()
{
    borderType   = BORDER_TWO_COLOR;
    color        = color1;
    this->color2 = color2;
    borderWidth  = width;
    segmentLen   = len;
    setRect(rect);
}

void BorderTwoColor::construct()
{
    // TODO - the question is: where should the border be?
    //  a) inside the croop (a true border)
    //  b) outside the crop (a frame)
    //  c) aligned with moddle of the crop (straddling it)

    faces.clear();

    qreal x,y;
    qreal bw, bh;

    QPointF tl = getRect().topLeft();
    QPointF tr = getRect().topRight();
    QPointF br = getRect().bottomRight();
    QPointF bl = getRect().bottomLeft();

    // top edge
    x = tl.x();
    y = tl.y();
    while (x < tr.x())
    {
        bw = segmentLen;
        bh = borderWidth;
        if (x + segmentLen > tr.x())
        {
            bw = tr.x() - x;
        }

        addSegment(x,y,bw,bh);
        x += segmentLen;
    }

    // right edge
    x = tr.x();
    y = tr.y() + borderWidth;
    while (y < br.y())
    {
        bw = borderWidth;
        bh = segmentLen;
        if (y + segmentLen > br.y())
        {
            bh = br.y() - y;
        }

        addSegment(x-borderWidth,y,bw,bh);
        y += segmentLen;
    }

    // bottom edge
    x = br.x() - borderWidth;
    y = br.y();
    while (x > bl.x())
    {
        bw = segmentLen;
        bh = borderWidth;
        if (x - segmentLen < bl.x())
        {
            bw = x - bl.x();
        }

        addSegment(x-bw,y-borderWidth,bw,bh);
        x -= segmentLen;
    }

    // left edge - final piece should not overlap
    x = bl.x();
    y = bl.y() - borderWidth;
    while (y > (tl.y() - borderWidth))
    {
        bw = borderWidth;
        bh = segmentLen;
        if (y - segmentLen < tl.y())
        {
            bh = y - tl.y() - borderWidth;
        }

        addSegment(x,y-bh,bw,bh);
        y -= segmentLen;
    }

    setRequiresConstruction(false);

    Sys::view->update();
}

void BorderTwoColor::addSegment(qreal x, qreal y, qreal width, qreal height)
{
    QRectF rect(x,y,width,height);

    EdgePoly ep(rect);
    auto f = std::make_shared<Face>(ep);
    f->color = nextBorderColor();
    faces.push_back(f);
}

void BorderTwoColor::draw(QPainter * painter, QTransform t)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (getRequiresConstruction())
        construct();

    GeoGraphics gg(painter,t);

    for (const auto & face : std::as_const(faces))
    {
        QPen pen(face->color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        gg.fillEdgePoly(*face,pen);
        gg.drawEdgePoly(*face,pen);
    }
}

void BorderTwoColor::get(QColor & color1, QColor & color2, qreal & width, qreal & length)
{
    color1 = this->color;
    color2 = this->color2;
    width  = borderWidth;
    length = segmentLen;
}

QColor BorderTwoColor::nextBorderColor()
{
    static int i = 0;
    if (i==0)
    {
        i = 1;
        return color;
    }
    else
    {
        i = 0;
        return color2;
    }
}

void BorderTwoColor::legacy_convertToModelUnits()
{
    convertCropToModelUnits();

    auto borderView = BorderView::getInstance();
    qreal w = borderView->screenToWorld(borderWidth);
    borderWidth = w;

    setRequiresConstruction(true);
}

////////////////////////////////////////////////
///
/// BorderBlocks
///
////////////////////////////////////////////////

BorderBlocks::BorderBlocks(QSizeF sz, QColor color, int rows, int cols, qreal width) : Border()
{
    borderType  = BORDER_BLOCKS;
    this->color = color;
    this->rows  = rows;
    this->cols  = cols;
    borderWidth = width;

    QRectF rect(QPointF(),sz);
    setRect(rect);
}

BorderBlocks::BorderBlocks(QRectF rect, QColor color, int rows, int cols, qreal width) : Border()
{
    borderType  = BORDER_BLOCKS;
    this->color = color;
    this->rows  = rows;
    this->cols  = cols;
    borderWidth = width;
    setRect(rect);
}

void BorderBlocks::construct()
{
    qDebug() << "BorderBlocks::construct";

    faces.clear();

    qreal side  = borderWidth * qTan(M_PI/8.0);
    qreal piece = sqrt(side*side*0.5);

    // top row
    QPointF start = getRect().topLeft();
    for (int i=0; i < cols; i++)
    {
        // trapezium
        QPolygonF tt;
        tt << (start + QPointF(0.0,               0.0));
        tt << (start + QPointF(side + (piece*2.0),0.0));
        tt << (start + QPointF(piece+side,        piece));
        tt << (start + QPointF(piece,             piece));

        EdgePoly ep(tt);
        auto f = std::make_shared<Face>(ep);
        f->color = color;
        faces.push_back(f);

        start += QPointF(borderWidth,0.0);
    }

    // bottom row
    start = getRect().bottomLeft();
    start -= QPointF(0,piece);
    for (int i=0; i < cols; i++)
    {
        // trapezium
        QPolygonF tt;
        tt << (start + QPointF(piece,             0.0));
        tt << (start + QPointF(piece+side,        0.0));
        tt << (start + QPointF(side + (piece*2.0),piece));
        tt << (start + QPointF(0.0,               piece));

        EdgePoly ep(tt);
        auto f = std::make_shared<Face>(ep);
        f->color = color;
        faces.push_back(f);

        start += QPointF(borderWidth,0.0);
    }

    side    = borderWidth * qTan(M_PI/8.0);
    piece   = sqrt(side*side*0.5);

    // left col
    start = getRect().topLeft();
    for (int i=0; i < rows; i++)
    {
        // trapezium
        QPolygonF tt;
        tt << (start + QPointF(0.0,               0.0));
        tt << (start + QPointF(piece,             piece));
        tt << (start + QPointF(piece,             side + piece));
        tt << (start + QPointF(0,                 (piece*2) + side));

        EdgePoly ep(tt);
        auto f = std::make_shared<Face>(ep);
        f->color = color;
        faces.push_back(f);

        start += QPointF(0.0,borderWidth);
    }

    // right col
    start = getRect().topRight();
    for (int i=0; i < rows; i++)
    {
        // trapezium
        QPolygonF tt;
        tt << (start + QPointF(0.0,               0.0));
        tt << (start + QPointF(0,                 (piece*2) + side));
        tt << (start + QPointF(-piece,            side +  piece));
        tt << (start + QPointF(-piece,            piece));

        EdgePoly ep(tt);
        auto f = std::make_shared<Face>(ep);
        f->color = color;
        faces.push_back(f);

        start += QPointF(0.0,borderWidth);
    }

    setRequiresConstruction(false);

    Sys::view->update();
}

void  BorderBlocks::draw(QPainter * painter, QTransform t)
{
    qDebug() << "BorderBlocks::draw" << Transform::toInfoString(t);

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (getRequiresConstruction())
        construct();

    GeoGraphics gg(painter,t);

    for (const auto & face : std::as_const(faces))
    {
        QPen pen(face->color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        QPen pen2(QColor(TileBlack), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

        gg.fillEdgePoly(*face,pen);
        gg.drawEdgePoly(*face,pen2);
    }
}

void BorderBlocks::get(QColor & color, int & rows, int & cols, qreal & width)
{
    color       = this->color;
    rows        = this->rows;
    cols        = this->cols;
    width       = borderWidth;
}

void BorderBlocks::legacy_convertToModelUnits()
{
    convertCropToModelUnits();

    auto borderView = BorderView::getInstance();
    qreal w = borderView->screenToWorld(borderWidth);
    borderWidth = w;

    setRequiresConstruction(true);
}
