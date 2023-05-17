#include "viewers/measure_view.h"
#include "settings/configuration.h"
#include "viewers/viewcontrol.h"
#include "geometry/transform.h"
#include "misc/geo_graphics.h"
#include "geometry/map.h"
#include "geometry/vertex.h"
#include "geometry/point.h"
#include <QDebug>

using std::make_shared;

typedef std::shared_ptr<class Vertex>       VertexPtr;

MeasViewPtr MeasureView::spThis;

MeasViewPtr MeasureView::getSharedInstance()
{
    if (!spThis)
    {
        ProtoPtr pp;
        spThis = make_shared<MeasureView>(pp);
    }
    return spThis;
}

MeasureView::MeasureView(ProtoPtr pp) : Thick(pp)
{
    config  = Configuration::getInstance();
    view    = ViewControl::getInstance();

    setColor(Qt::red);
    setZValue(9);
}

void MeasureView::draw(GeoGraphics * gg)
{
    layerPen.setColor(QColor(  0,128,  0,128));
    for (auto mm : measurements)
    {
        gg->drawLineDirect(mm->startS(), mm->endS(),layerPen);
        QString msg = QString("%1 (%2)").arg(QString::number(mm->lenS(),'f',2)).arg(QString::number(mm->lenW(),'f',8));
        gg->drawText(mm->endS() + QPointF(10,0),msg);
    }
}

void MeasureView::setMeasureMode(bool mode)
{
    measureMode = mode;
    if (!mode)
    {
        measurements.clear();
    }
}

void MeasureView::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    Q_UNUSED(btn);

    if (!view->isActiveLayer(this)) return;

    if (measureMode)
    {
        auto layers =  view->getActiveLayers();
        std::stable_sort(layers.begin(),layers.end(),Layer::sortByZlevel);
        Layer * measuredLayer = nullptr;
        for (auto layer : layers)
        {
            if (layer.get() != this)
            {
                measuredLayer = layer.get();
                break;
            }
        }

        Q_ASSERT(measuredLayer);
        auto m = make_shared<Measurement>(measuredLayer);
        measurements.push_back(m);

        measurement = make_shared<Measure2>(this, spt,m);
    }
}

void MeasureView::slot_mouseDragged(QPointF spt)
{
    if (measurement)
    {
        measurement->updateDragging(spt);
    }
}

void MeasureView::slot_mouseReleased(QPointF spt)
{
    if (measurement)
    {
        measurement->endDragging(spt);
    }
    measurement.reset();
}




/////////
///
///  Measure
///
/////////

Measure2::Measure2(Layer * layer, QPointF spt, MeasurementPtr m)
{
    this->m = m;
    this->layer = layer;
    m->setStart(spt);
    m->setEnd(spt);
}

void Measure2::updateDragging(QPointF spt)
{
    if (spt != m->startS())
    {
        m->setEnd(spt);
        m->active = true;
        layer->forceRedraw();
    }
}

void Measure2::draw(GeoGraphics * g2d)
{
    if (m->active)
    {
        g2d->drawLineDirect(m->startS(),m->endS(),QPen(QColor(206,179,102,230))); // drag color
        QString msg = QString("%1 (%2)").arg(QString::number(m->lenS(),'f',2),QString::number(m->lenW(),'f',8));
        g2d->drawText(m->endS() + QPointF(10,0),msg);
    }
}

void Measure2::endDragging(QPointF spt)
{
    if (m->active)
    {
       m->setEnd(spt);
    }
    layer->forceRedraw();
}

QLineF Measure2::normalVectorA(QLineF line)
{
    return QLineF(line.p1(), line.p1() + QPointF(line.dy(), -line.dx()));
}

QLineF Measure2::normalVectorB(QLineF line)
{
    return QLineF(line.p1(), line.p1() - QPointF(line.dy(), -line.dx()));
}