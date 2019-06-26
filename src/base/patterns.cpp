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

#include "base/border.h"
#include "base/canvas.h"
#include "base/configuration.h"
#include "base/utilities.h"
#include "base/misc.h"
#include "base/patterns.h"
#include "base/shapefactory.h"
#include "base/workspace.h"
#include "base/tiledpatternmaker.h"
#include "tapp/Rosette.h"
#include "tapp/RosetteConnectFigure.h"
#include "tapp/Star.h"
#include "tapp/Infer.h"
#include "style/Thick.h"
#include "style/Interlace.h"
#include "tapp/ExplicitFigure.h"
#include "viewers/figureview.h"
#include "viewers/designelementview.h"
#include "base/tilingmanager.h"

int Pattern::refs = 0;

/////////////////////////////////////////////////////////////
//	  Pattern
/////////////////////////////////////////////////////////////


Pattern::Pattern(qreal Diameter, QBrush Brush, int Row, int Col)
{
    diameter = Diameter;
    radius   = Diameter/2.0;
    Q_UNUSED(Row);
    Q_UNUSED(Col);

    qreal penWidth  = 7.0;
    linePen.setWidthF(penWidth);
    linePen.setColor(TileWhite);
    linePen.setJoinStyle(Qt::MiterJoin);
    linePen.setCapStyle(Qt::RoundCap);

    centerBrush = Brush;

    nopen   = QPen(Qt::NoPen);
    nobrush = QBrush(Qt::NoBrush);

    config    = Configuration::getInstance();
    workspace = Workspace::getInstance();

    refs++;
}

Pattern::~Pattern()
{
    //qDebug() << "deleting pattern...";
    refs--;
}


/////////////////////////////////////////////////////////////
// grid circles
/////////////////////////////////////////////////////////////
Pattern0::Pattern0(qreal diameter, QBrush brush) : Pattern(diameter, brush)
{
    layer1 = addLayer(1);
    layer2 = addLayer(2);
}

void Pattern0::build()
{
    linePen.setColor(QColor(Qt::blue));
    linePen.setWidthF(3.0);

    Canvas * canvas = Canvas::getInstance();
    canvas->stopTimer();

    doStep(1);
    doStep(2);
    doStep(3);
}

bool Pattern0::doStep(int Index)
{
    stepIndex = Index;

    TICK(1)
    {
        circles3x3grid(diameter,linePen);
    }
    TICK(2)
    {
        triangles3x3grid(linePen);
    }
    TICK(3)
    {
        circles3x3x3grid(diameter/2.0,linePen);
    }

    return false;
}

void Pattern0::circles3x3grid(qreal diameter, QPen pen)
{
    QPolygonF pts;
    ShapeFactory s(diameter);
    s.getOctGridPoints(pts);

    // circles
    ShapeFactory * w;

    int i = 0;


    pen.setColor(Qt::blue);
    w = new ShapeFactory(diameter,s.mapToScene(pts[i++]));
    w->addCircle(diameter,pen);
    layer1->addToGroup(w);

    pen.setColor(Qt::green);
    w = new ShapeFactory(diameter,s.mapToScene(pts[i++]));
    w->addCircle(diameter,pen);
    layer1->addToGroup(w);

    pen.setColor(Qt::red);
    w = new ShapeFactory(diameter,s.mapToScene(pts[i++]));
    w->addCircle(diameter,pen);
    layer1->addToGroup(w);

    pen.setColor(Qt::blue);
    w = new ShapeFactory(diameter,s.mapToScene(pts[i++]));
    w->addCircle(diameter,pen);
    layer1->addToGroup(w);

    pen.setColor(Qt::green);
    w = new ShapeFactory(diameter,s.mapToScene(pts[i++]));
    w->addCircle(diameter,pen);
    layer1->addToGroup(w);

    pen.setColor(Qt::red);
    w = new ShapeFactory(diameter,s.mapToScene(pts[i++]));
    w->addCircle(diameter,pen);
    layer1->addToGroup(w);

    pen.setColor(Qt::blue);
    w = new ShapeFactory(diameter,s.mapToScene(pts[i++]));
    w->addCircle(diameter,pen);
    layer1->addToGroup(w);

    pen.setColor(Qt::green);
    w = new ShapeFactory(diameter,s.mapToScene(pts[i++]));
    w->addCircle(diameter,pen);
    layer1->addToGroup(w);
}

void Pattern0::circles3x3x3grid(qreal diameter, QPen pen)
{
    qreal csize    = diameter;

    // circles
    ShapeFactory s(diameter);

    ShapeFactory * w;


    // top
    pen.setColor(Qt::blue);
    w = new ShapeFactory(diameter,s.mapToScene(-csize,-csize*2));
    w->addCircle(diameter,pen);
    layer2->addToGroup(w);

    pen.setColor(Qt::green);
    w = new ShapeFactory(diameter,s.mapToScene(csize,-csize*2));
    w->addCircle(diameter,pen);
    layer2->addToGroup(w);

    //row 2
    pen.setColor(Qt::red);
    w = new ShapeFactory(diameter,s.mapToScene(-csize*2.0,-csize));
    w->addCircle(diameter,pen);
    layer2->addToGroup(w);

    pen.setColor(Qt::red);
    w = new ShapeFactory(diameter,s.mapToScene(0.0,-csize));
    w->addCircle(diameter,pen);
    layer2->addToGroup(w);

    pen.setColor(Qt::blue);
    w = new ShapeFactory(diameter,s.mapToScene(csize*2.0,-csize));
    w->addCircle(diameter,pen);
    layer2->addToGroup(w);

    // row 3
    pen.setColor(Qt::green);
    w = new ShapeFactory(diameter,s.mapToScene(-csize,0.0));
    w->addCircle(diameter,pen);
    layer2->addToGroup(w);

    pen.setColor(Qt::green);
    w = new ShapeFactory(diameter,s.mapToScene(csize,0.0));
    w->addCircle(diameter,pen);
    layer2->addToGroup(w);

    // row 4
    pen.setColor(Qt::red);
    w = new ShapeFactory(diameter,s.mapToScene(-csize*2.0,csize));
    w->addCircle(diameter,pen);
    layer2->addToGroup(w);

    pen.setColor(Qt::blue);
    w = new ShapeFactory(diameter,s.mapToScene(0.0,csize));
    w->addCircle(diameter,pen);
    layer2->addToGroup(w);

    pen.setColor(Qt::blue);
    w = new ShapeFactory(diameter,s.mapToScene(csize*2.0,csize));
    w->addCircle(diameter,pen);
    layer2->addToGroup(w);

    // row 5
    pen.setColor(Qt::blue);
    w = new ShapeFactory(diameter,s.mapToScene(-csize,csize*2.0));
    w->addCircle(diameter,pen);
    layer2->addToGroup(w);

    pen.setColor(Qt::green);
    w = new ShapeFactory(diameter,s.mapToScene(csize,csize*2.0));
    w->addCircle(diameter,pen);
    layer2->addToGroup(w);

}

