#include <QPainter>
#include "gui/model_editors/border_edit/mouse_edit_border.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view.h"
#include "gui/top/system_view.h"
#include "gui/viewers/crop_viewer.h"
#include "gui/viewers/geo_graphics.h"
#include "gui/viewers/gui_modes.h"
#include "model/mosaics/border.h"
#include "model/mosaics/mosaic.h"
#include "model/motifs/tile_color_defs.h"
#include "qmath.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/map.h"
#include "sys/geometry/transform.h"

/*
 * A border (defined in model units) is both a a layer, and a crop
 * The crop defines its dimensions and type
 * (Earlier versions used screen units, and need conversion to model units)
 * A border is style, but has no prototype map
 *
 * A border is locked either to the view size or to the mosaic content
 *     both cases uses the transform from first style.
 *     but when locked to view size, content is (or shoulde be) adjusted as the view size changes
 */

Border::Border(Mosaic * parent) : Style (DERIVED,"Border")
{
    this->parent          = parent;
    borderType            = BORDER_NONE;
    created               = false;
    _cropType             = CROP_UNDEFINED;
    _useViewSize          = false;
    _requiresConversion   = false;
    debugMouse            = false;

    setZValue(BORDER_ZLEVEL);

    connect(Sys::sysview, &SystemView::sig_viewSizeChanged, this, &Border::viewResized);
    connect(Sys::sysview, &SystemView::sig_viewMoved,       this, &Border::viewMoved);
}

Border::Border(Mosaic *parent, const Crop & crop) : Style(DERIVED, "Border"), Crop(crop)
{
    this->parent          = parent;
    borderType            = BORDER_NONE;
    created               = false;
    _cropType             = CROP_UNDEFINED;
    _useViewSize          = false;
    _requiresConversion   = false;
    debugMouse            = false;

    setZValue(BORDER_ZLEVEL);

    connect(Sys::sysview, &SystemView::sig_viewSizeChanged, this, &Border::viewResized);
    connect(Sys::sysview, &SystemView::sig_viewMoved,       this, &Border::viewMoved);
}

void Border::setModelXform(const Xform & xf, bool update, uint sigid)
{
    Layer::setModelXform(xf,update,sigid);

    if (_useViewSize)
    {
        setBorderSize(viewControl()->getCanvas().getViewSize());
        resetStyleRepresentation();
    }
}

void Border::paint(QPainter *painter)
{
    Q_ASSERT(getRequiresConversion() == false);
    if (!isCreated())
    {
        createStyleRepresentation();
    }

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QTransform tr = getLayerTransform();
    qDebug().noquote() << "Border::paint" << Transform::info(tr);

    GeoGraphics gg(painter,tr);

    painter->save();
    draw(&gg);
    painter->restore();

    if (getMouseInteraction())
    {
        painter->save();
        getMouseInteraction()->draw(painter,mousePos);
        painter->restore();
    }
}

void Border::viewMoved()
{
    if (_useViewSize)
    {
        setBorderSize(viewControl()->getCanvas().getViewSize());
        resetStyleRepresentation();
    }
}

void Border::viewResized(QSize oldSize, QSize newSize)
{
    qDebug() << "Border::viewResized" << newSize;
    Q_UNUSED(oldSize);
    Q_UNUSED(newSize);
    viewMoved();
}

void Border::setUseViewSize(bool use)
{
    _useViewSize = use;
    if (use)
    {
        resetStyleRepresentation();
    }
}

