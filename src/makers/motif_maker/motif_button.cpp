#include <QDebug>

#include "makers/motif_maker/motif_button.h"
#include "geometry/map.h"
#include "geometry/vertex.h"
#include "misc/geo_graphics.h"
#include "mosaic/design_element.h"
#include "tile/tile.h"
#include "viewers/viewerbase.h"

////////////////////////////////////////////////////////////////////////////
//
// FeatureButton.java
//
// A tile button is an (optionally clickable) GeoView that displays
// a DesignElement (perhaps DesignElementButton would have been a better
// name).  It draws the underlying tile with the figure overlaid.
// It includes some optimization for drawing radially symmetric figures
// without building the complete map.
//
// These buttons are meant to function like radio buttons -- they live
// in groups, and one is active at any given time.  If a button is active,
// it gets drawn with a red border.
//
// This class is also used to show the large DesignElement being edited
// in the main editing window.  Nice!

QColor MotifButton::tileBorder   = QColor( 140, 140, 160 );
QColor MotifButton::tileIinterior = QColor( 240, 240, 255 );

MotifButton::MotifButton(int index)
{
    construct(nullptr,index);
}

MotifButton::MotifButton(DesignElementPtr dep, int index)
{
    construct(dep,index);
}

MotifButton::~MotifButton()
{
    //qDebug() << "MotifButton destructor";
}

void MotifButton::construct(DesignElementPtr del, int index)
{
    selected            = false;
    this->designElement = del;
    this->index         = index;

    if (del)
    {
        qDebug().noquote() << "MotifButton::construct() index:" << index << "del:" << del.get();
        del->describe();
    }
    else
    {
        qDebug().noquote() << "MotifButton::construct() index:" << index << "del: 0";
    }

    setSize(300,300);
    setStyleSheet("background-color: white;");
    setFrameStyle(QFrame::Box | QFrame::Plain);
    setLineWidth(1);
}

QSizeF MotifButton::getMinimumSize()
{
    return QSizeF(40, 40);
}

void MotifButton::setSize(QSize d )
{
    setFixedSize(d);
    transform = resetViewport(index, designElement.lock(), frameRect());
}

void MotifButton::setSize( int w, int h )
{
    setFixedSize(w,h);
    transform = resetViewport(index, designElement.lock(),frameRect());
}

DesignElementPtr MotifButton::getDesignElement()
{
    return designElement.lock();
}

void MotifButton::setDesignElement(DesignElementPtr del)
{
    if (del == designElement.lock())
        return;

    designElement = del;
    if (del)
    {
        qDebug().noquote() << "MotifButton::setDesignElement() index:" << index << "del:" << del.get();
        del->describe();
    }
    else
    {
        qDebug().noquote() << "MotifButton::setDesignElement() index:" << index << "del:0";
    }

    setViewTransform();
}

void MotifButton::setViewTransform()
{
    transform = resetViewport(index, designElement.lock(),frameRect());
    update();
}

QTransform MotifButton::resetViewport(int index, DesignElementPtr dep, QRect frameRect)
{
    //qDebug() << "reset viewport" << index;

    // Reset the viewport to look at the design element.
    // We can't do this until the Component is mapped and
    // the design element is set.  So we do it really lazily --
    // set a flag to reset the viewport whenever the current
    // DesignElement changes and recompute the viewport from
    // the paint function when the flag is set.

    QTransform transform;

    if(!dep)  return transform;

    MotifPtr fig = dep->getMotif();
    if (!fig) return transform;

    // Get the bounding box of all the figure's vertices and all the
    // tile's vertices.  Then show that region in the viewport.
    // In other words, scale the view to show the DesignElement.

    double xmin = 999.0;
    double xmax = -999.0;
    double ymin = 999.0;
    double ymax = -999.0;

    // This is a cheesy way to string together two streams of points.

    QPolygonF pts = dep->getTile()->getPoints();
    int pidx = 0;
    while ( pidx < pts.size())
    {
        QPointF p = pts[pidx++];
        qreal x = p.x();
        qreal y = p.y();

        xmin = qMin( xmin, x );
        xmax = qMax( xmax, x );
        ymin = qMin( ymin, y );
        ymax = qMax( ymax, y );
    }

#if 0
    MapPtr map = fig->getFigureMap();
    for (auto& vert : map->getVertices())
    {
        QPointF p = vert->getPosition();
        qreal x = p.x();
        qreal y = p.y();

        xmin = qMin( xmin, x );
        xmax = qMax( xmax, x );
        ymin = qMin( ymin, y );
        ymax = qMax( ymax, y );
    }

    map = fig->useDebugMap();
    if (map)
    {
        for (auto& vert : map->getVertices())
        {
            QPointF p = vert->getPosition();
            qreal x = p.x();
            qreal y = p.y();

            xmin = qMin( xmin, x );
            xmax = qMax( xmax, x );
            ymin = qMin( ymin, y );
            ymax = qMax( ymax, y );
        }
    }
#endif
    transform = lookAt(index,QRectF(xmin, ymin, xmax-xmin, ymax-ymin),frameRect);
    //qDebug().noquote() << "MotifButton::resetViewport() index:" << index << "del" << dep.get() << Transform::toInfoString(transform);

    return transform;
}