// triangles
void Pattern0::triangles3x3grid(QPen Pen)
{
    ShapeFactory * s = new ShapeFactory(diameter);
    layer1->addToGroup(s);

    QPolygonF pts;
    s->getOctGridPoints(pts);

    QPolygonF p;

    QPen pen = Pen;
    QBrush brush(Qt::NoBrush);

    pen.setColor(Qt::darkCyan);
    p << pts[7] << pts[2] << pts[4];
    s->addPolygon(pen,brush,p);

    pen.setColor(Qt::darkMagenta);
    p.clear();
    p << pts[1] << pts[4] << pts[6];
    s->addPolygon(pen,brush,p);

    pen.setColor(Qt::darkYellow);
    p.clear();
    p << pts[3] << pts[6] << pts[0];
    s->addPolygon(pen,brush,p);

    pen.setColor(Qt::darkGray);
    p.clear();
    p << pts[5] << pts[0] << pts[2];
    s->addPolygon(pen,brush,p);

}

/////////////////////////////////////////////////////////////
// packed circles
/////////////////////////////////////////////////////////////
Pattern1::Pattern1(qreal diameter, QBrush brush)  : Pattern(diameter,brush)
{
}

void Pattern1::build()
{
    qreal penWidth  = 3.0;
    QPen pen;
    pen.setWidthF(penWidth);

    Layer * layer1 = addLayer(1);
    Layer * layer2 = addLayer(2);
    Layer * layer3 = addLayer(3);

    ShapeFactory * s = new ShapeFactory(diameter);
    layer1->addToGroup(s);

    // centre
    pen.setColor(Qt::blue);
    s->addCircle(diameter,pen);
    s->add9ptStar(pen,nobrush);

    QPolygonF p;
    s->getHexPackingPoints(p);

    QPolygonF::iterator it;
    int count = 0;
    for (it = p.begin(); it != p.end(); it++, count++)
    {

        // circles
        if (count < 3)
            pen.setColor(Qt::red);
        else
            pen.setColor(Qt::green);
        ShapeFactory * w = new ShapeFactory(diameter);
        w->setLoc(*it);
        w->addCircle(diameter,pen);
        w->add9ptStar(pen,nobrush);
        layer2->addToGroup(w);
    }

    ShapeFactory * c = new ShapeFactory(diameter);
    layer3->addToGroup(c);

    pen.setColor(Qt::darkMagenta);
    QPointF p0(0.0,0.0);

    QPolygonF p2;
    p2 << p0 << p[0] << p[1];
    triangleInTriangle(c,p2, pen);

    p2.clear();
    p2  << p0 << p[1] << p[2];
    triangleInTriangle(c,p2, pen);

    p2.clear();
    p2  << p0 << p[2] << p[3];
    triangleInTriangle(c,p2, pen);

    p2.clear();
    p2  << p0 << p[3] << p[4];
    triangleInTriangle(c,p2, pen);

    p2.clear();
    p2  << p0 << p[4] << p[5];
    triangleInTriangle(c,p2, pen);

    p2.clear();
    p2  << p0 << p[5] << p[0];
    triangleInTriangle(c,p2, pen);
}

void Pattern1::triangleInTriangle(ShapeFactory * circle, QPolygonF & p, QPen & pen)
{
    circle->addPolygon(pen,QBrush(Qt::NoBrush),p);

    QPolygonF p2;
    p2 = ShapeFactory::getMidPoints(p);
    circle->addPolygon(pen,QBrush(Qt::NoBrush),p2);
}

/////////////////////////////////////////////////////////////
//  moving packed circles
/////////////////////////////////////////////////////////////
Pattern2::Pattern2(qreal diameter, QBrush brush) : Pattern(diameter,brush)
{
    layer1 = addLayer(1);
    layer3 = addLayer(3);
}

void Pattern2::build()
{
    qreal penWidth  = 3.0;
    a               = 0.0;
    b               = 0.0;

    QPen pen;
    pen.setWidthF(penWidth);

    ShapeFactory * w;

    // a stack of circles - all start at center
    for (int i=0; i < 7; i++)
    {
        w = new ShapeFactory(diameter);
        if (i==0)
        {
            pen.setColor(Qt::black);
            firstCircle = w->addCircle(diameter,pen);
        }
        else
        {
            pen.setColor(Qt::blue);
            w->addCircle(diameter,pen);
        }

        w->setVisible(false);
        shapes.push_back(w);
        layer1->addToGroup(w);
    }

    // cover with one large black circle
    w = shapes[0];
    firstCircle->setBrush(QBrush(Qt::black));
    firstCircle->setDiameter(BIG);
    w->setVisible(true);
    w->setZValue(1.0);
    //w->prepareGeometryChange();

    Canvas * canvas = Canvas::getInstance();
    canvas->setMaxStep(51);
}

