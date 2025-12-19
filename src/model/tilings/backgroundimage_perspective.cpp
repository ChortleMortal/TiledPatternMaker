#include <QImage>
#include <QFileDialog>
#include <QPainter>

#include "gui/model_editors/tiling_edit/tiling_mouseactions.h"
#include "model/tilings/backgroundimage_perspective.h"
#include "gui/viewers/gui_modes.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/vertex.h"


/////////
///
///  BackgroundImagePerspective
///
/////////

BackgroundImagePerspective::BackgroundImagePerspective()
{
    skewMode = false;
}

bool BackgroundImagePerspective::startDragging(QPointF spos)
{
    if (!skewMode)
        return false;

    if (sAccum.size() == 0)
    {
        return addPoint(spos);
    }

    return false;
}

bool BackgroundImagePerspective::addPoint(QPointF spos)
{
    if (!skewMode)
        return false;

    qDebug("Perspective::addPoint");

    VertexPtr vnew = std::make_shared<Vertex>(spos);

    int size = sAccum.size();
    if (size == 0)
    {
        sAccum.push_back(make_shared<Edge>(vnew));
        qDebug() << "point count = 1";
    }
    else if (size == 1)
    {
        EdgePtr last = sAccum.last();
        if (last->getType() == EDGETYPE_POINT)
        {
            last->setV2(vnew);
            qDebug() << "edge count =" << sAccum.size();
        }
        else
        {
            sAccum.push_back(make_shared<Edge>(last->v2,vnew));
            qDebug() << "edge count =" << sAccum.size();
        }
    }
    else if (size == 2)
    {
        EdgePtr last = sAccum.last();
        sAccum.push_back(make_shared<Edge>(last->v2,vnew));
        qDebug() << "edge count = " << sAccum.size();
        sAccum.push_back(make_shared<Edge>(vnew,sAccum.first()->v1));
        qDebug() << "completed with edge count" << sAccum.size();
        return false;
    }
    sLastDrag = QPointF();
    return true;
}

bool BackgroundImagePerspective::updateDragging(QPointF spt)
{
    if (!skewMode)
        return false;

    sLastDrag = spt;
    return true;
}

bool BackgroundImagePerspective::endDragging(QPointF spt)
{
    if (!skewMode)
        return false;

    if (sAccum.size() == 0)
        return false;
    
    if (!Geo::isNear(spt,sAccum.first()->v1->pt))
    {
        addPoint(spt);
    }
    sLastDrag = QPointF();
    return true;
}

void BackgroundImagePerspective::drawPerspective(QPainter * painter)
{
    if (sAccum.size() > 0 && !sLastDrag.isNull())
    {
        // draws line while dragginhg
        QColor drag_color = QColor(206,179,102,230);
        painter->setPen(QPen(drag_color,3));
        painter->setBrush(QBrush(drag_color));
        painter->drawLine(sAccum.last()->v2->pt,sLastDrag);
        painter->drawEllipse(sLastDrag,10,10);
    }
}

#if 0
void Perspective::forceRedraw()
{
    emit sig_updateView();
}
#endif

void BackgroundImagePerspective::setSkewMode(bool enb)
{
    skewMode = enb;
    if (enb)
    {
        sAccum.clear();
        sLastDrag = QPointF();
    }
}

