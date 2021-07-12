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

#include "designs/patterns.h"
#include "designs/shapefactory.h"
#include "base/border.h"
#include "base/utilities.h"
#include "tapp/rosette.h"
#include "tapp/rosette_connect_figure.h"
#include "tapp/star.h"
#include "tapp/infer.h"
#include "tapp/explicit_figure.h"
#include "style/thick.h"
#include "style/interlace.h"
#include "tile/tiling_manager.h"
#include "tile/placed_feature.h"
#include "tile/tiling.h"
#include "geometry/map.h"
#include "tapp/prototype.h"
#include "tapp/design_element.h"
#include "style/thick.h"
#include "tile/feature.h"
#include "settings/configuration.h"

int Pattern::refs = 0;

using std::make_shared;

typedef std::shared_ptr<Thick> ThickPtr;

/////////////////////////////////////////////////////////////
//	  Pattern
/////////////////////////////////////////////////////////////


Pattern::Pattern(qreal Diameter, QBrush Brush, int Row, int Col)
{
    diameter = Diameter;
    radius   = Diameter/2.0;
    Q_UNUSED(Row)
    Q_UNUSED(Col)

    qreal penWidth  = 7.0;
    linePen.setWidthF(penWidth);
    linePen.setColor(TileWhite);
    linePen.setJoinStyle(Qt::MiterJoin);
    linePen.setCapStyle(Qt::RoundCap);

    centerBrush = Brush;

    nopen   = QPen(Qt::NoPen);
    nobrush = QBrush(Qt::NoBrush);

    config    = Configuration::getInstance();

    refs++;
}

Pattern::~Pattern()
{
    //qDebug() << "deleting pattern...";
    refs--;
}


/////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////

Pattern5::Pattern5(qreal Diameter, QBrush Brush)  : Pattern(Diameter, Brush)
{
    layer1 = make_shared<Tile>();
    addLayer(layer1,1);
    layer2 = make_shared<Tile>();
    addLayer(layer2,2);
    layer3 = make_shared<Tile>();
    addLayer(layer3,3);
}

void Pattern5::build()
{
    // this used to generate a background
    ShapeFPtr c1 = make_shared<ShapeFactory>(diameter*1.8);
    c1->addCircle(diameter*1.8,nopen,QBrush(QColor(TileWhite)));
    layer1->addSubLayer(c1);

    ShapeFPtr s = make_shared<ShapeFactory>(diameter);
    s->addInscribedHexagon(linePen,centerBrush,0.0);
    layer2->addSubLayer(s);

    QPolygonF pts;
    s->getHexPackingPoints(pts);
    for (int i = 0; i < 6; i++)
    {
        ShapeFPtr c = make_shared<ShapeFactory>(diameter,pts[i]);
        c->addInscribedHexagon(linePen,QBrush(TileBlack),0.0);
        layer3->addSubLayer(c);
    }
    layer3->slot_rotate(30);
}

/////////////////////////////////////////////////////////////
//	  Pattern 6
/////////////////////////////////////////////////////////////
Pattern6::Pattern6(qreal Diameter, QBrush Brush) : Pattern(Diameter, Brush)

{
    layer1 = make_shared<Tile>();
    addLayer(layer1,1);
    layer2 = make_shared<Tile>();
    addLayer(layer2,2);
}

void Pattern6::build()
{
    ShapeFPtr s = make_shared<ShapeFactory>(diameter,-getLoc());
    layer1->addSubLayer(s);
    s->addInscribedHexagon(QPen(centerBrush.color()),centerBrush,0.0);


    QPolygonF pts;
    s->getHexPackingPoints(pts);
    for (int i = 0; i < 6; i++)
    {

        QTransform t = QTransform::fromTranslate(getLoc().x(),getLoc().y());
        QTransform t2 = t.inverted();
        ShapeFPtr c = make_shared<ShapeFactory>(diameter,t2.map(pts[i]));
        //ShapeFPtr c = make_shared<ShapeFactory>(diameter,pts[i]);
        c->addInscribedHexagon(QPen(TileBlack),QBrush(TileBlack),0.0);
        layer2->addSubLayer(c);
    }
}

/////////////////////////////////////////////////////////////
//	  Pattern 7
/////////////////////////////////////////////////////////////
Pattern7::Pattern7(qreal Diameter, QBrush Brush) : Pattern(Diameter, Brush)
{
    layer1 = make_shared<Tile>();
    addLayer(layer1,1);
}

void Pattern7::build()
{
    static bool two = false;


    ShapeFPtr s = make_shared<ShapeFactory>(diameter,-getLoc());
    layer1->addSubLayer(s);

    doStep(1);
    if (two == false)
    {
        doStep(3);
        two = true;
    }
    else
    {
        doStep(2);
        two = false;
    }
}

bool Pattern7::doStep(int Index)
{
    stepIndex = Index;
    ShapeFPtr s = std::dynamic_pointer_cast<ShapeFactory>(layer1->firstSubLayer());
    Q_ASSERT(s);

    TICK(1)
    {
        s->addCircumscribedOctagon(QPen(Qt::black,1.0),centerBrush,0.0);
    }
    TICK(2)
    {
        s->addInscribedRectangleInOctagon(QPen(TileBlack),QBrush(QColor(TileBlack)),0.0);
    }
    TICK(3)
    {
        s->addInscribedRectangleInOctagon(QPen(TileBlack),QBrush(QColor(TileBlack)),90.0);
    }

    return false;
}

/////////////////////////////////////////////////////////////
//
//	  Pattern 8 - Hu symbol
//
/////////////////////////////////////////////////////////////



PatternHuSymbol::PatternHuSymbol(int gridWidth, QPen GridPen, QPen InnerPen, QColor CanvasColor, qreal Diameter, QBrush Brush)
    : Pattern(Diameter, Brush)
{
    layer1 = make_shared<Tile>();
    addLayer(layer1,1);
    layer3 = make_shared<Tile>();
    addLayer(layer3,3);

    // 21 rows cols
    width = qreal(gridWidth);
    canvasColor = CanvasColor;

    gridPen = GridPen;
    gridPen.setJoinStyle(Qt::MiterJoin);
    gridPen.setCapStyle(Qt::RoundCap);

    innerPen = InnerPen;
    innerPen.setJoinStyle(Qt::MiterJoin);
    innerPen.setCapStyle(Qt::RoundCap);
}

void PatternHuSymbol::build()
{
    s = make_shared<ShapeFactory>(diameter,-getLoc());
    layer1->addSubLayer(s);

    s3 = make_shared<ShapeFactory>(diameter,-getLoc());
    layer3->addSubLayer(s3);

    //canvas->views().at(0)->setRenderHint(QPainter::Antialiasing ,false);
    //canvas->views().at(0)->setRenderHint(QPainter::SmoothPixmapTransform,false);
    s->enableAntialiasing(false);
    s3->enableAntialiasing(false);

    qDebug() << "loc =" << getLoc();
    qDebug() << "sloc=" << s->getLoc();

}

