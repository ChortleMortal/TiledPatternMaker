#include "mouse_edit_border.h"
#include "geometry/crop.h"
#include "misc/utilities.h"

#include <QDebug>

/////////////////////////////////////////////////////////
///
///  Mouse Edit Crop
///
/////////////////////////////////////////////////////////

MouseEditBorder::MouseEditBorder(QPointF spt, CropPtr crop)
{
    start        = nullptr;
    end          = nullptr;
    this->crop   = crop;
    ecMode       = EC_READY;
    ecCorner     = UNDEFINED;


    if (crop->getCropType() == CROP_CIRCLE)
    {
        Circle & sc = crop->getCircle();
        if (Utils::pointOnCircle(spt,sc,7))
        {
            start = new QPointF(spt);
            ecCircleMode = CM_EDGE;
        }
        else if (Utils::pointInCircle(spt,sc))
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

    crop->Crop::draw(painter,QTransform(),true);

    QRectF rect = crop->getRect();
    QPointF pt  = mousePos;
    if (   Point::isNear(pt,rect.bottomRight())
        || Point::isNear(pt,rect.topRight())
        || Point::isNear(pt,rect.bottomLeft())
        || Point::isNear(pt,rect.topLeft()))
    {
        qreal radius = 8.0;
        painter->setPen(QPen(Qt::blue,1));
        painter->setBrush(Qt::blue);
        painter->drawEllipse(pt, radius, radius);
    }
}

void MouseEditBorder::updateDragging(QPointF spt)
{
    if (!crop)
    {
        return;
    }
    if (!start)
    {
        return;
    }

    if (crop->getCropType() == CROP_RECTANGLE)
    {
        QRectF rect = crop->getRect();
        switch(ecMode)
        {
        case EC_READY:
            if (Point::isNear(spt,rect.topLeft()))
                ecCorner = TL;
            else if (Point::isNear(spt,rect.topRight()))
                ecCorner = TR;
            else if (Point::isNear(spt,rect.bottomRight()))
                ecCorner = BR;
            else if (Point::isNear(spt,rect.bottomLeft()))
                ecCorner = BL;
            if (ecCorner != UNDEFINED)
            {
                if (start) delete start;
                start  = new QPointF(spt);
                ecMode = CB_RESIZE;
            }
            else if (Utils::rectContains(rect,spt))
            {
                if (start) delete start;
                start  = new QPointF(spt);
                ecMode = EC_MOVE;
            }
            break;

        case EC_MOVE:
        {
            qDebug() << "move";
            QPointF delta = spt - *start;
            delete start;
            start  = new QPointF(spt);
            rect.moveTopLeft(rect.topLeft() + delta);
        }
        break;
        case CB_RESIZE:
        {
            QPointF delta = spt - *start;
            delete start;
            start  = new QPointF(spt);

            QSizeF oldSize = rect.size();
            QSizeF newSize;
            switch (ecCorner)
            {
            case TL:    // ok
                newSize = QSizeF(oldSize.width() - delta.x(),oldSize.height() - delta.y());
                rect.setSize(newSize);
                rect.moveTopLeft(spt);
                break;
            case TR:
                newSize = QSizeF(oldSize.width() + delta.x(),oldSize.height() - delta.y());
                rect.setSize(newSize);
                rect.moveTopRight(spt);
                break;
            case BL:
                newSize = QSizeF(oldSize.width() - delta.x(),oldSize.height() + delta.y());
                rect.setSize(newSize);
                rect.moveBottomLeft(spt);
                break;
            case BR:    // ok
                newSize = QSizeF(oldSize.width() + delta.x(),oldSize.height() + delta.y());
                rect.setSize(newSize);
                rect.moveBottomRight(spt);
                break;
            case UNDEFINED:
                break;
            }

            qDebug() << "resize: delta" << delta << "old" << oldSize << "new" << newSize;
        }
        break;
        }
        crop->setRect(rect);
    }
    else if (crop->getCropType() == CROP_CIRCLE)
    {
        Circle & c = crop->getCircle();
        if (ecCircleMode == CM_EDGE)
        {
            // first find direction
            qreal s = Point::dist2(*start,c.centre);
            qreal m = Point::dist2(spt,c.centre);
            bool sub = (s > m);
            // find delta
            qreal delta = Point::dist(spt,*start);
            delete start;
            start  = new QPointF(spt);
            if (sub)
                delta = -delta;
            qreal radius = c.radius + delta;
            c.setRadius(radius);
        }
        else if (ecCircleMode == CM_INSIDE)
        {
            QPointF delta = spt - *start;
            delete start;
            start  = new QPointF(spt);

            QPointF centre = c.centre + delta;
            c.setCenter(centre);
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