bool Pattern2::doStep(int Index)
{
    stepIndex = Index;
    //qDebug() << "step" << stepIndex;

    // fill in the circles
    ShapeFactory * w;

    RANGE(0,5)
    {
        int start    = SECONDS(0);
        int duration = SECONDS(5);
        fadeFromBlackToPoint(start,duration);
    }
    RANGE(5,5)
    {
        int start    = SECONDS(5);
        int duration = SECONDS(5);
        drawRadialLine(start,duration);
    }
    RANGE (10,5)
    {
        int start    = SECONDS(10);
        int duration = SECONDS(5);
        undrawRadial(start,duration);
        drawArcToCircles(start,duration);
    }
    RANGE(15,5)
    {
       STEP(15)
       {
           for (int i=0; i < 7; i++)
           {
               shapes[i]->setVisible(true);
           }
       }
       int start    = SECONDS(15);
       int duration = SECONDS(5);
       expandCircles(start,duration);
    }
    RANGE(20,8)
    {
        switch (stepIndex)
        {
        case SECONDS(20):
            layer3->setVisible(false);
            return true;
        case SECONDS(21):
            w = shapes[0];
            break;
        case SECONDS(22):
            w = shapes[5];
            break;
        case SECONDS(23):
            w = shapes[3];
            break;
        case SECONDS(24):
            w = shapes[1];
            break;
        case SECONDS(25):
            w = shapes[2];
            break;
        case SECONDS(26):
            w = shapes[4];
            break;
        case SECONDS(27):
            w = shapes[6];
            break;

        default:
            goto END;
        }

        w->add6ptStar(firstCircle->getPen(),firstCircle->getBrush());
        //w->create9ptStar();
        //w->setRadials(true);
        w->addInscribedHexagon(firstCircle->getPen(),firstCircle->getBrush(),30.0);
END:
        ;
    }
    RANGE(28,5)
    {
        rotateCircles();
    }
    RANGE(33,5)
    {
        rotateAll();
        rotateCircles();
    }
    RANGE(38,5)
    {
        rotateAll();
        int start    = SECONDS(38);
        int duration = SECONDS(5);
        contractCircles(start,duration);
        rotateCircles();
    }
    RANGE(43,5)
    {
        STEP(43)
        {
            a = shapes[0]->rotation();
            b = shapes[1]->rotation();
            //qDebug() << shapes[0]->rotation() << shapes[1]->rotation();
            a = qRound(a) % 360;
            b = qRound(b) % 360;
        }
        restoreRotation(SECONDS(43),SECONDS(5));
        //qDebug() << shapes[0]->rotation() << shapes[1]->rotation();
        rotateAll();
    }
    RANGE(48,1)
    {
        STEP(48)
        {
            //qDebug() << "inner rotations:" << shapes[0]->rotation() << shapes[1]->rotation();
            cStart = rotation();
            //qDebug() << "ext rotation:" << cStart;
            cStart = cStart - ((qRound(cStart) / 360) *360.0);
            //qDebug() << "nor rotation:" << cStart;
        }
        restoreAllRotation(SECONDS(48),SECONDS(1));
    }
    STEP(49)
    {
        //qDebug() << shapes[0]->rotation() << shapes[1]->rotation();
        //qDebug() << "total rotation" << rotation();
        return false;
    }
    return true;
}

void Pattern2::fadeFromBlackToPoint(int start, int duration)
{
    int i = stepIndex - start;
    qreal fraction =  getSLinearPos(i,duration);
    qreal amount = BIG - (fraction * BIG );
    //qDebug() << amount;

    firstCircle->setDiameter(amount);
    prepareGeometryChange();
}

void Pattern2::drawArcToCircles(int start, int duration)
{
    int i = stepIndex - start;
    qreal fraction =  getSLinearPos(i,duration);
    int amount = static_cast<int>(fraction * 5760.0);
    //qDebug() << amount;

    firstCircle->setArc(ARC,90*16,amount);
    prepareGeometryChange();
}

void Pattern2::drawRadialLine(int start,int duration)
{
    int i = stepIndex - start;
    qreal fraction =  getSLinearPos(i,duration);
    qreal amount = fraction * (diameter/2.0);
    qDebug() << amount;

    ShapeFactory * w = shapes[0];
    QPointF loc2 = mapToScene(w->getLoc());
    qDebug() << w->getLoc() << loc2;

    if (i == 0.0)
    {
        firstCircle->setBrush(QBrush(Qt::NoBrush));
        firstCircle->setDiameter(diameter);
        firstCircle->setPen(QPen(QColor(Qt::black),3.0));
        firstCircle->setArc(ARC,90*16,0);
        prepareGeometryChange();
        update();

        QPen linePen = QPen(QColor(Qt::black),3.0);
        //gli->setZValue(2.0); // FIXME
        pLine = w->addLine(linePen,QPointF(),QPointF());
    }

    pLine->clear();
    *pLine << QPointF(loc2.x(), loc2.y()) << QPointF(loc2.x(), loc2.y() - amount);
}

void Pattern2::undrawRadial(int start,int duration)
{
    int i = stepIndex - start;
    qreal fraction =  getSLinearPos(i,duration);
    qreal amount = fraction * (diameter/2.0);
    //qDebug() << amount;

    ShapeFactory * w = shapes[0];

    QPointF loc2 = mapToScene(w->getLoc());
    //qDebug() << w->getLoc() << loc2;

    pLine->clear();
    *pLine << QPointF(loc2.x(), loc2.y() - amount) << QPointF(loc2.x(), loc2.y() - (diameter/2.0));
}

void Pattern2::contractCircles(int start, int duration)
{
    int i = stepIndex - start;

    qreal csize, height;
    if (i >= duration)
    {
        csize  = 0;
        height = 0;
    }
    else
    {
        qreal fraction =  getSLinearPos(i,duration);
        csize  = diameter * fraction;
        csize  = diameter - csize;
        // packing of circles has equlateral triangle of centers
        height = sqrt((csize*csize) - ((csize/2.0)*(csize/2.0)));
    }

    positionCircles(csize * 0.5,height);
}

void Pattern2::expandCircles(int start, int duration)
{
    int i = stepIndex - start;

    qreal fraction =  getSLinearPos(i,duration);
    qreal csize    = diameter * fraction;
    // packing of circles has equlateral triangle of centers
    qreal height = sqrt((csize*csize) - ((csize/2.0)*(csize/2.0)));

    positionCircles(csize * 0.5,height);
}

void Pattern2::positionCircles(qreal xpos, qreal ypos)
{
    shapes[0]->setLoc(0,        0);          // centre - 0
    shapes[1]->setLoc(-xpos,    ypos);       // below left - 1
    shapes[2]->setLoc(xpos,     ypos);       // below right - 2
    shapes[3]->setLoc(-xpos*2,  0);          // left - 3
    shapes[4]->setLoc(xpos*2,   0);          // right - 4
    shapes[5]->setLoc(-xpos,    -ypos);      // top left -5
    shapes[6]->setLoc(xpos,     -ypos);      // top right - 6
}

