#include <QPainter>
#include "gui/viewers/debug_view.h"
#include "gui/viewers/geo_graphics.h"
#include "gui/viewers/motif_maker_view.h"
#include "model/mosaics/reader_base.h"
#include "model/settings/configuration.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling_reader.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/edge_poly.h"
#include "sys/geometry/vertex.h"
#include "sys/sys/fileservices.h"

using std::make_shared;

typedef std::shared_ptr<class Vertex> VertexPtr;

DebugView::DebugView() : LayerController(VIEW_DEBUG,DERIVED,"Debug View")
{
    doTestA = false;
    doTestB = false;

    setZLevel(Sys::config->gridZLevel);

    readConfig();

    setCanMeasure(false);
}

DebugView::~DebugView()
{}

void DebugView::paint(QPainter * painter)
{
    QTransform t;

    if (Sys::config->debugMotifView)
        t = Sys::motifMakerView->getLayerTransform();
    else
        t = getLayerTransform();

    if (doTestA)
    {
        execTestA(painter,t);
    }

    // draw debugMap
    Sys::debugMapCreate->paint(painter,t);
    Sys::debugMapPaint->paint(painter,t);

    // draw mwasurements
    if (canMeasure() && !measurePt.isNull())
    {
        painter->setPen(QPen(Qt::red,3));
        painter->drawEllipse(measurePt.x(), measurePt.y(),3,3);

        qreal sx    = measurePt.x();
        qreal sy    = measurePt.y();
        QPointF mpt = screenToModel(measurePt);
        qreal mx    = mpt.x();
        qreal my    = mpt.y();

        QString msg = QString("(%1,%2)(%3,%4)").arg(QString::number(sx,'f',2),
                                                    QString::number(sy,'f',2),
                                                    QString::number(mx,'f',8),
                                                    QString::number(my,'f',8));
        painter->drawText(measurePt+ + QPointF(10,0),msg);
    }

    if (measurements.size() > 0)
    {
        GeoGraphics gg(painter,getLayerTransform());
        QPen pen(QColor(0, 128, 0,128),3);
        for (auto && mm : std::as_const(measurements))
        {
            gg.drawLineDirect(mm->startS(), mm->endS(),pen);
            QString msg = QString("%1 (%2)").arg(QString::number(mm->lenS(),'f',2)).arg(QString::number(mm->lenW(),'f',8));
            gg.drawText(mm->endS() + QPointF(10,0),msg);
        }
    }

    if (doTestB)
    {
        execTestB(painter,t);
    }
}

void DebugView::readConfig()
{
    uint config     = Sys::config->debugViewConfig;

    _showLines      = (config & DVlines);
    _showArcCentres = (config & DVcentres);
    _showDirection  = (config & DVdirn);
    _showPoints     = (config & DVpoints);
    _showMarks      = (config & DVmarks);
    _showCircles    = (config & DVcircles);
    _showCurves     = (config & DVcurves);
}

void DebugView::writeConfig()
{
    uint config = 0;
    if (_showLines)         config |= DVlines;
    if (_showArcCentres)    config |= DVcentres;
    if (_showDirection)     config |= DVdirn;
    if (_showPoints)        config |= DVpoints;
    if (_showMarks)         config |= DVmarks;
    if (_showCircles)       config |= DVcircles;
    if (_showCurves)        config |= DVcurves;

    Sys::config->debugViewConfig = config;
}

void DebugView::do_testA(bool enb)
{
    doTestA = enb;
    Sys::viewController->slot_updateView();
}

void DebugView::do_testB(bool enb)
{
    doTestB = enb;
    Sys::viewController->slot_updateView();
}

void DebugView::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
{
    Q_UNUSED(btn);

    if (!viewControl()->isEnabled(VIEW_DEBUG))
        return;

    if (!canMeasure())
        return;

    auto layers =  Sys::viewController->getActiveLayers();
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

    measurement = make_shared<DebugViewMeasure>(this, spt,m);
}

void DebugView::slot_mouseDragged(QPointF spt)
{
    if (measurement)
    {
        measurement->updateDragging(spt);
    }
}

void DebugView::slot_mouseReleased(QPointF spt)
{
    if (measurement)
    {
        measurement->endDragging(spt);
    }
    measurement.reset();
}

void DebugView::slot_mouseDoublePressed(QPointF spt)
{
    Q_UNUSED(spt);
}

void DebugView::slot_mouseMoved(QPointF spt)
{
    if (canMeasure())
    {
        measurePt = spt;
        forceRedraw();
    }
}

/////////
///
///  Tests
///
/////////

