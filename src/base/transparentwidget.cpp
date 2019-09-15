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

#include "transparentwidget.h"
#include <QDebug>

TransparentWidget::TransparentWidget()
{
    // this makes it transparent
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlag(Qt::FramelessWindowHint);
    onTop = true;

    // this gives some border handle to grab on to
    setFrameStyle(QFrame::Box | QFrame::Plain);
    setLineWidth(20);
    setStyleSheet("color: rgb(255,0,0,64);");

    // also
    setAttribute(Qt::WA_DeleteOnClose);

    //grabMouse();
}

TransparentWidget::~TransparentWidget()
{
    //releaseMouse();
}

void TransparentWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        oldPos = event->globalPos();
    }
    else if (event->button() == Qt::RightButton)
    {
        close();    // deletes
    }
}

void TransparentWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint delta = event->globalPos() - oldPos;
    move(x() + delta.x(), y() + delta.y());
    oldPos = event->globalPos();
}


void TransparentWidget::keyPressEvent( QKeyEvent *k )
{
    int key = k->key();
    switch (key)
    {
    case 'Q':
        close();    // deletes
        break;
    case 'T':
        onTop = !onTop;
        setWindowFlag(Qt::WindowStaysOnTopHint,onTop);
        break;
    default:
        break;
    }
}
