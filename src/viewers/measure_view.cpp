#include "viewers/measure_view.h"
#include "settings/configuration.h"
#include "viewers/view_controller.h"
#include "misc/geo_graphics.h"
#include "geometry/vertex.h"
#include <QDebug>

using std::make_shared;

typedef std::shared_ptr<class Vertex>       VertexPtr;

MeasureView::MeasureView(ProtoPtr pp) : Thick(pp)
{
    config  = Sys::config;

    setColor(Qt::red);
    setZValue(MEASURE_ZLEVEL);
}

MeasureView::~MeasureView()
{}

void MeasureView::draw(GeoGraphics * gg)
{
    QPen pen(QColor(0, 128, 0,128),3);
    for (auto && mm : std::as_const(measurements))
    {
        gg->drawLineDirect(mm->startS(), mm->endS(),pen);
        QString msg = QString("%1 (%2)").arg(QString::number(mm->lenS(),'f',2)).arg(QString::number(mm->lenW(),'f',8));
        gg->drawText(mm->endS() + QPointF(10,0),msg);
    }
}

void MeasureView::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    Q_UNUSED(btn);

    if (!view->isActiveLayer(this)) return;

    auto layers =  view->getActiveLayers();
    std::stable_sort(layers.begin(),layers.end(),Layer::sortByZlevelP);
    Layer * measuredLayer = nullptr;
    for (auto layer : layers)
    {
        if (layer == this)
        {
            measuredLayer = layer;
            break;
        }
    }

    Q_ASSERT(measuredLayer);
    auto m = new Measurement(measuredLayer);
    measurements.push_back(m);

    measurement = make_shared<Measure2>(this, spt,m);
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

Measure2::Measure2(Layer * layer, QPointF spt, Measurement *m)
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