void Pattern2::rotateCircles()
{
    static qreal fwd  = 0.0;
    static qreal back = 0.0;

    fwd  += 1.0;
    back += -1.0;

    ShapeFactory * c;
    for (int i=0; i<7; i++)
    {
        c = shapes[i];
        c->setTransformOriginPoint(c->getLoc());
        if (i==0)
        {
            c->setRotation(fwd);
        }
        else
        {
            c->setRotation(back);
        }
    }
}

void Pattern2::restoreRotation(int start, int duration)
{
    int i = stepIndex - start;
    qreal fraction =  getSLinearPos(i,duration);
    qreal amounta = a - (fraction * a);
    qreal amountb = b - (fraction * b);
    qDebug() << "a:" << a << "b:" << b;

    ShapeFactory * c;
    for (int i=0; i<7; i++)
    {
        c = shapes[i];
        c->setTransformOriginPoint(c->getLoc());
        if (i==0)
        {
            c->setRotation(amounta);
        }
        else
        {
            c->setRotation(amountb);
        }
    }
}

void Pattern2::rotateAll()
{
    static qreal all  = 0.0;

    setTransformOriginPoint(boundingRect().center());
    all += 0.5;
    setRotation(all);
}

void Pattern2::restoreAllRotation(int start, int duration)
{
    int i = stepIndex - start;
    qreal fraction =  getSLinearPos(i,duration);
    qreal amountc = (fraction * (360.0 - cStart));

    setTransformOriginPoint(boundingRect().center());
    qreal rot = cStart + amountc;
    qDebug() << "before" << rotation();
    setRotation(rot);
    qDebug() << "after " << rotation();
}


/////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////
Pattern3::Pattern3(qreal Diameter)  : Pattern(Diameter,QBrush(Qt::NoBrush))
{
    layer1 = addLayer(1);
    layer2 = addLayer(2);
    layer3 = addLayer(3);
}

void Pattern3::build()
{
    linePen.setWidthF(3.0);
    linePen.setColor(Qt::black);

    ShapeFactory * s = new ShapeFactory(diameter);
    layer1->addToGroup(s);

    Canvas * canvas = Canvas::getInstance();
    canvas->stopTimer();

    doStep(0);
    doStep(1);
    doStep(2);
    doStep(3);
    doStep(4);
}

bool Pattern3::doStep(int Index)
{
    stepIndex = Index;
    //qDebug() << "step" << stepIndex;

    QBrush abrush(Qt::green);
    QBrush bbrush(Qt::blue);

    ShapeFactory * s = dynamic_cast<ShapeFactory*>(layer1->childItems().first());

    TICK(0)
    {
        s->addInscribedTriangle(linePen, abrush, 0.0);
    }
    TICK(1)
    {
        s->addCircumscribedTriangle(linePen, bbrush, 0.0);
    }
    TICK(2)
    {
        QPolygonF pts;
        s->getHexPackingPoints(pts);
        for (int i = 0; i < 6; i++)
        {
            ShapeFactory * c = new ShapeFactory(diameter,s->mapToScene(pts[i]));
            c->addCircle(diameter,linePen);
            c->addInscribedTriangle(linePen, nobrush, -30.0);
            layer2->addToGroup(c);
        }
        layer2->setRotation(30);
    }
    TICK(3)
    {
        ShapeFactory * c = new ShapeFactory(diameter*2,s->mapToScene(s->getLoc()));
        c->addCircle(diameter,linePen);
        layer3->addToGroup(c);
    }
    TICK(4)
    {
        ShapeFactory * c = new ShapeFactory(diameter*3,s->mapToScene(s->getLoc()));
        c->addCircle(diameter,linePen);
        layer3->addToGroup(c);
    }
    return true;
}

/////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////
Pattern4::Pattern4(qreal Diameter) : Pattern(Diameter,QBrush(Qt::NoBrush))
{
    layer1 = addLayer(1);
    layer2 = addLayer(2);
    layer3 = addLayer(3);
}

void Pattern4::build()
{
    linePen.setWidthF(3.0);
    linePen.setColor(Qt::black);

    ShapeFactory * s = new ShapeFactory(diameter);
    layer1->addToGroup(s);

    Canvas * canvas = Canvas::getInstance();
    canvas->stopTimer();

    doStep(0);
    doStep(1);
    doStep(2);
    doStep(3);
    doStep(4);
}

bool Pattern4::doStep(int Index)
{
    stepIndex = Index;
    //qDebug() << "step" << stepIndex;

   ShapeFactory * s = dynamic_cast<ShapeFactory*>(layer1->childItems().first());

    TICK(0)
    {
        Polygon2 * p = s->addInscribedSquare(linePen,nobrush,0.00);
        p->radials = true;
    }
    TICK(1)
    {
        s->addCircumscribedSquare(linePen,nobrush,0.0);
    }
    TICK(2)
    {
        QPolygonF pts;
        s->getHexPackingPoints(pts);
        for (int i = 0; i < 6; i++)
        {
            ShapeFactory * c = new ShapeFactory(diameter,s->mapToScene(pts[i]));
            c->addCircle(diameter,linePen);
            layer2->addToGroup(c);
        }
        layer2->setRotation(30);
    }
    TICK(3)
    {
        ShapeFactory * c = new ShapeFactory(diameter*2,s->mapToScene(s->getLoc()));
        c->addCircle(diameter,linePen);
        layer3->addToGroup(c);
    }
    TICK(4)
    {
        ShapeFactory * c = new ShapeFactory(diameter*3,s->mapToScene(s->getLoc()));
        c->addCircle(diameter,linePen);
        layer3->addToGroup(c);
    }
    return true;
}

/////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////

Pattern5::Pattern5(qreal Diameter, QBrush Brush)  : Pattern(Diameter, Brush)
{
    layer1 = addLayer(1);
    layer2 = addLayer(2);
    layer3 = addLayer(3);
}