void DebugView::execTestA(QPainter * painter, QTransform tr)
{
    QPointF p0(0,0);
    QPointF p1(2,0);
    QPointF p2(2,2);
    QPointF p3(0,2);

    Sys::debugMapCreate->insertDebugMark(p0,"p0");
    Sys::debugMapCreate->insertDebugMark(p1,"p1");
    Sys::debugMapCreate->insertDebugMark(p2,"p2");
    Sys::debugMapCreate->insertDebugMark(p3,"p3");

    QPointF q0(1,1);
    //QPointF q1(1,3);
    //QPointF q2(3,3);
    //QPointF q3(3,1);

    //Sys::debugMap->insertDebugMark(q0,"q0");
    //Sys::debugMap->insertDebugMark(q1,"q1");
    //Sys::debugMap->insertDebugMark(q2,"q2");
    //Sys::debugMap->insertDebugMark(q3,"q3");

    QPolygonF poly1;
    poly1 << p0 << p1 << p2 << p3;
    EdgePoly ep1(poly1);

    QPointF centre = q0;
    EdgePtr edge = ep1.get()[0];
    edge->chgangeToCurvedEdge(centre,CURVE_CONVEX);
    edge = ep1.get()[2];
    edge->chgangeToCurvedEdge(centre,CURVE_CONVEX);

    Q_ASSERT(ep1.isClockwise());

#if 0
    QPen pen(Qt::red,2);
    painter->setPen(pen);
    ep1.paint(painter,tr);
#else
    QPainterPath pp = ep1.getPainterPath();
    QPainterPath pp2 = tr.map(pp);

    painter->fillPath(pp2,QBrush(Qt::green));

    QPen pen(Qt::red,2);
    painter->setPen(pen);
    painter->drawPath(pp2);
#endif

    //QPolygonF poly2;
    //poly2 << q0 << q1 << q2 << q3;
    //EdgePoly ep2(poly2);
    //ep2.paint(painter,tr);
}

void DebugView::execTestB(QPainter * painter, QTransform tr)
{
    Q_UNUSED(painter)
    Q_UNUSED(tr)

    QString tilingName = "6 Motif";
    VersionedName vn(tilingName);
    VersionedFile filename = FileServices::getFile(vn,FILE_TILING);
    TilingReader treader(Sys::viewController);
    ReaderBase mrbase;
    auto tp1 = treader.readTilingXML(filename, &mrbase);

    PlacedTiles pts = tp1->unit().getIncluded();
    for (auto & pt : pts)
    {
        auto tile = pt->getTile();
        qDebug() << tile->getBasePoints();
        qDebug() << tile->getPoints();
        qDebug() << pt->getPlacedPoints();
    }

    tilingName = "6 Motif.v1";
    VersionedName vn2(tilingName);
    VersionedFile filename2 = FileServices::getFile(vn,FILE_TILING);
    TilingReader treader2(Sys::viewController);
    ReaderBase mrbase2;
    auto tp2 = treader2.readTilingXML(filename, &mrbase2);

    PlacedTiles pts2 = tp2->unit().getIncluded();
    for (auto & pt : pts2)
    {
        auto tile = pt->getTile();
        qDebug() << tile->getBasePoints();
        qDebug() << tile->getPoints();
        qDebug() << pt->getPlacedPoints();
    }

}

/////////
///
///  Measure
///
/////////

DebugViewMeasure::DebugViewMeasure(Layer * layer, QPointF spt, Measurement *m)
{
    this->m = m;
    this->layer = layer;
    m->setStart(spt);
    m->setEnd(spt);
}

void DebugViewMeasure::updateDragging(QPointF spt)
{
    if (spt != m->startS())
    {
        m->setEnd(spt);
        m->active = true;
        layer->forceRedraw();
    }
}

void DebugViewMeasure::draw(GeoGraphics * g2d)
{
    if (m->active)
    {
        g2d->drawLineDirect(m->startS(),m->endS(),QPen(QColor(206,179,102,230))); // drag color
        QString msg = QString("%1 (%2)").arg(QString::number(m->lenS(),'f',2),QString::number(m->lenW(),'f',8));
        g2d->drawText(m->endS() + QPointF(10,0),msg);
    }
}

void DebugViewMeasure::endDragging(QPointF spt)
{
    if (m->active)
    {
        m->setEnd(spt);
    }
    layer->forceRedraw();
}

QLineF DebugViewMeasure::normalVectorA(QLineF line)
{
    return QLineF(line.p1(), line.p1() + QPointF(line.dy(), -line.dx()));
}

QLineF DebugViewMeasure::normalVectorB(QLineF line)
{
    return QLineF(line.p1(), line.p1() - QPointF(line.dy(), -line.dx()));
}

