#include "mouse_edit_border.h"
#include "geometry/crop.h"
#include "geometry/geo.h"
#include "viewers/border_view.h"

#include <QDebug>

/////////////////////////////////////////////////////////
///
///  Mouse Edit Crop
///
/////////////////////////////////////////////////////////

MouseEditBorder::MouseEditBorder(BorderView * view, QPointF spt, CropPtr crop)
{
    bview        = view;
    start        = nullptr;
    end          = nullptr;
    mcrop        = crop;
    ecMode       = EC_READY;
    ecCorner     = UNDEFINED;

    if (mcrop->getCropType() == CROP_CIRCLE)
    {
        Circle sc = bview->worldToScreen(mcrop->getCircle());
        if (Geo::pointOnCircle(spt,sc,7))
        {
            start = new QPointF(spt);
            ecCircleMode = CM_EDGE;
        }
        else if (Geo::pointInCircle(spt,sc))
        {
            start = new QPointF(spt);
            ecCircleMode = CM_INSIDE;
        }
        else
        {
            ecCircleMode = CM_OUTSIDE;
        }
    }
    else
    {
        start = new QPointF(spt);
    }
}

void MouseEditBorder::draw(QPainter * painter,QPointF mousePos)
{
    qDebug() << "MouseEditBorder::draw";

    mcrop->Crop::draw(painter,bview->getLayerTransform(),true);

    QRectF srect = bview->worldToScreen(mcrop->getRect());
    QPointF pt  = mousePos;
    if (   Geo::isNear(pt,srect.bottomRight())
        || Geo::isNear(pt,srect.topRight())
        || Geo::isNear(pt,srect.bottomLeft())
        || Geo::isNear(pt,srect.topLeft()))
    {
        qreal radius = 8.0;
        painter->setPen(QPen(Qt::blue,1));
        painter->setBrush(Qt::blue);
        painter->drawEllipse(pt, radius, radius);
    }
}

void MouseEditBorder::updateDragging(QPointF spt)
{
    if (!mcrop)
    {
        return;
    }
    if (!start)
    {
        return;
    }

    if (mcrop->getCropType() == CROP_RECTANGLE)
    {
        QRectF srect = bview->worldToScreen(mcrop->getRect());
        switch(ecMode)
        {
        case EC_READY:
            if (Geo::isNear(spt,srect.topLeft()))
                ecCorner = TL;
            else if (Geo::isNear(spt,srect.topRight()))
                ecCorner = TR;
            else if (Geo::isNear(spt,srect.bottomRight()))
                ecCorner = BR;
            else if (Geo::isNear(spt,srect.bottomLeft()))
                ecCorner = BL;
            if (ecCorner != UNDEFINED)
            {
                if (start) delete start;
                start  = new QPointF(spt);
                ecMode = CB_RESIZE;
            }
            else if (Geo::rectContains(srect,spt))
            {
                if (start) delete start;
                start  = new QPointF(spt);
                ecMode = EC_MOVE;
            }
            break;

        case EC_MOVE:
        {
            qDebug() << "border move";
            QPointF delta = spt - *start;
            delete start;
            start  = new QPointF(spt);
            srect.moveTopLeft(srect.topLeft() + delta);
        }
        break;
        case CB_RESIZE:
        {
            QPointF delta = spt - *start;
            delete start;
            start  = new QPointF(spt);

            QSizeF oldSize = srect.size();
            QSizeF newSize;
            switch (ecCorner)
            {
            case TL:    // ok
                newSize = QSizeF(oldSize.width() - delta.x(),oldSize.height() - delta.y());
                srect.setSize(newSize);
                srect.moveTopLeft(spt);
                break;
            case TR:
                newSize = QSizeF(oldSize.width() + delta.x(),oldSize.height() - delta.y());
                srect.setSize(newSize);
                srect.moveTopRight(spt);
                break;
            case BL:
                newSize = QSizeF(oldSize.width() - delta.x(),oldSize.height() + delta.y());
                srect.setSize(newSize);
                srect.moveBottomLeft(spt);
                break;
            case BR:    // ok
                newSize = QSizeF(oldSize.width() + delta.x(),oldSize.height() + delta.y());
                srect.setSize(newSize);
                srect.moveBottomRight(spt);
                break;
            case UNDEFINED:
                break;
            }

            qDebug() << "resize: delta" << delta << "old" << oldSize << "new" << newSize;
            auto mrect = bview->screenToWorld(srect);
            mcrop->setRect(mrect);
        }
        break;
        }
    }
    else if (mcrop->getCropType() == CROP_CIRCLE)
    {
        Circle sc = bview->worldToScreen(mcrop->getCircle());
        if (ecCircleMode == CM_EDGE)
        {
            // first find direction
            qreal s = Geo::dist2(*start,sc.centre);
            qreal m = Geo::dist2(spt,sc.centre);
            bool sub = (s > m);
            // find delta
            qreal delta = Geo::dist(spt,*start);
            delete start;
            start  = new QPointF(spt);
            if (sub)
                delta = -delta;
            qreal radius = sc.radius + delta;
            sc.setRadius(radius);

            Circle mc = bview->screenToWorld(sc);
            mcrop->setCircle(mc);
        }
        else if (ecCircleMode == CM_INSIDE)
        {
            QPointF delta = spt - *start;
            delete start;
            start  = new QPointF(spt);

            QPointF centre = sc.centre + delta;
            sc.setCenter(centre);

            Circle mc = bview->screenToWorld(sc);
            mcrop->setCircle(mc);
        }
    }
}

void MouseEditBorder::endDragging( QPointF spt)
{
    updateDragging(spt);

    qDebug() << "end dragging";

    if (ecMode != EC_READY)
    {
        ecMode = EC_READY;
    }
    // else ignore
}

