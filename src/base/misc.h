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

#ifndef MISC_H
#define MISC_H

#include <QGraphicsItem>
#include <QPainter>
#include "viewers/GeoGraphics.h"
#include <QtWidgets>

class MarkX : public QGraphicsItem
{
public:
    MarkX(QPointF a, QPen  pen, int index=0);
    MarkX(QPointF a, QPen  pen, QString txt);

    void setHuge() {huge = true;}

    QRectF boundingRect() const Q_DECL_OVERRIDE;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

private:
    bool    huge;
    QPointF _a;
    QPen    _pen;
    int     _index;
    QString _txt;
};

class AQLabel : public QLabel
{
    Q_OBJECT

public:
    AQLabel();

    void keyPressEvent( QKeyEvent *k ) Q_DECL_OVERRIDE;

signals:
    void sig_takeNext();
    void sig_cyclerQuit();
    void sig_view_images();
};

#endif // MISC_H
