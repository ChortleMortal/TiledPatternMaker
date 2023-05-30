#include "misc/border.h"
#include "geometry/crop.h"
#include "geometry/map.h"
#include "settings/configuration.h"
#include "misc/geo_graphics.h"
#include<QPainter>

typedef std::shared_ptr<const class Map>    constMapPtr;

/*
 * A border (defined in model units) is both a a layer, and a crop
 * The crop defines its dimensions and type
 *..
 * ViewControl adds a border as a layer
 */

Border::Border()
{
    borderType  = BORDER_NONE;
    _cropType   = CROP_UNDEFINED;


    //ViewControl * view = ViewControl::getInstance();
    //connect(view, &View::sig_viewSizeChanged, this, &Border::resize);
}

Border::Border(CropPtr crop) : Crop(crop)
{
    //ViewControl * view = ViewControl::getInstance();
    //connect(view, &View::sig_viewSizeChanged, this, &Border::resize);
}

void Border::resize(QSize oldSize, QSize newSize)
{
#if 0
    switch(_cropType)
    {
    case CROP_RECTANGLE:
    {
        // TODO - verify this works
        QRectF oldRect((QPointF()),QSizeF(oldSize));
        if (_rect == screenToWorld(oldRect))
        {
            // only resize of border is an outide border framing the mosaic
            QRectF arect((QPointF()),QSizeF(newSize));
            _rect = screenToWorld(arect);
            construct();
        }
    }
        break;

    case CROP_CIRCLE:
        break;
    case CROP_POLYGON:
    case CROP_UNDEFINED:
        break;
    }
#else
    Q_UNUSED(newSize);
    Q_UNUSED(oldSize);
#endif
}

////////////////////////////////////////////////
///
/// BorderPlain
///
////////////////////////////////////////////////

BorderPlain::BorderPlain(QSizeF sz, qreal width, QColor color) : Border()
{
    borderType   = BORDER_PLAIN;
    this->width  = width;
    this->color  = color;

    QRectF rect(QPointF(),sz);
    setRect(rect);
}

BorderPlain::BorderPlain(QRectF rect, qreal width, QColor color)
{
    borderType   = BORDER_PLAIN;
    this->width  = width;
    this->color  = color;

    setRect(rect);
}

BorderPlain::BorderPlain(Circle c, qreal width, QColor color)
{
    borderType   = BORDER_PLAIN;
    this->width  = width;
    this->color  = color;

    setCircle(c);
}

BorderPlain::BorderPlain(QPolygonF p, qreal width, QColor color)
{
    borderType   = BORDER_PLAIN;
    this->width  = width;
    this->color  = color;

    setPolygon(p);
}

BorderPlain::BorderPlain(CropPtr crop, qreal width, QColor color) : Border(crop)
{
    // some defaults
    borderType   = BORDER_PLAIN;
    this->width  = width;
    this->color  = color;
}

void BorderPlain::construct()
{
    bmap = std::make_shared<Map>("Border");

    QPen pen(color,width);
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
}

void  BorderPlain::draw(QPainter * painter, QTransform t)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    t = QTransform();   // KLUDGE reset to units
    GeoGraphics gg(painter,t);

    for (const auto & edge : qAsConst(bmap->getEdges()))
    {
        gg.drawEdge(edge,QPen(color,width));
    }
}

void BorderPlain::get(qreal & width, QColor & color)
{
    width = this->width;
    color = this->color;
}

////////////////////////////////////////////////
///
/// BorderTwoColor
/// An outer border with alternating tiles
///
////////////////////////////////////////////////

BorderTwoColor::BorderTwoColor(QSizeF sz, QColor color1, QColor color2, qreal width) : Border()
{
    borderType   = BORDER_TWO_COLOR;
    color        = color1;
    this->color2 = color2;
    this->width  = width;

    QRectF rect(QPointF(),sz);
    setRect(rect);
}

BorderTwoColor::BorderTwoColor(QRectF rect, QColor color1, QColor color2, qreal width) : Border()
{
    borderType   = BORDER_TWO_COLOR;
    color        = color1;
    this->color2 = color2;
    this->width  = width;
    setRect(rect);
}

