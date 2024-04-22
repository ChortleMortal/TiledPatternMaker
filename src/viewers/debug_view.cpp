#include <QPainter>
#include "viewers/debug_view.h"
#include "settings/configuration.h"
#include "geometry/debug_map.h"
#include "geometry/vertex.h"

using std::make_shared;

typedef std::shared_ptr<class Vertex> VertexPtr;

DebugView::DebugView() : LayerController("Debug View",true)
{
    config  = Sys::config;
    setZValue(config->gridZLevel);
    dMap    = make_shared<DebugMap>("Debug View");

    readConfig();
}

DebugView::~DebugView()
{}

void DebugView::clear()
{
    dMap->wipeout();
}

void DebugView::setMap(DebugMapPtr map)
{
    dMap = map;
}

void DebugView::paint(QPainter * painter)
{
    painter->setPen(QPen(Qt::red,2));
    draw(painter);
}

void DebugView::draw(QPainter * painter)
{
    auto t = getLayerTransform();
    dMap->paint(painter,t,_showDirection,_showArcCentres,_showVertices);
}

void DebugView::readConfig()
{
    uint config     = Sys::config->debugViewConfig;

    _viewEnable     = (config & DVenable);
    _showVertices   = (config & DVvertices);
    _showArcCentres = (config & DVcentres);
    _showDirection  = (config & DVdirn);
}

void DebugView::writeConfig()
{
    uint config = 0;
    if (_viewEnable)        config |= DVenable;
    if (_showVertices)      config |= DVvertices;
    if (_showArcCentres)    config |= DVcentres;
    if (_showDirection)     config |= DVdirn;

    Sys::config->debugViewConfig = config;

}
void DebugView::setModelXform(const Xform & xf, bool update)
{
    Q_ASSERT(_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << xf.info() << (isUnique() ? "unique" : "common");
    xf_model = xf;
    forceLayerRecalc(update);
}

const Xform & DebugView::getModelXform()
{
    Q_ASSERT(_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "GET" << getLayerName() << xf_model.info() << (isUnique() ? "unique" : "common");
    return xf_model;
}


void DebugView::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    Q_UNUSED(spt);
    Q_UNUSED(btn);
}

void DebugView::slot_mouseDragged(QPointF spt)
{
    Q_UNUSED(spt);
}

void DebugView::slot_mouseMoved(QPointF spt)
{
    Q_UNUSED(spt);
}

void DebugView::slot_mouseReleased(QPointF spt)
{
    Q_UNUSED(spt);
}

void DebugView::slot_mouseDoublePressed(QPointF spt)
{
    Q_UNUSED(spt);
}

void DebugView::slot_mouseTranslate(QPointF pt)
{
    //if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_GRID) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()) || (view->getKbdMode(KBD_MODE_XFORM_VIEW) && config->lockGridToView))
    {
        Xform xf = getModelXform();
        xf.setTranslateX(xf.getTranslateX() + pt.x());
        xf.setTranslateY(xf.getTranslateY() + pt.y());
        setModelXform(xf,true);
    }
}

void DebugView::slot_wheel_scale(qreal delta)
{
    //if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_GRID) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()) || (view->getKbdMode(KBD_MODE_XFORM_VIEW) && config->lockGridToView))
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setModelXform(xf,true);
    }
}

void DebugView::slot_wheel_rotate(qreal delta)
{
    //if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_GRID) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()) || (view->getKbdMode(KBD_MODE_XFORM_VIEW) && config->lockGridToView))
    {
        Xform xf = getModelXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setModelXform(xf,true);
    }
}

void DebugView::slot_scale(int amount)
{
    //if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_GRID) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()) || (view->getKbdMode(KBD_MODE_XFORM_VIEW) && config->lockGridToView))
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1 + static_cast<qreal>(amount)/100.0));
        setModelXform(xf,true);
    }
}

void DebugView::slot_rotate(int amount)
{
    //if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_GRID) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()) || (view->getKbdMode(KBD_MODE_XFORM_VIEW) && config->lockGridToView))
    {
        Xform xf = getModelXform();
        xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        setModelXform(xf,true);
    }
}

void DebugView::slot_moveX(qreal amount)
{
    //if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_GRID) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()) || (view->getKbdMode(KBD_MODE_XFORM_VIEW) && config->lockGridToView))
    {
        qDebug() << "GridView::slot_moveX" << getLayerName();
        Xform xf = getModelXform();
        xf.setTranslateX(xf.getTranslateX() + amount);
        setModelXform(xf,true);
    }
}

void DebugView::slot_moveY(qreal amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_GRID) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()) || (view->getKbdMode(KBD_MODE_XFORM_VIEW) && config->lockGridToView))
    {
        qDebug().noquote() << "GridView:: slot_moveY" << getLayerName();
        Xform xf = getModelXform();
        xf.setTranslateY(xf.getTranslateY() + amount);
        setModelXform(xf,true);
    }
}

void DebugView::slot_setCenter(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_GRID) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()) || (view->getKbdMode(KBD_MODE_XFORM_VIEW) && config->lockGridToView))
    {
        setCenterScreenUnits(spt);
    }
}