bool PatternHuSymbol::doStep(int Index)
{
    stepIndex = Index;

    TICK(0)
    {
        gridPen.setCapStyle(Qt::RoundCap);

        ShapeFactory c(width*16.0,-getLoc());
        Polygon2 * p = c.addCircumscribedOctagon(gridPen,nobrush,0.0);
        p->setInnerPen(innerPen);
        s->addPolygon(*p);
    }
    TICK (1)
    {
        gridPen.setCapStyle(Qt::SquareCap);
        innerPen.setCapStyle(Qt::SquareCap);

        ShapeFactory c(width*4.0,-getLoc());
        Polygon2 * p = c.addCircumscribedSquare(gridPen,nobrush,0.0);
        p->setInnerPen(innerPen);
        s->addPolygon(*p);
    }
    TICK(2)
    {

        ShapeFactory c(width*12.0,-getLoc());
        Polygon2 * p = c.addCircumscribedSquare(nopen,nobrush,0.0);

        QPolygonF pf = ShapeFactory::getMidPoints(*p);

        QPolygonF line1;
        line1 << pf[0] << pf[2];
        Polyline2 * pl = s->addPolyline(gridPen,line1);
        pl->setInnerPen(innerPen);

        QPolygonF line2;
        line2 << pf[1] << pf[3];
        pl = s->addPolyline(gridPen,line2);
        pl->setInnerPen(innerPen);
    }

    ////////////////////////////
    TICK(3)
    {
        ShapeFactory c(width*2.0);
        Polygon2 * p = c.addCircumscribedSquare(gridPen,nobrush,0.0);
        p->setInnerPen(innerPen);
        p->translate(QPointF(-width,-(width*5.0))) ;
        s->addPolygon(*p);
    }
    TICK(4)
    {
        ShapeFactory c(width*2.0);
        Polygon2 * p = c.addCircumscribedSquare(gridPen,nobrush,0.0);
        p->setInnerPen(innerPen);
        p->translate(QPointF(width,(width*5.0))) ;
        s->addPolygon(*p);
    }
    TICK(5)
    {
        ShapeFactory c(width*2.0);
        Polygon2 * p = c.addCircumscribedSquare(gridPen,nobrush,0.0);
        p->setInnerPen(innerPen);
        p->translate(QPointF((width*5.0),-width)) ;
        s->addPolygon(*p);
    }
    TICK(6)
    {
        ShapeFactory c(width*2.0);
        Polygon2 * p = c.addCircumscribedSquare(gridPen,nobrush,0.0);
        p->translate(QPointF(-(width*5.0),width)) ;
        p->setInnerPen(innerPen);
        s->addPolygon(*p);
    }

    //////////////////////////

    TICK(7)
    {
        gridPen.setCapStyle(Qt::SquareCap);
        innerPen.setCapStyle(Qt::SquareCap);

        QPen pena = gridPen;
        pena.setColor(canvasColor);
        QPen penb = innerPen;
        penb.setColor(canvasColor);
        QPen linePen(QColor(TileGold),2.0);
        linePen.setCapStyle(Qt::SquareCap);

        // canvas filler implemented as polyline
        QPolygonF p;
        p << QPointF(width*3.0,-(width*8.1))
          << QPointF(width*3.0,-(width*4.0));
        Polyline2 *pl = s->addPolyline(nopen,p);
        pl->setInnerPen(pena);

        // now add some yellow line vertical overlays (to clean up edges)
        QPointF a = QPointF((width*2.5)-1.0,-(width*8.5)+1.0);
        QPointF b = QPointF((width*2.5)-1.0,-(width*3.5)-1.0);
        s3->addLine(linePen,QLineF(a,b));

        a = QPointF((width*3.5)+1.0,-(width*8.5)+1.0);
        b = QPointF((width*3.5)+1.0,-(width*3.5)-1.0);
        s3->addLine(linePen,QLineF(a,b));
    }

    TICK(8)
    {
        gridPen.setCapStyle(Qt::SquareCap);
        innerPen.setCapStyle(Qt::SquareCap);

        QPen pena = gridPen;
        pena.setColor(canvasColor);
        QPen penb = innerPen;
        penb.setColor(canvasColor);
        QPen linePen(QColor(TileGold),2.0);
        linePen.setCapStyle(Qt::SquareCap);

        // canvas filler implemented as polyline
        QPolygonF p;
        p << QPointF(-width*3.0,width*8.1)
          << QPointF(-width*3.0,width*4.0);
        Polyline2 *pl = s->addPolyline(nopen,p);
        pl->setInnerPen(pena);

        // now add some yellow line vertical overlays (to clean up edges)
        QPointF a = QPointF((-width*2.5)+1.0,(width*8.5)-1.0);
        QPointF b = QPointF((-width*2.5)+1.0,(width*3.5)+1.0);
        s3->addLine(linePen,QLineF(a,b));

        a = QPointF((-width*3.5)-1.0,(width*8.5)-1.0);
        b = QPointF((-width*3.5)-1.0,(width*3.5)+1.0);
        s3->addLine(linePen,QLineF(a,b));
    }

    TICK(9)
    {
        gridPen.setCapStyle(Qt::SquareCap);
        innerPen.setCapStyle(Qt::SquareCap);

        QPen pena = gridPen;
        pena.setColor(canvasColor);
        QPen penb = innerPen;
        penb.setColor(canvasColor);
        QPen linePen(QColor(TileGold),2.0);
        linePen.setCapStyle(Qt::SquareCap);

        // canvas filler implemented as polyline
        QPolygonF p;
        p << QPointF(-(width*8.1),-width*3.0)
          << QPointF(-(width*4.0),-width*3.0);
        Polyline2 *pl = s->addPolyline(nopen,p);
        pl->setInnerPen(pena);

        // now add some yellow line vertical overlays (to clean up edges)
        QPointF a = QPointF(-(width*8.5)+1.0,-(width*2.5)+1.0);
        QPointF b = QPointF(-(width*3.5)-1.0,-(width*2.5)+1.0);
        s3->addLine(linePen,QLineF(a,b));

        a = QPointF(-(width*8.5)+1.0,-(width*3.5)-1.0);
        b = QPointF(-(width*3.5)-1.0,-(width*3.5)-1.0);
        s3->addLine(linePen,QLineF(a,b));
    }

    TICK(10)
    {
        gridPen.setCapStyle(Qt::SquareCap);
        innerPen.setCapStyle(Qt::SquareCap);

        QPen pena = gridPen;
        pena.setColor(canvasColor);
        QPen penb = innerPen;
        penb.setColor(canvasColor);
        QPen linePen(QColor(TileGold),2.0);
        linePen.setCapStyle(Qt::SquareCap);

        // canvas filler implemented as polyline
        QPolygonF p;
        p << QPointF((width*8.1),width*3.0)
          << QPointF((width*4.0),width*3.0);
        Polyline2 *pl = s->addPolyline(nopen,p);
        pl->setInnerPen(pena);

        // now add some yellow line vertical overlays (to clean up edges)
        QPointF a = QPointF((width*8.5)-1.0,(width*2.5)-1.0);
        QPointF b = QPointF((width*3.5)+1.0,(width*2.5)-1.0);
        s3->addLine(linePen,QLineF(a,b));

        a = QPointF((width*8.5)-1.0,(width*3.5)+1.0);
        b = QPointF((width*3.5)+1.0,(width*3.5)+1.0);
        s3->addLine(linePen,QLineF(a,b));

    }

////////

    TICK(11)
    {
        gridPen.setCapStyle(Qt::SquareCap);

        QPolygonF poly;
        poly << QPointF(0.0      ,-(width*4.0))
             << QPointF(width*2.0,-(width*4.0))
             << QPointF(width*2.0,-(width*8.0));
        Polyline2 * pl = s->addPolyline(gridPen,poly);
        pl->setInnerPen(innerPen);
    }

    TICK(12)
    {
        QPolygonF poly;
        poly << QPointF(0.        ,(width*4.0))
             << QPointF(-width*2.0,(width*4.0))
             << QPointF(-width*2.0,(width*8.0));
        Polyline2 * pl = s->addPolyline(gridPen,poly);
        pl->setInnerPen(innerPen);
    }

    TICK(13)
    {
        QPolygonF poly;
        poly << QPointF(-(width*4.0),0.0)
             << QPointF(-(width*4.0),-width*2.0)
             << QPointF(-(width*8.0),-width*2.0);
        Polyline2 * pl = s->addPolyline(gridPen,poly);
        pl->setInnerPen(innerPen);
    }

    TICK(14)
    {
        QPolygonF poly;
        poly << QPointF(width*4.0,0.0)
             << QPointF(width*4.0,(width*2.0))
             << QPointF(width*8.0,(width*2.0));
        Polyline2 * pl = s->addPolyline(gridPen,poly);
        pl->setInnerPen(innerPen);
    }

    ////////////////

    TICK(15)
    {
        gridPen.setCapStyle(Qt::SquareCap);

        QPolygonF poly;
        poly << QPointF(width*4.0,-(width*4.0))
             << QPointF(width*4.0,-(width*7.0));
        Polyline2 * pl = s->addPolyline(gridPen,poly);
        pl->setInnerPen(innerPen);
    }
    TICK(16)
    {
        QPolygonF poly;
        poly << QPointF(-width*4.0,(width*7.0))
             << QPointF(-width*4.0,(width*4.0));
        Polyline2 * pl = s->addPolyline(gridPen,poly);
        pl->setInnerPen(innerPen);

    }
    TICK(17)
    {
        QPolygonF poly;
        poly << QPointF(-(width*7.0),-width*4.0)
             << QPointF(-(width*4.0),-width*4.0);
        Polyline2 * pl = s->addPolyline(gridPen,poly);
        pl->setInnerPen(innerPen);
    }
    TICK(18)
    {
        QPolygonF poly;
        poly << QPointF(width*7.0,(width*4.0))
             << QPointF(width*4.0,(width*4.0));
        Polyline2 * pl = s->addPolyline(gridPen,poly);
        pl->setInnerPen(innerPen);
    }

    //////////////

    TICK(19)
    {
        Polygon2 * p = s->addCircumscribedOctagon(gridPen,nobrush,0.0);
        p->setInnerPen(innerPen);
    }
    TICK(20)
    {
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////
//
//	  Pattern 9 - Hu symbol with weave
//
/////////////////////////////////////////////////////////////

PatternHuInterlace::PatternHuInterlace(int gridWidth, QPen GridPen, QPen InnerPen, QColor CanvasColor, qreal Diameter, QBrush Brush)
    : Pattern(Diameter, Brush)
{
    layer1 = make_shared<Tile>();
    addLayer(layer1,1);
    layer2 = make_shared<Tile>();
    addLayer(layer2,2);
    layer3 = make_shared<Tile>();
    addLayer(layer3,3);
    layer4 = make_shared<Tile>();
    addLayer(layer4,4);

    // 21 rows cols
    width = qreal(gridWidth);

    canvasColor = CanvasColor;

    gridPen = GridPen;
    gridPen.setJoinStyle(Qt::MiterJoin);
    gridPen.setCapStyle(Qt::RoundCap);

    innerPen = InnerPen;
    innerPen.setJoinStyle(Qt::MiterJoin);
    innerPen.setCapStyle(Qt::RoundCap);
}

void PatternHuInterlace::build()
{
    l1 = make_shared<ShapeFactory>(diameter);
    l1->enableAntialiasing(false);
    layer1->addSubLayer(l1);

    // level 2
    l2 = make_shared<ShapeFactory>(diameter);
    l2->enableAntialiasing(false);
    layer2->addSubLayer(l2);

    // level 3
    l3 = make_shared<ShapeFactory>(diameter);
    l3->enableAntialiasing(false);
    layer3->addSubLayer(l3);

    // level 4
    l4 = make_shared<ShapeFactory>(diameter);
    l4->enableAntialiasing(false);
    layer4->addSubLayer(l4);


}

bool PatternHuInterlace::doStep(int Index)
{
    stepIndex = Index;
    ShapeFPtr s = std::dynamic_pointer_cast<ShapeFactory>(layer1->firstSubLayer());
    Q_ASSERT(s);

    TICK(0)
    {
        // inner octagon
        gridPen.setCapStyle(Qt::RoundCap);

        ShapeFactory c(width*16.0);
        Polygon2 * p = c.addCircumscribedOctagon(gridPen,nobrush,0.0);
        p->setInnerPen(innerPen);
        l1->addPolygon(*p);
    }
    TICK (1)
    {
        // inner square
        gridPen.setCapStyle(Qt::SquareCap);
        innerPen.setCapStyle(Qt::SquareCap);

        ShapeFactory f(width*4.0);
        Polygon2 * p = f.addCircumscribedSquare(gridPen,nobrush,0.0);
        p->setInnerPen(innerPen);
#if 1
        l3->addPolygon(*p);
#else
        // break into 4 pieces
        Polyline2 * a = new Polyline2(gridPen);
        a->setInnerPen(innerPen);
        Polyline2 * b = new Polyline2(gridPen);
        b->setInnerPen(innerPen);
        Polyline2 * c = new Polyline2(gridPen);
        c->setInnerPen(innerPen);
        Polyline2 * d = new Polyline2(gridPen);
        d->setInnerPen(innerPen);

        *a << p->at(0) << p->at(1);
        *b << p->at(1) << p->at(2);
        *c << p->at(2) << p->at(3);
        *d << p->at(3) << p->at(0);
        l1->addPolyline(*a);
        l2->addPolyline(*b);
        l3->addPolyline(*c);
        l4->addPolyline(*d);
#endif
    }
    TICK(2)
    {

        ShapeFactory c(width*12.0);
        Polygon2 * p = c.addCircumscribedSquare(nopen,nobrush,0.0);

        QPolygonF pf = ShapeFactory::getMidPoints(*p);

        // horizontal line
        QPolygonF line1;
        line1 << pf[0] << pf[2];
        Polyline2 * pl = l2->addPolyline(gridPen,line1);
        pl->setInnerPen(innerPen);

        // vertical line
        QPolygonF line2;
        line2 << pf[1] << pf[3];
        pl = l4->addPolyline(gridPen,line2);
        pl->setInnerPen(innerPen);
    }

    ////////////////////////////
    TICK(3)
    {
        ShapeFactory c(width*2.0);
        Polygon2 * p = c.addCircumscribedSquare(gridPen,nobrush,0.0);
        p->setInnerPen(innerPen);
        p->translate( s->getLoc() + QPointF(-width,-(width*5.0))) ;
        l4->addPolygon(*p);
    }
    TICK(4)
    {
        ShapeFactory c(width*2.0);
        Polygon2 * p = c.addCircumscribedSquare(gridPen,nobrush,0.0);
        p->setInnerPen(innerPen);
        p->translate( s->getLoc() + QPointF(width,(width*5.0))) ;
        l4->addPolygon(*p);
    }
    TICK(5)
    {
        ShapeFactory c(width*2.0);
        Polygon2 * p = c.addCircumscribedSquare(gridPen,nobrush,0.0);
        p->setInnerPen(innerPen);
        p->translate( s->getLoc() + QPointF((width*5.0),-width)) ;
        l2->addPolygon(*p);
    }
    TICK(6)
    {
        ShapeFactory c(width*2.0);
        Polygon2 * p = c.addCircumscribedSquare(gridPen,nobrush,0.0);
        p->translate( s->getLoc() + QPointF(-(width*5.0),width)) ;
        p->setInnerPen(innerPen);
        l2->addPolygon(*p);
    }

    //////////////////////////

    TICK(7)
    {
        gridPen.setCapStyle(Qt::SquareCap);
        innerPen.setCapStyle(Qt::SquareCap);

        QPen pena = gridPen;
        pena.setColor(canvasColor);
        QPen penb = innerPen;
        penb.setColor(canvasColor);
        QPen linePen(QColor(TileGold),2.0);
        linePen.setCapStyle(Qt::SquareCap);

        // canvas filler implemented as polyline
        QPolygonF p;
        p << QPointF(width*3.0,-(width*8.1))
          << QPointF(width*3.0,-(width*4.0));
        Polyline2 *pl = s->addPolyline(nopen,p);
        pl->setInnerPen(pena);

        // now add some yellow line vertical overlays (to clean up edges)
        QPointF a = QPointF((width*2.5)-1.0,-(width*8.5)+1.0);// + mapToScene(s->getLoc());
        QPointF b = QPointF((width*2.5)-1.0,-(width*3.5)-1.0);// + mapToScene(s->getLoc());
        l3->addLine(linePen,QLineF(a,b));

        a = QPointF((width*3.5)+1.0,-(width*8.5)+1.0);// + mapToScene(s->getLoc());
        b = QPointF((width*3.5)+1.0,-(width*3.5)-1.0);// + mapToScene(s->getLoc());
        l3->addLine(linePen,QLineF(a,b));
    }

    TICK(8)
    {
        gridPen.setCapStyle(Qt::SquareCap);
        innerPen.setCapStyle(Qt::SquareCap);

        QPen pena = gridPen;
        pena.setColor(canvasColor);
        QPen penb = innerPen;
        penb.setColor(canvasColor);
        QPen linePen(QColor(TileGold),2.0);
        linePen.setCapStyle(Qt::SquareCap);

        // canvas filler implemented as polyline
        QPolygonF p;
        p << QPointF(-width*3.0,width*8.1)
          << QPointF(-width*3.0,width*4.0);
        Polyline2 *pl = s->addPolyline(nopen,p);
        pl->setInnerPen(pena);

        // now add some yellow line vertical overlays (to clean up edges)
        QPointF a = QPointF((-width*2.5)+1.0,(width*8.5)-1.0);// + mapToScene(s->getLoc());
        QPointF b = QPointF((-width*2.5)+1.0,(width*3.5)+1.0);// + mapToScene(s->getLoc());
        l3->addLine(linePen,QLineF(a,b));

        a = QPointF((-width*3.5)-1.0,(width*8.5)-1.0);// + mapToScene(s->getLoc());
        b = QPointF((-width*3.5)-1.0,(width*3.5)+1.0);// + mapToScene(s->getLoc());
        l3->addLine(linePen,QLineF(a,b));
    }

    TICK(9)
    {
        gridPen.setCapStyle(Qt::SquareCap);
        innerPen.setCapStyle(Qt::SquareCap);

        QPen pena = gridPen;
        pena.setColor(canvasColor);
        QPen penb = innerPen;
        penb.setColor(canvasColor);
        QPen linePen(QColor(TileGold),2.0);
        linePen.setCapStyle(Qt::SquareCap);

        // canvas filler implemented as polyline
        QPolygonF p;
        p << QPointF(-(width*8.1),-width*3.0)
          << QPointF(-(width*4.0),-width*3.0);
        Polyline2 *pl = s->addPolyline(nopen,p);
        pl->setInnerPen(pena);

        // now add some yellow line vertical overlays (to clean up edges)
        QPointF a = QPointF(-(width*8.5)+1.0,-(width*2.5)+1.0);// + mapToScene(s->getLoc());
        QPointF b = QPointF(-(width*3.5)-1.0,-(width*2.5)+1.0);// + mapToScene(s->getLoc());
        l3->addLine(linePen,QLineF(a,b));

        a = QPointF(-(width*8.5)+1.0,-(width*3.5)-1.0);// + mapToScene(s->getLoc());
        b = QPointF(-(width*3.5)-1.0,-(width*3.5)-1.0);// + mapToScene(s->getLoc());
        l3->addLine(linePen,QLineF(a,b));
    }

    TICK(10)
    {
        gridPen.setCapStyle(Qt::SquareCap);
        innerPen.setCapStyle(Qt::SquareCap);

        QPen pena = gridPen;
        pena.setColor(canvasColor);
        QPen penb = innerPen;
        penb.setColor(canvasColor);
        QPen linePen(QColor(TileGold),2.0);
        linePen.setCapStyle(Qt::SquareCap);

        // canvas filler implemented as polyline
        QPolygonF p;
        p << QPointF((width*8.1),width*3.0)
          << QPointF((width*4.0),width*3.0);
        Polyline2 *pl = s->addPolyline(nopen,p);
        pl->setInnerPen(pena);

        // now add some yellow line vertical overlays (to clean up edges)
        QPointF a = QPointF((width*8.5)-1.0,(width*2.5)-1.0);// + mapToScene(s->getLoc());
        QPointF b = QPointF((width*3.5)+1.0,(width*2.5)-1.0);// + mapToScene(s->getLoc());
        l3->addLine(linePen,QLineF(a,b));

        a = QPointF((width*8.5)-1.0,(width*3.5)+1.0);// + mapToScene(s->getLoc());
        b = QPointF((width*3.5)+1.0,(width*3.5)+1.0);// + mapToScene(s->getLoc());
        l3->addLine(linePen,QLineF(a,b));
    }

////////

    TICK(11)
    {
        gridPen.setCapStyle(Qt::SquareCap);

        QPolygonF poly;
        poly << QPointF(0.0      ,-(width*4.0))
             << QPointF(width*2.0,-(width*4.0))
             << QPointF(width*2.0,-(width*8.0));
        Polyline2 * pl = s->addPolyline(gridPen,poly);
        pl->setInnerPen(innerPen);
    }

    TICK(12)
    {
        QPolygonF poly;
        poly << QPointF(0.        ,(width*4.0))
             << QPointF(-width*2.0,(width*4.0))
             << QPointF(-width*2.0,(width*8.0));
        Polyline2 * pl = s->addPolyline(gridPen,poly);
        pl->setInnerPen(innerPen);
    }

    TICK(13)
    {
        QPolygonF poly;
        poly << QPointF(-(width*4.0),0.0)
             << QPointF(-(width*4.0),-width*2.0)
             << QPointF(-(width*8.0),-width*2.0);
        Polyline2 * pl = s->addPolyline(gridPen,poly);
        pl->setInnerPen(innerPen);
    }

    TICK(14)
    {
        QPolygonF poly;
        poly << QPointF(width*4.0,0.0)
             << QPointF(width*4.0,(width*2.0))
             << QPointF(width*8.0,(width*2.0));
        Polyline2 * pl = s->addPolyline(gridPen,poly);
        pl->setInnerPen(innerPen);
    }

    ////////////////

    TICK(15)
    {
        gridPen.setCapStyle(Qt::SquareCap);

        QPolygonF poly;
        poly << QPointF(width*4.0,-(width*4.0))
             << QPointF(width*4.0,-(width*7.0));
        Polyline2 * pl = s->addPolyline(gridPen,poly);
        pl->setInnerPen(innerPen);
    }
    TICK(16)
    {
        QPolygonF poly;
        poly << QPointF(-width*4.0,(width*7.0))
             << QPointF(-width*4.0,(width*4.0));
        Polyline2 * pl = s->addPolyline(gridPen,poly);
        pl->setInnerPen(innerPen);

    }
    TICK(17)
    {
        QPolygonF poly;
        poly << QPointF(-(width*7.0),-width*4.0)
             << QPointF(-(width*4.0),-width*4.0);
        Polyline2 * pl = s->addPolyline(gridPen,poly);
        pl->setInnerPen(innerPen);
    }
    TICK(18)
    {
        QPolygonF poly;
        poly << QPointF(width*7.0,(width*4.0))
             << QPointF(width*4.0,(width*4.0));
        Polyline2 * pl = s->addPolyline(gridPen,poly);
        pl->setInnerPen(innerPen);
    }

    //////////////

    TICK(19)
    {
        Polygon2 * p = s->addCircumscribedOctagon(gridPen,nobrush,0.0);
        p->setInnerPen(innerPen);
    }
    TICK(20)
    {
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////
//	  Pattern 10
/////////////////////////////////////////////////////////////

Pattern10::Pattern10(qreal Diameter, QBrush Brush) : Pattern(Diameter, Brush)
{
    layer1 = make_shared<Tile>();
    addLayer(layer1,1);
}

void Pattern10::build()
{
    ShapeFPtr s = make_shared<ShapeFactory>(diameter,-getLoc());
    layer1->addSubLayer(s);

    doStep(0);
    //step(1);
    //step(2);
    //step(3);
}

bool Pattern10::doStep(int Index)
{
    stepIndex = Index;
    ShapeFPtr s = std::dynamic_pointer_cast<ShapeFactory>(layer1->firstSubLayer());
    Q_ASSERT(s);

    TICK(0)
    {
        s->addInscribedEnneagram(linePen,nobrush,0);
    }
    TICK(1)
    {
        s->addInscribedNonagon(linePen, nobrush,0.0);
    }
    TICK(2)
    {
        s->addInscribedPentagon(linePen,nobrush,0);
    }
    TICK(3)
    {
        s->addInscribedHepatgon(linePen,nobrush,0);
    }

    return false;
}



/////////////////////////////////////////////////////////////
//	  Pattern 11 - Alhambra
/////////////////////////////////////////////////////////////
Pattern11::Pattern11(qreal Diameter, QBrush Brush, qreal rotation, eDirection direction)  : Pattern(Diameter, Brush)
{
    layer1 = make_shared<Tile>();
    addLayer(layer1,1);
    layer2 = make_shared<Tile>();
    addLayer(layer2,2);

    Rotation  = rotation;
    Direction = direction;
}

void Pattern11::build()
{
    // the hexagon
    ShapeFPtr s = make_shared<ShapeFactory>(diameter);
    layer1->addSubLayer(s);
    Polygon2 * p = s->addInscribedHexagon(nopen,centerBrush,Rotation);

    // calc midpoints
    Polygon2 p3 = ShapeFactory::getMidPoints(*p);

    QBrush b1(Qt::black);
    QBrush b2(Qt::white);
    QPen   p1(Qt::black,1);
    QPen   p2(Qt::white,1);

    qreal radius = diameter/2.0;

    QVector<QPointF> circleCenters;
    for (int i=0; i< 6; i++)
    {
        QPointF qpf = p->at(i);
        circleCenters << QLineF(QPointF(0,0),qpf).pointAt(0.5);
    }

    QVector<qreal> angles;
    if (Rotation == 90.0)
    {
        angles << 150.0 <<  90.0 <<  30.0 << -30.0 << -90.0 << -150.0;
    }
    else
    {
        angles << 240.0 <<  180.0 <<  120.0 << 60.0 << 0.0  << -60.0;
    }

    QRectF  arect(QPointF(),QSizeF(radius,radius));

    for (int i=0; i < 6; i++)
    {
        ShapeFPtr s2 = make_shared<ShapeFactory>(diameter);
        layer2->addSubLayer(s2);

        QPainterPath & pp = s2->getPPath();
        s2->setPPath((i & 1) ? p1 : p2,(i & 1) ? b1 : b2);

        int iPrev = i - 1;
        if (iPrev < 0) iPrev = 5;

        int iPrev2 = iPrev - 1;
        if (iPrev2 < 0) iPrev2 = 5;

        qreal span = (Direction == CCW) ? 120.0 : -120.0;

        // first arc
        arect.moveCenter(circleCenters[i]);
        pp.moveTo(circleCenters[i]);
        pp.arcMoveTo(arect,angles[i]);
        pp.arcTo(arect,angles[i],span);

        // sidelines
        if (Direction == CCW)
        {
            pp.lineTo(p->at(i));
            pp.lineTo(p3.at(iPrev));
        }
        else
        {
            pp.lineTo(p->at(iPrev));
            pp.lineTo(p3.at(iPrev2));
        }

        // second arc
        arect.moveCenter(circleCenters[iPrev]);
        pp.moveTo(circleCenters[iPrev]);
        pp.arcMoveTo(arect,angles[iPrev] + span);
        pp.arcTo(arect,angles[iPrev] + span, -span);
    }
}

/////////////////////////////////////////////////////////////
//	  Pattern 12 - Alhambra 2
/////////////////////////////////////////////////////////////
Pattern12::Pattern12(qreal Diameter, QBrush Brush, qreal rotation, eDirection direction, int Row, int Col) : Pattern(Diameter, Brush, Row, Col)
{
    layer1 = make_shared<Tile>();
    addLayer(layer1,1);
    layer2 = make_shared<Tile>();
    addLayer(layer2,2);
    layer3 = make_shared<Tile>();
    addLayer(layer3,3);

    Rotation  = rotation;
    Direction = direction;
}

void Pattern12::build()
{
    // the hexagon
    ShapeFPtr s = make_shared<ShapeFactory>(diameter);
    layer1->addSubLayer(s);
  //hexagon = s->addInscribedHexagon(QPen(QColor(Qt::red),3),Brush,rotation);
    hexagon = s->addInscribedHexagon(nopen,centerBrush,Rotation);

    // calc midpoints
    Polygon2 midpoints = ShapeFactory::getMidPoints(*hexagon);

    QBrush bColor;
    QBrush bGreen(AlhambraGreen);
    QBrush bWhite(TileWhite);
    QBrush bBlack(TileBlack);
    QBrush bBrown(AlhambraBrown);
    QBrush bBlue(AlhambraBlue);
    QBrush bGold(AlhambraGold);

    QPen   pColor;
    QPen   pGreen(QColor(AlhambraGreen),1);
    QPen   pWhite(QColor(TileWhite),1);
    QPen   pBlack(QColor(TileBlack),1);
    QPen   pBrown(QColor(AlhambraBrown),1);
    QPen   pBlue(QColor(AlhambraBlue)   ,1);
    QPen   pGold(QColor(AlhambraGold),1);

    QPen   pens[]    = {pBlue, pBrown, pBlack, pGreen};
    QBrush brushes[] = {bBlue, bBrown, bBlack, bGreen};

    qreal radius = diameter/2.0;

    QVector<QPointF> circleCenters;

    for (int i=0; i< 6; i++)
    {
        QPointF qpf = hexagon->at(i);
        circleCenters << QLineF(QPointF(0,0),qpf).pointAt(0.5);
    }

    QVector<qreal> angles;
    if (Rotation == 90.0)
    {
        angles << 150.0 <<  90.0 <<  30.0 << -30.0 << -90.0 << -150.0;
    }
    else
    {
        angles << 240.0 <<  180.0 <<  120.0 << 60.0 << 0.0  << -60.0;
    }

    QRectF  arect(QPointF(),QSizeF(radius,radius));

    // select colors for the printer path
    int posn = (row / 2) + col;
    if (row & 1) posn++;

    for (int i=0; i < 6; i++)
    {
        ShapeFPtr s2 = make_shared<ShapeFactory>(diameter);
        layer2->addSubLayer(s2);

        // this steps the colors diagonaly
        QPainterPath & pp = s2->getPPath();
        pColor = pens[posn % 4];
        bColor = brushes[posn % 4];
        if (i==1)
        {
            pColor = pens[(posn+1) % 4];
            bColor = brushes[(posn+1) % 4];
        }

        s2->setPPath((i & 1) ? pColor : pWhite,(i & 1) ? bColor : bWhite);

        // placement for ppath segments
        int iPrev = i - 1;
        if (iPrev < 0) iPrev = 5;

        int iPrev2 = iPrev - 1;
        if (iPrev2 < 0) iPrev2 = 5;

        qreal span = (Direction == CCW) ? 120.0 : -120.0;

        // first arc
        arect.moveCenter(circleCenters[i]);
        pp.moveTo(circleCenters[i]);
        pp.arcMoveTo(arect,angles[i]);
        pp.arcTo(arect,angles[i],span);

        // sidelines
        if (Direction == CCW)
        {
            pp.lineTo(hexagon->at(i));
            pp.lineTo(midpoints.at(iPrev));
        }
        else
        {
            pp.lineTo(hexagon->at(iPrev));
            pp.lineTo(midpoints.at(iPrev2));
        }

        // second arc
        arect.moveCenter(circleCenters[iPrev]);
        pp.moveTo(circleCenters[iPrev]);
        pp.arcMoveTo(arect,angles[iPrev] + span);
        pp.arcTo(arect,angles[iPrev] + span, -span);
    }

    ShapeFPtr s3;

    // add hexagons in colors
    s3 = make_shared<ShapeFactory>(diameter/2.9, hexagon->at(0));
    layer3->addSubLayer(s3);
    s3->addInscribedHexagon(pWhite,bWhite,0.0);

    s3 = make_shared<ShapeFactory>(diameter/2.9, hexagon->at(4));
    layer3->addSubLayer(s3);
    s3->addInscribedHexagon(pWhite,bWhite,0.0);

    // add stars in colors
    s3 = make_shared<ShapeFactory>(diameter/2.9, hexagon->at(3));
    layer3->addSubLayer(s3);
    s3->add6ptStar(pGold,bGold,90.0);

    s3 = make_shared<ShapeFactory>(diameter/2.9, hexagon->at(5));
    layer3->addSubLayer(s3);
    s3->add6ptStar(pGold,bGold,90.0);
}

/////////////////////////////////////////////////////////////
//	  Pattern 13 - The once I could not complete
/////////////////////////////////////////////////////////////

PatternIncompleteA::PatternIncompleteA(qreal Diameter, QBrush Brush) : Pattern(Diameter, Brush)
{
    layer1 = make_shared<Tile>();
    addLayer(layer1,1);
    layer2 = make_shared<Tile>();
    addLayer(layer2,2);
    layer3 = make_shared<Tile>();
    addLayer(layer3,3);
    layer4 = make_shared<Tile>();
    addLayer(layer4,4);
}

void PatternIncompleteA::build()
{
    // brushes & pens
    QBrush bBlack(Qt::black);
    //QPen   rPen = QPen(QColor(Qt::red),1);

    linePen.setJoinStyle(Qt::MiterJoin);
    linePen.setCapStyle(Qt::RoundCap);

    // outlines for debug
    ShapeFPtr s4 = make_shared<ShapeFactory>(diameter);
    layer4->addSubLayer(s4);
    //layer4->setFlag(ItemClipsChildrenToShape);
    // defining (invisible) square
    //Polygon2 *  p = s4->addCircumscribedSquare(rPen,nobrush);
    Polygon2 *  p = s4->addCircumscribedSquare(nopen,nobrush);  // invisible
    // debug outlines (red)
    //s4->addCircle(diameter,QPen(QColor(Qt::red)));

    // main design
    ShapeFPtr s1 = make_shared<ShapeFactory>(diameter);
    layer1->addSubLayer(s1);
    //layer1->setFlag(ItemClipsChildrenToShape);

    qreal radius = diameter/2.0;
    QPointF a;
    QPointF b;
    QPointF c;
    QPointF d;

    QGraphicsEllipseItem i(-radius,-radius,diameter,diameter);

    //QLineF line2(p->at(0),p->at(2));
    //int points = Utils::circleLineIntersectionPoints(i,radius,line2,a,b);

    QLineF line3(p->at(1),p->at(3));
    int points = Utils::circleLineIntersectionPoints(i,radius,line3,c,d);

    Q_UNUSED(points)

    QPointF left(-radius,0);
    QPointF right(radius,0);
    QPointF top(0,-radius);
    QPointF bot(0,radius);

    QLineF aline;

    aline.setPoints(right,a);
    aline.setLength(aline.length() * 1.5);
    s1->addLine(linePen,aline);
    //s2->addLine(QPen(QColor(Qt::red),1),aline);

    aline.setPoints(left,b);
    aline.setLength(aline.length() * 1.5);
    s1->addLine(linePen,aline);
    //s2->addLine(QPen(QColor(Qt::red),1),aline);

    aline.setPoints(right,c);
    aline.setLength(aline.length() * 1.5);
    s1->addLine(linePen,aline);
    //s2->addLine(QPen(QColor(Qt::red),1),aline);

    aline.setPoints(left,d);
    aline.setLength(aline.length() * 1.5);
    s1->addLine(linePen,aline);
    //s2->addLine(QPen(QColor(Qt::red),1),aline);

    aline.setPoints(top,a);
    aline.setLength(aline.length() * 1.5);
    s1->addLine(linePen,aline);
    //s2->addLine(QPen(QColor(Qt::red),1),aline);

    aline.setPoints(bot,b);
    aline.setLength(aline.length() * 1.5);
    s1->addLine(linePen,aline);
    //s2->addLine(QPen(QColor(Qt::red),1),aline);

    aline.setPoints(bot,c);
    aline.setLength(aline.length() * 1.5);
    s1->addLine(linePen,aline);
    //s2->addLine(QPen(QColor(Qt::red),1),aline);

    aline.setPoints(top,d);
    aline.setLength(aline.length() * 1.5);
    s1->addLine(linePen,aline);
    //s2->addLine(QPen(QColor(Qt::red),1),aline);

#if 0
    // marks for a,b,c,d
    MarkX  * m;
    m = new MarkX(a,QPen(QColor(Qt::green),3),"a");
    layers[4].addToGroup(m);

    m = new MarkX(b,QPen(QColor(Qt::green),3),"b");
    layers[4].addToGroup(m);

    m = new MarkX(c,QPen(QColor(Qt::green),3),"c");
    layers[4].addToGroup(m);

    m = new MarkX(d,QPen(QColor(Qt::green),3),"d");
    layers[4].addToGroup(m);
#endif

    // the eraser
    ShapeFPtr s2 = make_shared<ShapeFactory>(diameter/2.6);  // need to do the math 2.6
    layer2->addSubLayer(s2);
    //layers[2].setFlag(ItemClipsChildrenToShape);
    s2->addCircumscribedOctagon(QPen(QColor(Qt::black),7,Qt::SolidLine,Qt::FlatCap),bBlack,22.5);
    //Polygon2 * pio = s3->addInscribedOctagon(QPen(QColor(Qt::blue),3,Qt::SolidLine,Qt::FlatCap),nobrush,22.5);
    //pio->identify(&layers[4]);
    //Utils::identify(&layers[4],pio);

    // the star
    //DesignViewerPtr m3 = make_shared<DesignViewer>(diameter);
    //tileLayers[3].addToGroup(m3.get());
    //layers[2].setFlag(ItemClipsChildrenToShape);
    FigurePtr fp = make_shared<Star>(8,3.0,2.0);
    FeaturePtr nullFeature = make_shared<Feature>(8,0);
    DesignElementPtr dep   = make_shared<DesignElement>(nullFeature,fp);
#if 0 // TODO
    Viewer * viewer = new Viewer;
    FigureView * figView   = viewer->viewFigure(dep, QPointF(0,0),height);
    figView->setPen(linePen);
    //figView->setLoc(owner->getStartTile());
    //figView->setZValue(20);
    layer3->addToGroup(figView);
#endif
}


/////////////////////////////////////////////////////////////
//	  Pattern 14
/////////////////////////////////////////////////////////////

Pattern14::Pattern14(qreal Diameter, QBrush Brush) : Pattern(Diameter, Brush)
{
}

void Pattern14::build()
{
    LayerPtr layer1 = make_shared<Tile>();
    addLayer(layer1,1);

    ShapeFPtr s1 = make_shared<ShapeFactory>(diameter);
    layer1->addSubLayer(s1);

    //s1->addCircumscribedSquare(QPen(QColor(Qt::red),7), nobrush);

    s1->addInscribedSquare(linePen,nobrush);
    s1->addInscribedSquare(linePen,nobrush,45.0);

    s1->addInscribedSquare(nopen,centerBrush);
    s1->addInscribedSquare(nopen,centerBrush,45.0);
}

/////////////////////////////////////////////////////////////
//	  Pattern 15
/////////////////////////////////////////////////////////////

Pattern15::Pattern15(qreal Diameter, QBrush Brush) : Pattern(Diameter, Brush)
{
}

void Pattern15::build()
{
    LayerPtr layer1 = make_shared<Tile>();
    addLayer(layer1,1);

    ShapeFPtr s1 = make_shared<ShapeFactory>(diameter);
    layer1->addSubLayer(s1);

    //s1->addCircumscribedSquare(QPen(QColor(Qt::red),1), nobrush);
    s1->addInscribedHexagon(linePen, nobrush,90.0);
}

/////////////////////////////////////////////////////////////
//	  Pattern 16
/////////////////////////////////////////////////////////////

Pattern16::Pattern16(qreal Diameter, QBrush Brush) : Pattern(Diameter, Brush)
{
}

void Pattern16::build()
{
    LayerPtr layer1 = make_shared<Tile>();
    addLayer(layer1,1);;
    ShapeFPtr s1 = make_shared<ShapeFactory>(diameter);
    layer1->addSubLayer(s1);

    LayerPtr layer4 = make_shared<Tile>();
    addLayer(layer4,4);
    ShapeFPtr s4 = make_shared<ShapeFactory>(diameter);
    layer4->addSubLayer(s4);

    s4->addCircumscribedSquare(QPen(QColor(Qt::red),1), nobrush);

    s1->addInscribedOctagon(linePen, nobrush,22.5);
}

/////////////////////////////////////////////////////////////
//	  Pattern 22 - Japanese Kumiko 1
/////////////////////////////////////////////////////////////

PatternKumiko1::PatternKumiko1(qreal Diameter, QBrush Brush) : Pattern(Diameter, Brush)
{
}

void PatternKumiko1::build()
{
    QPen woodPen;
    woodPen.setWidthF(11.0);
    woodPen.setColor(QColor(0xa2,0x79,0x67));
    woodPen.setJoinStyle(Qt::MiterJoin);
    woodPen.setCapStyle(Qt::RoundCap);

    qreal ypos = sqrt((diameter*diameter) - (radius*radius));
    //qDebug() << "ypos" << ypos << diameter << radius;

#if 0
    ShapeFPtr s1 = make_shared<ShapeFactory>(diameter);
    layers[1].addToGroup(s1);
#endif

    s2 = make_shared<ShapeFactory>(diameter);
    LayerPtr layer2 = make_shared<Tile>();
    addLayer(layer2,2);;

    QPointF a(-radius,-ypos);
    QPointF b( radius,-ypos);
    s2->addLine(woodPen,a,b);

    QPointF c(-radius,ypos);
    QPointF d( radius,ypos);
    s2->addLine(woodPen,c,d);

    QPointF e(-radius,0);
    QPointF f( radius,0);
    s2->addLine(woodPen,e,f);

    s3 = make_shared<ShapeFactory>(diameter);
    LayerPtr layer3 = make_shared<Tile>();
    addLayer(layer3,3);;
    layer3->addSubLayer(s3);

    QPointF a1(0,ypos);
    QPointF b1(radius,0);
    s3->addLine(woodPen,a1,b1);

    QPointF c1(0,ypos);
    QPointF d1(-radius,0);
    s3->addLine(woodPen,c1,d1);

    QPointF a2(0,ypos);
    QPointF b2(radius,ypos*2);
    s3->addLine(woodPen,a2,b2);

    QPointF c2(0,ypos);
    QPointF d2(-radius,ypos*2);
    s3->addLine(woodPen,c2,d2);


    s4 = make_shared<ShapeFactory>(diameter);
    LayerPtr layer4 = make_shared<Tile>();
    addLayer(layer4,4);;
    layer4->addSubLayer(s4);

    Polygon2 * p1 = s4->addStretchedExternalHexagon(woodPen, nobrush,90.0);
    Q_UNUSED(p1)

    s5 = make_shared<ShapeFactory>(diameter);
    LayerPtr layer5 = make_shared<Tile>();
    addLayer(layer5,5);;
    layer5->addSubLayer(s5);

    Polygon2 * p2 = s5->addStretchedExternalHexagon(woodPen, nobrush,90.0);
    p2->translate(radius,ypos);
    //Utils::identify(&layers[5],p2);

#if 0
    ShapeFPtr s6 = make_shared<ShapeFactory>(diameter);
    layers[6].addToGroup(s1);
    MarkX * m = new MarkX(QPointF(0,0),QPen(QColor(Qt::green),5),"center");
    layers[6].addToGroup(m);
#endif
}

/////////////////////////////////////////////////////////////////////

PatternKumiko2::PatternKumiko2(qreal Diameter, QBrush Brush) : Pattern(Diameter, Brush)
{
    layer2 = make_shared<Tile>();
    addLayer(layer2,2);;
}

void PatternKumiko2::build()
{
    qDebug() << "PatternKumko2::build()";

    QPen woodPen;
    woodPen.setWidthF(11.0);
    woodPen.setColor(QColor(0xa2,0x79,0x67));
    woodPen.setJoinStyle(Qt::MiterJoin);
    woodPen.setCapStyle(Qt::RoundCap);

    qreal ypos = sqrt((diameter*diameter) - (radius*radius));
    //qDebug() << "ypos" << ypos << diameter << radius;

#if 0
    ShapeFPtr s1 = make_shared<ShapeFactory>(diameter);
    layers[1].addToGroup(s1);
#endif

    s2 = make_shared<ShapeFactory>(diameter);
    QPointF a(-radius,-ypos);
    QPointF b( radius,-ypos);
    s2->addLine(woodPen,a,b);

    QPointF c(-radius,ypos);
    QPointF d( radius,ypos);
    s2->addLine(woodPen,c,d);

    QPointF e(-radius,0);
    QPointF f( radius,0);
    s2->addLine(woodPen,e,f);

    s3 = make_shared<ShapeFactory>(diameter);
    QPointF a1(0,ypos);
    QPointF b1(radius,0);
    s3->addLine(woodPen,a1,b1);

    QPointF c1(0,ypos);
    QPointF d1(-radius,0);
    s3->addLine(woodPen,c1,d1);

    QPointF a2(0,ypos);
    QPointF b2(radius,ypos*2);
    s3->addLine(woodPen,a2,b2);

    QPointF c2(0,ypos);
    QPointF d2(-radius,ypos*2);
    s3->addLine(woodPen,c2,d2);

    s4 = make_shared<ShapeFactory>(diameter);
    Polygon2 * p1 = s4->addStretchedExternalHexagon(woodPen, nobrush,90.0);
    Q_UNUSED(p1)

    s5 = make_shared<ShapeFactory>(diameter);
    Polygon2 * p2 = s5->addStretchedExternalHexagon(woodPen, nobrush,90.0);
    p2->translate(radius,ypos);
    //Utils::identify(&layers[5],p2);

#if 0
    ShapeFPtr s6 = make_shared<ShapeFactory>(diameter);
    layers[6].addToGroup(s1);
    MarkX * m = new MarkX(QPointF(0,0),QPen(QColor(Qt::green),5),"center");
    layers[6].addToGroup(m);
#endif


    // Make a map of the shape factory design and create a style from it
    MapPtr map = make_shared<Map>("PatternKumiko2 map");
    map->addShapeFactory(s2);
    map->addShapeFactory(s3);
    map->addShapeFactory(s4);
    map->addShapeFactory(s5);

    // make an explicit figure and position it
    QString tileName  = "Kumiko2";
    TilingManager tm;
    TilingPtr t = tm.loadTiling(tileName,SM_LOAD_FROM_MOSAIC);
    if (!t)
    {
        t = make_shared<Tiling>(tileName, trans1, trans2);
        t->setFillData(fd);
        FeaturePtr fp = make_shared<Feature>(4,0.0);
        PlacedFeaturePtr pfp = make_shared<PlacedFeature>(fp,QTransform());
        t->add(pfp);
        t->setDescription("Kumiko2 translation vectors");
        t->setAuthor("David A. Casper");
        TilingManager tm;
        tm.saveTiling(tileName,t);
    }

    PrototypePtr proto = make_shared<Prototype>(t);

    FigurePtr fp = make_shared<ExplicitFigure>(map,FIG_TYPE_EXPLICIT,10);
    const QVector<PlacedFeaturePtr> qlfp = t->getPlacedFeatures();

    DesignElementPtr dep = make_shared<DesignElement>(qlfp[0]->getFeature(),fp);
    proto->addElement(dep);
    proto->createProtoMap();

    ThickPtr thick = make_shared<Thick>(proto);
    thick->setColor(QColor(0xa2,0x79,0x67));
    thick->setDrawOutline(true);
    thick->createStyleRepresentation();

    layer2->addSubLayer(thick);
}
