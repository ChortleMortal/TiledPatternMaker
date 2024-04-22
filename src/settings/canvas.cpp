#include "settings/canvas.h"
#include "misc/sys.h"
#include "settings/configuration.h"
#include "viewers/view.h"

Canvas::Canvas()
{
    _defaultSize  = QSizeF(Sys::DEFAULT_WIDTH,Sys::DEFAULT_HEIGHT);
    _canvasSize   = QSizeF(Sys::DEFAULT_WIDTH,Sys::DEFAULT_HEIGHT);
    _deltaSize    = QSizeF(0,0);
    _bkgdColor    = QColor(Qt::white);

    setModelAlignment(M_ALIGN_TILING);  // default

    computeCanvasTransform();

    _bkgdColor = Qt::white;
}

void Canvas::reInit()
{
    _bounds     = _defaultBounds;
    _canvasSize = _defaultSize;
    _deltaSize  = QSizeF(0,0);

    computeCanvasTransform();
}

void Canvas::initCanvasSize(QSizeF size)
{
    _canvasSize = size;
    _deltaSize  = QSizeF(0,0);
    qDebug() << "Canvas::initCanvasSize" << _canvasSize << "viewSize" << Sys::view->size();
    computeCanvasTransform();
}

void Canvas::setDeltaCanvasSize(QSizeF size)
{
    Q_ASSERT(Sys::config->scaleToView);

    _deltaSize += size;
    qDebug() << "Canvas::setDeltaSize" << size << _canvasSize;
    computeCanvasTransform();
}

/*
    Results:
    VIEW_DESIGN         scale=75 rot=0 (0) trans=750 550
    VIEW_PROTO          scale=75 rot=0 (0) trans=750 550
    VIEW_PROTO_FEATURE  scale=75 rot=0 (0) trans=750 550
    VIEW_DEL            scale=75 rot=0 (0) trans=750 550
    VIEW_FIGURE_MAKER   scale=45 rot=0 (0) trans=450 450
    VIEW_TILING         scale=75 rot=0 (0) trans=750 550
    VIEW_TILIING_MAKER  scale=100 rot=0 (0)trans=500 500
    VIEW_MAP_EDITOR     scale=45 rot=0 (0) trans=450 450
*/

void Canvas::computeCanvasTransform()
{
    if (_canvasSize.isNull())
    {
        _canvasTransform.reset();
        return;
    }

    qreal w      = _canvasSize.width()  + _deltaSize.width();
    qreal h      = _canvasSize.height() + _deltaSize.height();
    qreal aspect = w / h;
    qreal height = _bounds.width / aspect;
    qreal scalex = w /_bounds.width;

    QTransform first  = QTransform().translate(-_bounds.left, - (_bounds.top - height));
    QTransform second = QTransform().scale(scalex,scalex);
    QTransform third  = QTransform().translate(0.0,((w -h)/2.0));
    _canvasTransform  = first * second * third;
    _canvasInverted   = _canvasTransform.inverted();

    _scale = w/_canvasSize.width();
}