void Pattern5::build()
{
    ShapeFactory * s = new ShapeFactory(diameter);
    layer1->addToGroup(s);

    layer1->setZValue(2.0);
    layer2->setZValue(3.0);
    layer3->setZValue(1.0);

    // this used to generate a background
    ShapeFactory * c2 = new ShapeFactory(diameter*1.8);
    c2->addCircle(diameter*1.8,nopen,QBrush(QColor(TileWhite)));
    layer3->addToGroup(c2);

    Canvas * canvas = Canvas::getInstance();
    canvas->stopTimer();

    doStep(0);
    doStep(3);
}

bool Pattern5::doStep(int Index)
{
    stepIndex = Index;
    //qDebug() << "step" << stepIndex;

   ShapeFactory * s = dynamic_cast<ShapeFactory*>(layer1->childItems().first());

    TICK(0)
    {
        s->addInscribedHexagon(linePen,centerBrush,0.0);
    }
    TICK(1)
    {
        s->addCircumscribedHexagon(linePen,QBrush(QColor(TileBlack)),0.0);
    }
    TICK(2)
    {
        s->addExternalHexagon(linePen,centerBrush,90.0);
    }
    TICK(3)
    {
        QPolygonF pts;
        s->getHexPackingPoints(pts);
        for (int i = 0; i < 6; i++)
        {
            //ShapeFactory * c = new ShapeFactory(diameter,s->mapToScene(pts[i]));
            ShapeFactory * c = new ShapeFactory(diameter,pts[i]);
            c->addInscribedHexagon(linePen,QBrush(TileBlack),0.0);
            layer2->addToGroup(c);
        }
        layer2->setRotation(30);
    }
    TICK(31)
    {
        QBrush brush1;
        brush1.setColor(TileGreen);
        brush1.setStyle(Qt::SolidPattern);

        QPolygonF pts;
        s->getHexPackingPoints(pts);
        for (int i = 0; i < 6; i++)
        {
            ShapeFactory * c = new ShapeFactory(diameter,pts[i]);
            c->addCircle(diameter,linePen);
            c->addCircumscribedHexagon(linePen,brush1,0.0);
            layer2->addToGroup(c);
        }
        layer2->setRotation(30);
    }
    TICK(4)
    {
        ShapeFactory * c = new ShapeFactory(diameter*2);
        c->addCircle(diameter*2,linePen);
        layer3->addToGroup(c);
    }
    TICK(5)
    {
        ShapeFactory * c = new ShapeFactory(diameter*3);
        c->addCircle(diameter*3,linePen);
        layer3->addToGroup(c);
    }
    return true;
}

/////////////////////////////////////////////////////////////
//	  Pattern 6
/////////////////////////////////////////////////////////////
Pattern6::Pattern6(qreal Diameter, QBrush Brush) : Pattern(Diameter, Brush)

{
    layer1 = addLayer(1);
    layer2 = addLayer(2);
}

void Pattern6::build()
{
    ShapeFactory * s = new ShapeFactory(diameter);
    layer1->addToGroup(s);

    Canvas * canvas = Canvas::getInstance();
    canvas->stopTimer();

    doStep(0);
    doStep(3);
}