void BorderTwoColor::construct()
{
    qreal w  = getRect().width();
    qreal h  = getRect().height();

    qreal x = 0.0;
    qreal y = 0.0;

    qreal bw, bh;

    // top
    while (x < w)
    {
        bh = width;
        if (x + LENGTH1 > w)
        {
            bw = w-x;
        }
        else
        {
            bw = LENGTH1;
        }
        QRectF rect(x,y,bw,bh);
        QPolygonF poly(rect);
        EdgePoly ep(poly);
        auto f = std::make_shared<Face>(ep);
        f->color = nextBorderColor();
        faces.push_back(f);
        x+= LENGTH1;
    }

    //right
    y = width;
    x = w - width;
    while (y < h)
    {
        bw = width;
        if (y + LENGTH1 > h)
        {
            bh = h-y;
        }
        else
        {
            bh = LENGTH1;
        }
        QRectF rect(x,y,bw,bh);
        QPolygonF poly(rect);
        EdgePoly ep(poly);
        auto f = std::make_shared<Face>(ep);
        f->color = nextBorderColor();
        faces.push_back(f);
        y += LENGTH1;
    }

    // bottom
    y = h - width;
    x = w - width - LENGTH1 - 1;
    while (x >= 0.0)
    {
        bh = width;
        bw = LENGTH1;
        QRectF rect(x,y,bw,bh);
        QPolygonF poly(rect);
        EdgePoly ep(poly);
        auto f = std::make_shared<Face>(ep);
        f->color = nextBorderColor();
        faces.push_back(f);
        if (x - LENGTH1 < 0.0)
        {
            bw = x;
            QRectF rect(0.0,y,bw,bh);
            QPolygonF poly(rect);
            EdgePoly ep(poly);
            auto f = std::make_shared<Face>(ep);
            f->color = nextBorderColor();
            faces.push_back(f);
        }
        x -= LENGTH1;
    }

    // left
    x = 0.0;
    y = h - width - LENGTH1 -1;
    while (y >= 0.0)
    {
        bw = width;
        bh = LENGTH1;
        QRectF rect(x,y,bw,bh);
        QPolygonF poly(rect);
        EdgePoly ep(poly);
        auto f = std::make_shared<Face>(ep);
        f->color = nextBorderColor();
        faces.push_back(f);
        if (y - LENGTH1 < width)
        {
            bh = y - width;
            QRectF rect(0.0,width,bw,bh);
            QPolygonF poly(rect);
            EdgePoly ep(poly);
            auto f = std::make_shared<Face>(ep);
            f->color = nextBorderColor();
            faces.push_back(f);
        }
        y -= LENGTH1;
    }
}

void BorderTwoColor::draw(QPainter * painter, QTransform t)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    t =  QTransform();      // KLUDGE
    GeoGraphics gg(painter,t);

    for (const auto & face : qAsConst(faces))
    {
        gg.fillEdgePoly(*face,face->color);
        gg.drawEdgePoly(*face,face->color,1);
    }
}

void BorderTwoColor::get(QColor & color1, QColor & color2, qreal & width)
{
    color1 = this->color;
    color2 = this->color2;
    width  = this->width;
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

////////////////////////////////////////////////
///
/// BorderBlocks
///
////////////////////////////////////////////////

BorderBlocks::BorderBlocks(QSizeF sz, QColor color, qreal width, int rows, int cols) : Border()
{
    borderType  = BORDER_BLOCKS;
    this->color = color;
    this->width = width;
    this->rows  = rows;
    this->cols  = cols;

    QRectF rect(QPointF(),sz);
    setRect(rect);
}

BorderBlocks::BorderBlocks(QRectF rect, QColor color, qreal width, int rows, int cols) : Border()
{
    borderType  = BORDER_BLOCKS;
    this->color = color;
    this->width = width;
    this->rows  = rows;
    this->cols  = cols;
    setRect(rect);
}

void BorderBlocks::construct()
{
    qreal w  = getRect().width();
    qreal h  = getRect().height();

    width = w /cols;

    qreal side  = width * qTan(M_PI/8.0);
    qreal piece = sqrt(side*side*0.5);

    // top row
    QPointF start(0.0,0.0);
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

        start += QPointF(width,0.0);
    }

    // bottom row
    start.setX(0.0);
    start.setY(h-piece);
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

        start += QPointF(width,0.0);
    }

    width = h /rows;

    side  = width * qTan(M_PI/8.0);
    piece = sqrt(side*side*0.5);

    // left col
    start= QPointF(0.0,0.0);
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

        start += QPointF(0.0,width);
    }

    // right col
    start = QPointF(w,0);
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

        start += QPointF(0.0,width);
    }
}

void  BorderBlocks::draw(QPainter * painter, QTransform t)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    t = QTransform();   // KLUDGE
    GeoGraphics gg(painter,t);

    for (const auto & face : qAsConst(faces))
    {
        gg.fillEdgePoly(*face,face->color);
        gg.drawEdgePoly(*face,QColor(TileBlack),1);
    }
}

void BorderBlocks::get(QColor & color, qreal & diameter, int & rows, int & cols)
{
    color       = this->color;
    diameter    = this->width;
    rows        = this->rows;
    cols        = this->cols;
}
