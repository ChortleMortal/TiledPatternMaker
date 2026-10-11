#include "model/settings/canvas.h"
#include "sys/geometry/transform.h"
#include "sys/sys.h"

Canvas::Canvas()
{
    init();
}

void Canvas::init()
{
    _viewSize    = QSize(Sys::DEFAULT_WIDTH, Sys::DEFAULT_HEIGHT);   // default
    _canvasSize  = QSize(Sys::DEFAULT_WIDTH, Sys::DEFAULT_HEIGHT);   // default
    _deltaCanvasSize   = QSize(0,0);
    _fillData.init();
    _distortion.init();
    computeCanvasTransform();
}

Canvas::Canvas(const Canvas & other)
{
    _viewSize   = other._viewSize;
    _canvasSize = other._canvasSize;
    _fillData   = other._fillData;
    _distortion = other._distortion;
    _deltaCanvasSize  = other._deltaCanvasSize;
    computeCanvasTransform();
}

Canvas & Canvas::operator=(const Canvas & other)
{
    _viewSize   = other._viewSize;
    _canvasSize = other._canvasSize;
    _fillData   = other._fillData;
    _distortion = other._distortion;
    _deltaCanvasSize  = other._deltaCanvasSize;
    computeCanvasTransform();
    return *this;
}

bool Canvas::operator==(const Canvas & other) const
{
    return  _viewSize   == other._viewSize &&
            _canvasSize == other._canvasSize &&
            _fillData   == other._fillData &&
            _distortion == other._distortion;
}

void Canvas::setDefaultSize()
{
#ifdef VARIABLE_BOUNDS
    _bounds     = _defaultBounds;
#endif
    _canvasSize  = QSize(Sys::DEFAULT_WIDTH, Sys::DEFAULT_HEIGHT);   // default
}

void Canvas::setCanvasSize(QSize size)
{
    //qDebug() << "Canvas::setCanvasSize" << size;
    _canvasSize = size;
    _deltaCanvasSize  = QSize(0,0);
    computeCanvasTransform();
}

void Canvas::setDeltaCanvasSize(QSize size)
{
    //qDebug() << "Canvas::setDeltaCanvasSize"  << size;
    _deltaCanvasSize += size;
    computeCanvasTransform();
}

/*
    Results:
    VIEW_LEGACY         scale=75 rot=0 (0) trans=750 550
    VIEW_PROTO          scale=75 rot=0 (0) trans=750 550
    VIEW_PROTO_FEATURE  scale=75 rot=0 (0) trans=750 550
    VIEW_DEL            scale=75 rot=0 (0) trans=750 550
    VIEW_FIGURE_MAKER   scale=45 rot=0 (0) trans=450 450
    VIEW_TILING         scale=75 rot=0 (0) trans=750 550
    VIEW_TILIING_MAKER  scale=100 rot=0 (0)trans=500 500
    VIEW_MAP_EDITOR     scale=45 rot=0 (0) trans=450 450
*/

/*
 * The canvas transform converts any point in model space into a point in screen (pixel) space,
 * and it's invert converts from screen to model
 * Model space is defined by Bounds which although can be varied, in this application are so far fixed.
 * Model space has a width of 20.0  and it's top left coordinate is defined as (-10.0, 10.0) and from
 * this a rectangle can be defined using an aspecst ratio whuich is the same as the aspect ratrio of
 * the screen view
 */

void Canvas::computeCanvasTransform()
{
    if (_canvasSize.isNull())
    {
        _canvasTransform.reset();
        _undistortedTransform.reset();
        return;
    }

    qreal w      = _canvasSize.width()  + _deltaCanvasSize.width();
    qreal h      = _canvasSize.height() + _deltaCanvasSize.height();
    qreal aspect = w / h;
    qreal height = _bounds.width / aspect;
    qreal scalex = w /_bounds.width;

    QTransform first  = QTransform().translate(-_bounds.left, - (_bounds.top - height));
    QTransform second = QTransform().scale(scalex,scalex);
    QTransform third  = QTransform().translate(0.0,((w -h)/2.0));
    _undistortedTransform = first * second * third;

    if (_distortion.getMode() == DISTORT_CANVAS)
    {
        _canvasTransform  = _distortion.getTransform() * _undistortedTransform;
    }
    else
    {
        _canvasTransform = _undistortedTransform;
    }

    //qDebug() << "Canvas Transform for" << w << "x" << h << "is" << Transform::info(_canvasTransform);
}

QPointF Canvas::getCenter()
{
    qreal w      = _canvasSize.width()  + _deltaCanvasSize.width();
    qreal h      = _canvasSize.height() + _deltaCanvasSize.height();
    return QPointF(w/2.0,h/2.0);
}

QPointF Canvas::getViewCenter()
{
    QRectF r(QPoint(0,0),_viewSize);
    return r.center();
}

void Canvas::dump() const
{
    qDebug() << "Canvas" << this;
    qDebug() << "View size" << _viewSize << "canvas size" << _canvasSize << "delta canvas" << _deltaCanvasSize;;
    qDebug() << "Canvas Transform" << Transform::info(_canvasTransform);
}