bool Pattern6::doStep(int Index)
{
    stepIndex = Index;
    //qDebug() << "step" << stepIndex;

   ShapeFactory * s = dynamic_cast<ShapeFactory*>(layer1->childItems().first());

    TICK(0)
    {
        s->addInscribedHexagon(QPen(centerBrush.color()),centerBrush,0.0);
    }
    TICK(3)
    {
        QPolygonF pts;
        s->getHexPackingPoints(pts);
        for (int i = 0; i < 6; i++)
        {
            ShapeFactory * c = new ShapeFactory(diameter,s->mapToScene(pts[i]));
            c->addInscribedHexagon(QPen(TileBlack),QBrush(TileBlack),0.0);
            layer2->addToGroup(c);
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////
//	  Pattern 7
/////////////////////////////////////////////////////////////
Pattern7::Pattern7(qreal Diameter, QBrush Brush) : Pattern(Diameter, Brush)
{
    layer1 = addLayer(1);
}

void Pattern7::build()
{
    static bool two = false;


    ShapeFactory * s = new ShapeFactory(diameter);
    layer1->addToGroup(s);

    Canvas * canvas = Canvas::getInstance();
    canvas->stopTimer();

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
    ShapeFactory * s = dynamic_cast<ShapeFactory*>(layer1->childItems().first());
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
    layer1 = addLayer(1);
    layer3 = addLayer(3);

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
    s = new ShapeFactory(diameter);
    layer1->addToGroup(s);

    s3 = new ShapeFactory(diameter);
    layer3->addToGroup(s3);

    //canvas->views().at(0)->setRenderHint(QPainter::Antialiasing ,false);
    //canvas->views().at(0)->setRenderHint(QPainter::SmoothPixmapTransform,false);
    s->enableAntialiasing(false);
    s3->enableAntialiasing(false);
}

bool PatternHuSymbol::doStep(int Index)
{
    stepIndex = Index;

    TICK(0)
    {
        gridPen.setCapStyle(Qt::RoundCap);

        ShapeFactory c(width*16.0);
        Polygon2 * p = c.addCircumscribedOctagon(gridPen,nobrush,0.0);
        p->setInnerPen(innerPen);
        s->addPolygon(*p);
    }
    TICK (1)
    {
        gridPen.setCapStyle(Qt::SquareCap);
        innerPen.setCapStyle(Qt::SquareCap);

        ShapeFactory c(width*4.0);
        Polygon2 * p = c.addCircumscribedSquare(gridPen,nobrush,0.0);
        p->setInnerPen(innerPen);
        s->addPolygon(*p);
    }
    TICK(2)
    {

        ShapeFactory c(width*12.0);
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
        p->translate( s->getLoc() + QPointF(-width,-(width*5.0))) ;
        s->addPolygon(*p);
    }
    TICK(4)
    {
        ShapeFactory c(width*2.0);
        Polygon2 * p = c.addCircumscribedSquare(gridPen,nobrush,0.0);
        p->setInnerPen(innerPen);
        p->translate( s->getLoc() + QPointF(width,(width*5.0))) ;
        s->addPolygon(*p);
    }
    TICK(5)
    {
        ShapeFactory c(width*2.0);
        Polygon2 * p = c.addCircumscribedSquare(gridPen,nobrush,0.0);
        p->setInnerPen(innerPen);
        p->translate( s->getLoc() + QPointF((width*5.0),-width)) ;
        s->addPolygon(*p);
    }
    TICK(6)
    {
        ShapeFactory c(width*2.0);
        Polygon2 * p = c.addCircumscribedSquare(gridPen,nobrush,0.0);
        p->translate( s->getLoc() + QPointF(-(width*5.0),width)) ;
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
    layer1 = addLayer(1);
    layer2 = addLayer(2);
    layer3 = addLayer(3);
    layer4 = addLayer(4);


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
    l1 = new ShapeFactory(diameter);
    l1->enableAntialiasing(false);
    layer1->addToGroup(l1);

    // level 2
    l2 = new ShapeFactory(diameter);
    l2->enableAntialiasing(false);
    layer2->addToGroup(l2);

    // level 3
    l3 = new ShapeFactory(diameter);
    l3->enableAntialiasing(false);
    layer3->addToGroup(l3);

    // level 4
    l4 = new ShapeFactory(diameter);
    l4->enableAntialiasing(false);
    layer4->addToGroup(l4);


}

bool PatternHuInterlace::doStep(int Index)
{
    stepIndex = Index;
    ShapeFactory * s = dynamic_cast<ShapeFactory*>(layer1->childItems().first());

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
    layer1 = addLayer(1);
}

void Pattern10::build()
{

    ShapeFactory * s = new ShapeFactory(diameter);
    layer1->addToGroup(s);

    Canvas * canvas = Canvas::getInstance();
    canvas->stopTimer();

    doStep(0);
    //step(1);
    //step(2);
    //step(3);
}

bool Pattern10::doStep(int Index)
{
    stepIndex = Index;
    ShapeFactory * s = dynamic_cast<ShapeFactory*>(layer1->childItems().first());

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
    layer1 = addLayer(1);
    layer2 = addLayer(2);

    Rotation  = rotation;
    Direction = direction;
}

void Pattern11::build()
{
    // the hexagon
    ShapeFactory * s = new ShapeFactory(diameter);
    layer1->addToGroup(s);
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
        ShapeFactory * s2 = new ShapeFactory(diameter);
        layer2->addToGroup(s2);

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
    layer1 = addLayer(1);
    layer2 = addLayer(2);
    layer3 = addLayer(3);

    Rotation  = rotation;
    Direction = direction;
}

void Pattern12::build()
{
    // the hexagon
    ShapeFactory * s = new ShapeFactory(diameter);
    layer1->addToGroup(s);
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
        ShapeFactory * s2 = new ShapeFactory(diameter);
        layer2->addToGroup(s2);

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

    ShapeFactory * s3;

    // add hexagons in colors
    s3 = new ShapeFactory(diameter/2.9, hexagon->at(0));
    layer3->addToGroup(s3);
    s3->addInscribedHexagon(pWhite,bWhite,0.0);

    s3 = new ShapeFactory(diameter/2.9, hexagon->at(4));
    layer3->addToGroup(s3);
    s3->addInscribedHexagon(pWhite,bWhite,0.0);

    // add stars in colors
    s3 = new ShapeFactory(diameter/2.9, hexagon->at(3));
    layer3->addToGroup(s3);
    s3->add6ptStar(pGold,bGold,90.0);

    s3 = new ShapeFactory(diameter/2.9, hexagon->at(5));
    layer3->addToGroup(s3);
    s3->add6ptStar(pGold,bGold,90.0);
}

/////////////////////////////////////////////////////////////
//	  Pattern 13 - The once I could not complete
/////////////////////////////////////////////////////////////

PatternIncompleteA::PatternIncompleteA(qreal Diameter, QBrush Brush) : Pattern(Diameter, Brush)
{
    layer1 = addLayer(1);
    layer2 = addLayer(2);
    layer3 = addLayer(3);
    layer4 = addLayer(4);
}

void PatternIncompleteA::build()
{
    // brushes & pens
    QBrush bBlack(Qt::black);
    QPen   rPen = QPen(QColor(Qt::red),1);

    linePen.setJoinStyle(Qt::MiterJoin);
    linePen.setCapStyle(Qt::RoundCap);

    // outlines for debug
    ShapeFactory * s4 = new ShapeFactory(diameter);
    layer4->addToGroup(s4);
    layer4->setFlag(ItemClipsChildrenToShape);
    // defining (invisible) square
    //Polygon2 *  p = s4->addCircumscribedSquare(rPen,nobrush);
    Polygon2 *  p = s4->addCircumscribedSquare(nopen,nobrush);  // invisible
    // debug outlines (red)
    //s4->addCircle(diameter,QPen(QColor(Qt::red)));

    // main design
    ShapeFactory * s1 = new ShapeFactory(diameter);
    layer1->addToGroup(s1);
    layer1->setFlag(ItemClipsChildrenToShape);

    qreal radius = diameter/2.0;
    QPointF a;
    QPointF b;
    QPointF c;
    QPointF d;

    QGraphicsEllipseItem i(-radius,-radius,diameter,diameter);

    QLineF line2(p->at(0),p->at(2));
    int points = Utils::circleLineIntersectionPoints(i,radius,line2,a,b);

    QLineF line3(p->at(1),p->at(3));
    points = Utils::circleLineIntersectionPoints(i,radius,line3,c,d);

    Q_UNUSED(points);

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
    ShapeFactory * s2 = new ShapeFactory(diameter/2.6);  // need to do the math 2.6
    layer2->addToGroup(s2);
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
    FeaturePtr nullFeature = make_shared<Feature>();
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
    Layer * layer1 = addLayer(1);

    ShapeFactory * s1 = new ShapeFactory(diameter);
    layer1->addToGroup(s1);

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
    Layer * layer1 = addLayer(1);
    ShapeFactory * s1 = new ShapeFactory(diameter);
    layer1->addToGroup(s1);

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
    Layer * layer1 = addLayer(1);
    ShapeFactory * s1 = new ShapeFactory(diameter);
    layer1->addToGroup(s1);

    Layer * layer4 = addLayer(4);
    ShapeFactory * s4 = new ShapeFactory(diameter);
    layer4->addToGroup(s4);

    s4->addCircumscribedSquare(QPen(QColor(Qt::red),1), nobrush);

    s1->addInscribedOctagon(linePen, nobrush,22.5);
}
/////////////////////////////////////////////////////////////
//	  Pattern 17 - Rosette
/////////////////////////////////////////////////////////////

PatternRosette::PatternRosette(qreal Diameter, QBrush Brush) : Pattern(Diameter, Brush)
{
}

void PatternRosette::build()
{
#if 0
    Layer * layer4 = addLayer(4);
    ShapeFactory * s4 = new ShapeFactory(diameter/2.0);
    layer4->addToGroup(s4);
    s4->addCircumscribedSquare(QPen(QColor(Qt::green)),nobrush);

    Layer * layer3 = addLayer(3);
    ShapeFactory * s3 = new ShapeFactory(diameter/2.0);
    layer3->addToGroup(s3);
    s3->addCircle(diameter/2.0,QPen(QColor(Qt::blue)));
#endif

    Layer * layer2         = addLayer(2);
    FigurePtr  fp          = make_shared<Rosette>(8,0.5,2.0);

#if 0
    FeaturePtr featp       = make_shared<Feature>(8);
    DesignElementPtr dep   = make_shared<DesignElement>(featp,fp);
    FigureView * fv        = viewFigure(dep, QPointF(0.0,0.0),height);
    layer2->addToGroup(fv);
    QPointF pt(600.0,450.0);
    MarkX * mark = new MarkX(pt,QPen(Qt::blue,3)," ");
    layer2->addToGroup(mark);
#else
    // place the figure
    QString tileName  = "DAC new 4.v5";
    TilingManager* tm = TilingManager::getInstance();
    TilingPtr t       = tm->loadTiling(tileName);
    Q_ASSERT(t);

    PrototypePtr proto = make_shared<Prototype>(t);
    QList<PlacedFeaturePtr> qlfp = t->getPlacedFeatures();

    DesignElementPtr dep = make_shared<DesignElement>(qlfp[0]->getFeature(),fp);
    proto->addElement(dep);

    QPolygonF bounds;
    bounds << QPointF(-diameter, diameter);
    bounds << QPointF( diameter, diameter);
    bounds << QPointF( diameter,-diameter);
    bounds << QPointF(-diameter,-diameter);

    Interlace * interlace = new Interlace(proto,make_shared<QPolygonF>(bounds));
    interlace->setColor(QColor(0xa2,0x79,0x67));
    interlace->setDrawOutline(true);
    interlace->setGap(0.05);

    layer2->addToGroup(interlace);
#endif
#if 0
    MarkX  * mark;
    for (int i=0; i < r->points.size(); i++)
    {
        QPointF pt = r->points.at(i);
        pt *= radius;
        mark = new MarkX(pt,QPen(QColor(Qt::green),3),i);
        layers[4].addToGroup(mark);
    }
    for (int i=0; i < m->getVertices().size(); i++)
    {
        QPointF pt = m->getVertices().at(i)->getPosition();
        pt *= radius;
        mark = new MarkX(pt,QPen(QColor(Qt::blue),3),i);
        layers[4].addToGroup(mark);
    }
#endif
#if 0

    for (int i=0; i < m->getEdges()->size(); i++)
    {
        Edge *  e = m->getEdges()->at(i);
        QPointF pt1 = e->getV1()->getPosition();
        pt1 *= radius;
        mark = new MarkX(pt1,QPen(QColor(Qt::white),3),i*2);
        layers[4].addToGroup(mark);
        QPointF pt2 = e->getV2()->getPosition();
        pt2 *= radius;
        mark = new MarkX(pt2,QPen(QColor(Qt::white),3),(i*2)+1);
        layers[4].addToGroup(mark);
    }
#endif
}

/////////////////////////////////////////////////////////////
//	  Pattern 18 - Star
/////////////////////////////////////////////////////////////

PatternStar::PatternStar(qreal Diameter, QBrush Brush) : Pattern(Diameter, Brush)
{
}

void PatternStar::build()
{
#if 0
    Layer * layer4 = addLayer(4);
    ShapeFactory * s4 = new ShapeFactory(diameter/2.0);
    layer4->addToGroup(s4);
    s4->addCircumscribedSquare(QPen(QColor(Qt::green)),nobrush);

    Layer * layer3 = addLayer(3);
    ShapeFactory * s3 = new ShapeFactory(diameter/2.0);
    layer3->addToGroup(s3);
    s3->addCircle(diameter/2.0,QPen(QColor(Qt::blue)));
#endif

    Layer * layer2         = addLayer(2);
    FigurePtr fp           = make_shared<Star>(8,3.0,2.0);

#if 0
    FeaturePtr nullFeature = make_shared<Feature>(8);
    DesignElementPtr dep   = make_shared<DesignElement>(nullFeature,fp);
    FigureView * fv        = viewFigure(dep, QPointF(0.0,0.0),height);
    layer2->addToGroup(fv);

    QPointF pt(600.0,450.0);
    MarkX * mark = new MarkX(pt,QPen(Qt::blue,3)," ");
    layer2->addToGroup(mark);
#else
    // place the figure
    QString tileName  = "DAC new 4.v5";
    TilingManager* tm = TilingManager::getInstance();
    TilingPtr t       = tm->loadTiling(tileName);
    Q_ASSERT(t);

    PrototypePtr proto = make_shared<Prototype>(t);
    QList<PlacedFeaturePtr> qlfp = t->getPlacedFeatures();

    DesignElementPtr dep = make_shared<DesignElement>(qlfp[0]->getFeature(),fp);
    proto->addElement(dep);

    QPolygonF bounds;
    bounds << QPointF(-diameter, diameter);
    bounds << QPointF( diameter, diameter);
    bounds << QPointF( diameter,-diameter);
    bounds << QPointF(-diameter,-diameter);

    Thick * thick = new Thick(proto,make_shared<QPolygonF>(bounds));
    thick->setColor(QColor(0xa2,0x79,0x67));
    thick->setDrawOutline(true);

    layer2->addToGroup(thick);
#endif

#if 0
    MarkX  * mark;
    for (int i=0; i < r->points.size(); i++)
    {
        QPointF pt = r->points.at(i);
        pt *= radius;
        mark = new MarkX(pt,QPen(QColor(Qt::green),3),i);
        layers[4].addToGroup(mark);
    }
#endif
#if 0
    for (int i=0; i < m->getVertices().size(); i++)
    {
        QPointF pt = m->getVertices().at(i)->getPosition();
        pt *= radius;
        mark = new MarkX(pt,QPen(QColor(Qt::blue),3),i);
        layers[4].addToGroup(mark);
    }
#endif
#if 0

    for (int i=0; i < m->getEdges()->size(); i++)
    {
        Edge *  e = m->getEdges()->at(i);
        QPointF pt1 = e->getV1()->getPosition();
        pt1 *= radius;
        mark = new MarkX(pt1,QPen(QColor(Qt::white),3),i*2);
        layers[4].addToGroup(mark);
        QPointF pt2 = e->getV2()->getPosition();
        pt2 *= radius;
        mark = new MarkX(pt2,QPen(QColor(Qt::white),3),(i*2)+1);
        layers[4].addToGroup(mark);
    }
#endif
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
    ShapeFactory * s1 = new ShapeFactory(diameter);
    layers[1].addToGroup(s1);
#endif

    s2 = new ShapeFactory(diameter);
    Layer * layer2 = addLayer(2);
    layer2->addToGroup(s2);

    QPointF a(-radius,-ypos);
    QPointF b( radius,-ypos);
    s2->addLine(woodPen,a,b);

    QPointF c(-radius,ypos);
    QPointF d( radius,ypos);
    s2->addLine(woodPen,c,d);

    QPointF e(-radius,0);
    QPointF f( radius,0);
    s2->addLine(woodPen,e,f);

    s3 = new ShapeFactory(diameter);
    Layer * layer3 = addLayer(3);
    layer3->addToGroup(s3);

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


    s4 = new ShapeFactory(diameter);
    Layer * layer4 = addLayer(4);
    layer4->addToGroup(s4);

    Polygon2 * p1 = s4->addStretchedExternalHexagon(woodPen, nobrush,90.0);
    Q_UNUSED(p1);

    s5 = new ShapeFactory(diameter);
    Layer * layer5 = addLayer(5);
    layer5->addToGroup(s5);

    Polygon2 * p2 = s5->addStretchedExternalHexagon(woodPen, nobrush,90.0);
    p2->translate(radius,ypos);
    //Utils::identify(&layers[5],p2);

#if 0
    ShapeFactory * s6 = new ShapeFactory(diameter);
    layers[6].addToGroup(s1);
    MarkX * m = new MarkX(QPointF(0,0),QPen(QColor(Qt::green),5),"center");
    layers[6].addToGroup(m);
#endif
}

/////////////////////////////////////////////////////////////////////

PatternKumiko2::PatternKumiko2(qreal Diameter, QBrush Brush) : Pattern(Diameter, Brush)
{
   layer2 = addLayer(2);
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
    ShapeFactory * s1 = new ShapeFactory(diameter);
    layers[1].addToGroup(s1);
#endif

    s2 = new ShapeFactory(diameter);
    QPointF a(-radius,-ypos);
    QPointF b( radius,-ypos);
    s2->addLine(woodPen,a,b);

    QPointF c(-radius,ypos);
    QPointF d( radius,ypos);
    s2->addLine(woodPen,c,d);

    QPointF e(-radius,0);
    QPointF f( radius,0);
    s2->addLine(woodPen,e,f);

    s3 = new ShapeFactory(diameter);
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

    s4 = new ShapeFactory(diameter);
    Polygon2 * p1 = s4->addStretchedExternalHexagon(woodPen, nobrush,90.0);
    Q_UNUSED(p1);

    s5 = new ShapeFactory(diameter);
    Polygon2 * p2 = s5->addStretchedExternalHexagon(woodPen, nobrush,90.0);
    p2->translate(radius,ypos);
    //Utils::identify(&layers[5],p2);

#if 0
    ShapeFactory * s6 = new ShapeFactory(diameter);
    layers[6].addToGroup(s1);
    MarkX * m = new MarkX(QPointF(0,0),QPen(QColor(Qt::green),5),"center");
    layers[6].addToGroup(m);
#endif

    // Make a map of the shape factory design and create a style from it
    MapPtr map = make_shared<Map>();
    map->addShapeFactory(s2);
    map->addShapeFactory(s3);
    map->addShapeFactory(s4);
    map->addShapeFactory(s5);
    //map->dumpVertices(false);

    // make an explicit figure and position it
    QString tileName  = "Kumiko2";
    TilingManager* tm = TilingManager::getInstance();
    TilingPtr t       = tm->loadTiling(tileName);
    if (!t)
    {
        TileMaker * tm = new TileMaker();
        tm->beginTiling(tileName);
        tm->setFillData(fd);
        tm->setTranslations(trans1,trans2);
        tm->beginRegularFeature( 4 );
        tm->addPlacement( Transform( 1.0, 0.0, 0.0, 0.0, 1.0, 0.0 ) );
        tm->endFeature();
        tm->b_setDescription("Kumiko2 translation vectors");
        tm->b_setAuthor("David A. Casper");

        t = tm->EndTiling();

        workspace->setTiling(t);
        workspace->saveTiling(tileName,t);
    }

    PrototypePtr proto = make_shared<Prototype>(t);

    FigurePtr fp = make_shared<ExplicitFigure>(map,FIG_TYPE_EXPLICIT);
    QList<PlacedFeaturePtr> qlfp = t->getPlacedFeatures();

    DesignElementPtr dep = make_shared<DesignElement>(qlfp[0]->getFeature(),fp);
    proto->addElement(dep);

    QPolygonF bounds;
    bounds << QPointF(-diameter, diameter);
    bounds << QPointF( diameter, diameter);
    bounds << QPointF( diameter,-diameter);
    bounds << QPointF(-diameter,-diameter);

    Thick * thick = new Thick(proto,make_shared<QPolygonF>(bounds));
    thick->setColor(QColor(0xa2,0x79,0x67));
    thick->setDrawOutline(true);
    thick->createStyleRepresentation();

    layer2->addToGroup(thick);
}
