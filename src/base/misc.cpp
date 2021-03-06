/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "base/misc.h"
#include "base/layer.h"

MarkX::MarkX(QPointF a, QPen pen, int index) : Layer("MarkX")
{
    _a     = a;
    _pen   = pen;
    _index = index;
    huge   = false;

}

MarkX::MarkX(QPointF a, QPen pen, QString txt) : Layer("MarkX")
{
    _a     = a;
    _pen   = pen;
    _txt   = txt;
    huge   = false;
}

void MarkX::paint(QPainter *painter)
{
    painter->setPen(_pen);

    qreal len   = 5;
    if (huge)
    {
        len = 200;
    }

    painter->drawLine(QPointF(_a.x()-len,_a.y()),QPointF(_a.x()+len,_a.y()));
    painter->drawLine(QPointF(_a.x(),_a.y()-len),QPointF(_a.x(),_a.y()+len));

    qreal width = 120;
    QRectF arect(_a.x()+20,_a.y()-10,width,20);
    if (_txt.isEmpty())
    {
        painter->drawText(arect,QString::number(_index));
    }
    else
    {
        painter->drawText(arect,_txt);
    }
}

void MarkX::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{ Q_UNUSED(spt); Q_UNUSED(btn);}
void MarkX::slot_mouseDragged(QPointF spt)
{ Q_UNUSED(spt)}
void MarkX::slot_mouseTranslate(QPointF pt)
{ Q_UNUSED(pt)}
void MarkX::slot_mouseMoved(QPointF spt)
{ Q_UNUSED(spt)}
void MarkX::slot_mouseReleased(QPointF spt)
{ Q_UNUSED(spt)}
void MarkX::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt)}
void MarkX::slot_wheel_scale(qreal delta)
{ Q_UNUSED(delta);}
void MarkX::slot_wheel_rotate(qreal delta)
{ Q_UNUSED(delta);}
void MarkX::slot_scale(int amount)
{ Q_UNUSED(amount);}
void MarkX::slot_rotate(int amount)
{ Q_UNUSED(amount);}
void MarkX:: slot_moveX(int amount)
{ Q_UNUSED(amount);}
void MarkX::slot_moveY(int amount)
{ Q_UNUSED(amount);}