void Border::setBorderSize(QSize viewSize)
{
    qDebug() << "Border::setBorderSize" << viewSize;

    switch(_cropType)
    {
    case CROP_RECTANGLE:
    {
        QRectF arect((QPointF()),QSizeF(viewSize));
        QRectF brect = screenToModel(arect);
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
    switch (_cropType)
    {
    case CROP_RECTANGLE:
    {
        QRectF rect = getRect();
        rect = screenToModel(rect);
        setRect(rect);
    }   break;

    case CROP_CIRCLE:
    {
        Circle c = getCircle();
        c = screenToModel(c);
        setCircle(c);
    }   break;

    case CROP_POLYGON:
    {
        QPolygonF poly = screenToModel(getAPolygon().get());
        setPolygon(poly);
    }   break;

    case CROP_UNDEFINED:
        break;
    }
}

void Border::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
{
    Q_UNUSED(btn);

    if (!viewControl()->isEnabled(VIEW_MOSAIC))
        return;
    if (Sys::cropViewer->getShowCrop(CM_MOSAIC))
        return;
    if (!Sys::controlPanel->isVisiblePage(PAGE_BORDER_MAKER))
        return;

    setMousePos(spt);

    auto c = dynamic_cast<Crop *>(this);
    auto mec = std::make_shared<MouseEditBorder>(this,mousePos,c);
    setMouseInteraction(mec);
    mec->updateDragging(mousePos);
}

void Border::slot_mouseDragged(QPointF spt)
{
    if (!viewControl()->isEnabled(VIEW_MOSAIC))
        return;

    if (Sys::cropViewer->getShowCrop(CM_MOSAIC))
        return;

    setMousePos(spt);

    if (debugMouse) qDebug().noquote() << "drag" << mousePos << screenToModel(mousePos);
    BorderMouseActionPtr mec = getMouseInteraction();
    if (mec)
    {
        mec->updateDragging(mousePos);
    }

    forceRedraw();
}

void Border::slot_mouseMoved(QPointF spt)
{
    if (!viewControl()->isEnabled(VIEW_MOSAIC))
        return;
    if (Sys::cropViewer->getShowCrop(CM_MOSAIC))
        return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << mousePos;
}

void Border::slot_mouseReleased(QPointF spt)
{
    if (!viewControl()->isEnabled(VIEW_MOSAIC))
        return;
    if (Sys::cropViewer->getShowCrop(CM_MOSAIC))
        return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "release" << mousePos << screenToModel(mousePos);

    BorderMouseActionPtr mma = getMouseInteraction();
    if (mma)
    {
        mma->endDragging(mousePos);
        resetMouseInteraction();
        resetStyleRepresentation();
        forceRedraw();
    }
}

void Border::setMousePos(QPointF pt)
{
    Qt::KeyboardModifiers km = QApplication::keyboardModifiers();
    if (km & Qt::ControlModifier)
    {
        mousePos.setY(pt.y());
    }
    else if (km & Qt::ShiftModifier)
    {
        mousePos.setX(pt.x());
    }
    else
    {
        mousePos = pt;
    }
}

////////////////////////////////////////////////
///
/// BorderPlain
///
////////////////////////////////////////////////

BorderPlain::BorderPlain(Mosaic *parent, QSizeF sz, qreal width, QColor color) : Border(parent)
{
    borderType   = BORDER_PLAIN;
    borderWidth  = width;
    this->color  = color;

    QRectF rect(QPointF(),sz);
    setRect(rect);
}

BorderPlain::BorderPlain(Mosaic * parent, QRectF rect, qreal width, QColor color) : Border(parent)
{
    borderType   = BORDER_PLAIN;
    borderWidth  = width;
    this->color  = color;

    setRect(rect);
}

BorderPlain::BorderPlain(Mosaic * parent, Circle c, qreal width, QColor color) : Border(parent)
{
    borderType   = BORDER_PLAIN;
    borderWidth  = width;
    this->color  = color;

    setCircle(c);
}

BorderPlain::BorderPlain(Mosaic * parent, QPolygonF p, qreal width, QColor color) : Border(parent)
{
    borderType   = BORDER_PLAIN;
    borderWidth  = width;
    this->color  = color;

    setPolygon(p);
}

BorderPlain::BorderPlain(Mosaic *parent, const Crop & crop, qreal width, QColor color) : Border(parent, crop)
{
    // some defaults
    borderType   = BORDER_PLAIN;
    borderWidth  = width;
    this->color  = color;
}

void BorderPlain::createStyleRepresentation()
{
    if (created)
        return;

    bmap = std::make_shared<Map>("Border");

    QPen pen(color,borderWidth);
    pen.setJoinStyle(Qt::MiterJoin);

    switch (_cropType)
    {
    case CROP_RECTANGLE:
        bmap = std::make_shared<Map>("Border",QPolygonF(getRect()));
        break;

    case CROP_POLYGON:
        bmap = std::make_shared<Map>("Border",getAPolygon().get());
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

    created = true;
}

void  BorderPlain::draw(GeoGraphics *gg)
{
    QPen pen(color,borderWidth);
    for (const auto & edge : std::as_const(bmap->getEdges()))
    {
        gg->drawEdge(edge,pen);
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

    resetStyleRepresentation();
}

////////////////////////////////////////////////
///
/// BorderTwoColor
/// An outer border with alternating tiles
///
////////////////////////////////////////////////

BorderTwoColor::BorderTwoColor(Mosaic * parent, QSizeF sz, QColor color1, QColor color2, qreal width, qreal len) : Border(parent)
{
    borderType   = BORDER_TWO_COLOR;
    color        = color1;
    this->color2 = color2;
    borderWidth  = width;
    segmentLen   = len;
    QRectF rect(QPointF(),sz);
    setRect(rect);
}

BorderTwoColor::BorderTwoColor(Mosaic * parent, QRectF rect, QColor color1, QColor color2, qreal width, qreal len) : Border(parent)
{
    borderType   = BORDER_TWO_COLOR;
    color        = color1;
    this->color2 = color2;
    borderWidth  = width;
    segmentLen   = len;
    setRect(rect);
}

void BorderTwoColor::createStyleRepresentation()
{
    // TODO - the question is: where should the border be?
    //  a) inside the croop (a true border)
    //  b) outside the crop (a frame)
    //  c) aligned with moddle of the crop (straddling it)

    if (created)
        return;

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

    created = true;
}

void BorderTwoColor::addSegment(qreal x, qreal y, qreal width, qreal height)
{
    QRectF rect(x,y,width,height);

    EdgePoly ep(rect);
    auto f = std::make_shared<Face>(ep);
    f->color = nextBorderColor();
    faces.push_back(f);
}

void BorderTwoColor::draw(GeoGraphics * gg)
{
    for (const auto & face : std::as_const(faces))
    {
        QPen pen(face->color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        gg->fillEdgePoly(*face,pen);
        gg->drawEdgePoly(*face,pen);
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
    borderWidth = screenToModel(borderWidth);
    segmentLen  = screenToModel(segmentLen);

    convertCropToModelUnits();

    resetStyleRepresentation();
}

////////////////////////////////////////////////
///
/// BorderBlocks
///
////////////////////////////////////////////////

BorderBlocks::BorderBlocks(Mosaic * parent, QSizeF sz, QColor color, int rows, int cols, qreal width) : Border(parent)
{
    borderType  = BORDER_BLOCKS;
    this->color = color;
    this->rows  = rows;
    this->cols  = cols;
    borderWidth = width;

    QRectF rect(QPointF(),sz);
    setRect(rect);
}

BorderBlocks::BorderBlocks(Mosaic *parent, QRectF rect, QColor color, int rows, int cols, qreal width) : Border(parent)
{
    borderType  = BORDER_BLOCKS;
    this->color = color;
    this->rows  = rows;
    this->cols  = cols;
    borderWidth = width;
    setRect(rect);
}

void BorderBlocks::createStyleRepresentation()
{
    qDebug() << "BorderBlocks::construct";

    if (created)
        return;

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

    created = true;
}

void  BorderBlocks::draw(GeoGraphics * gg)
{
    for (const auto & face : std::as_const(faces))
    {
        QPen pen(face->color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        QPen pen2(QColor(TileBlack), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

        gg->fillEdgePoly(*face,pen);
        gg->drawEdgePoly(*face,pen2);
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
    borderWidth = screenToModel(borderWidth);

    convertCropToModelUnits();

    resetStyleRepresentation();
}