QTransform MotifButton::resetViewport(int index, MapPtr map, QRect frameRect)
{
    //qDebug() << "reset viewport" << index;

    // Reset the viewport to look at the design element.
    // We can't do this until the Component is mapped and
    // the design element is set.  So we do it really lazily --
    // set a flag to reset the viewport whenever the current
    // DesignElement changes and recompute the viewport from
    // the paint function when the flag is set.

    QTransform transform;

    if(!map)  return transform;

    // Get the bounding box of all the figure's vertices and all the
    // tile's vertices.  Then show that region in the viewport.
    // In other words, scale the view to show the DesignElement.

    double xmin = 999.0;
    double xmax = -999.0;
    double ymin = 999.0;
    double ymax = -999.0;

    // This is a cheesy way to string together two streams of points.

    for (auto & vert : qAsConst(map->getVertices()))
    {
        QPointF p = vert->pt;
        qreal x = p.x();
        qreal y = p.y();

        xmin = qMin( xmin, x );
        xmax = qMax( xmax, x );
        ymin = qMin( ymin, y );
        ymax = qMax( ymax, y );
    }

    transform = lookAt(index,QRectF(xmin, ymin, xmax-xmin, ymax-ymin),frameRect);

    return transform;
}

QTransform MotifButton::lookAt(int index, QRectF rect, QRect frameRect)
{
    // Leave some breathing room around the rectangle to look at.
    // Having the region of interest bleed right to edge of the
    // window doesn't look too good.

    //qDebug() << "lookAt:" << index << rect;

    QSizeF d         = rect.size();
    d *= 1.25;
    QPointF center   = rect.center();
    QRectF paintRect = QRectF(center.x()-(d.width()/2), center.y()-(d.height()/2), d.width(), d.height());

    QTransform t;
    if ( d.width() > 0 && d.height() > 0 )
    {
        t = centerInside(index,paintRect,frameRect);
        //forceRedraw();
    }
    return t;
}

QTransform MotifButton::centerInside(int index, QRectF first, QRectF other)
{
    Q_UNUSED(index);
/*
 * A useful routine when doing things like printing to
 * postscript.  Given two rectangles, find a good transform
 * for centering the first one inside the second one.
 */
    double xscale = other.width() / first.width();
    double yscale = other.height() / first.height();

    double scale = std::min(xscale, yscale);

    QTransform Ts = QTransform().scale(scale, scale);

    QPointF my_center   = first.center();
    QPointF your_center = other.center();

    QTransform your = QTransform::fromTranslate(your_center.x(),your_center.y());
    QTransform my   = QTransform::fromTranslate(-my_center.x(),-my_center.y());
    QTransform res  = my   * Ts * your;
    //qDebug() << "res" << Transform::toInfoString(res);
    return res;
}

void MotifButton::tally(bool select)
{
    if (selected == select)
        return;

    selected = select;

    if (selected)
        setStyleSheet("background-color: rgb(255,240,240);");
    else
        setStyleSheet("background-color: white;");
}

void MotifButton::paintEvent(QPaintEvent * event)
{
    auto del = designElement.lock();
    if (!del)
    {
        QFrame::paintEvent(event);
        return;
    }

    //qDebug() << "MotifButton::paintEvent() index ="  << index;
    QPainter painter(this);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

#ifdef INVERT_VIEW
    // invert
    painter.translate(0,height());
    painter.scale(1.0, -1.0);
#endif

    //qDebug().noquote() << "paint btn:" << index << Transform::toInfoString(transform);
    GeoGraphics gg(&painter, transform);

    ViewerBase::drawTile(&gg,del->getTile(),QBrush(tileIinterior),QPen(tileBorder,3));

    ViewerBase::drawMotif(&gg,del->getMotif(),QPen(Qt::black,3));

#if 0
    QString tempLabel;
    tempLabel = "tile=" + Utils::addr(designElement->getTile().get());
    painter.drawText(5,50,tempLabel);
    tempLabel = "motif=" + Utils::addr(designElement->getMotif().get());
    painter.drawText(5,20,tempLabel);
#endif

    QFrame::paintEvent(event);
}
