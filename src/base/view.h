﻿/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
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

#ifndef VIEW_H
#define VIEW_H

#include <QtCore>
#include <QtWidgets>

#include "base/configuration.h"

//#define DEBUG_PAINT

class Canvas;
class Cycler;
class ControlPanel;

class View : public QGraphicsView
{
    Q_OBJECT

public:
    static View * getInstance();
    static void  releaseInstance();

    void matchSizeToCanvas();

public slots:
    void slot_sceneRectChanged(const QRectF &rect);

signals:
    void sig_mousePressed(QPointF pos,Qt::MouseButton);
    void sig_mouseDoublePressed(QPointF pos,Qt::MouseButton);
    void sig_mouseDragged(QPointF pos);
    void sig_mouseReleased(QPointF pos);
    void sig_mouseMoved(QPointF pos);
    void keyEvent(QKeyEvent * k);

protected:
    void keyPressEvent( QKeyEvent *k ) Q_DECL_OVERRIDE;
#ifdef DEBUG_PAINT
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
#endif
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent * event) Q_DECL_OVERRIDE;

private:
    View();
    ~View() override;

    static View    * mpThis;
    Canvas         * canvas;
    Configuration  * config;

    bool   dragging;
    bool   expectedResize;
};

#endif // VIEW_H
