////////////////////////////////////////////////////////////////////////////
//
// Figure.java
//
// Making the user interface operate directly on maps would be
// a hassle.  Maps are a very low level geometry and topological
// structure.  Not so good for interacting with users.  So I
// define a Figure class, which is a higher level structure --
// an object that knows how to build maps.  Subclasses of Feature
// understand different ways of bulding maps, but have the advantage
// of being parameterizable at a high level.

#include "figures/figure.h"
#include "tile/feature.h"
#include "geometry/map.h"
#include "geometry/edge.h"

using std::make_shared;

int Figure::refs = 0;

Figure::Figure()
{
    refs++;
    figType           = FIG_TYPE_UNDEFINED;
    extBoundarySides  = 1;  // defaults to a circle
    extBoundaryScale  = 1.0;
    figureScale       = 1.0;
    figureRotate      = 0.0;
    hasCircleBoundary = true;

    figureMap = make_shared<Map>("figureMap");
}

Figure::Figure(const Figure & other)
{
    refs++;

    figType           = FIG_TYPE_UNDEFINED;

    extBoundarySides  = other.extBoundarySides;
    extBoundaryScale  = other.extBoundaryScale;
    figureScale       = other.figureScale;
    figureRotate      = other.figureRotate;

    radialFigBoundary = other.radialFigBoundary;
    extBoundary       = other.extBoundary;
    hasCircleBoundary = other.hasCircleBoundary;

    figureMap         = other.figureMap;
}

Figure::~Figure()
{
    //qDebug() << "Figure destructor" << this;
    refs--;
}

bool Figure::equals(const  FigurePtr other)
{
    if (figType != other->figType)
        return false;
    if (extBoundarySides  != other->extBoundarySides)
        return false;
    if (extBoundaryScale  != other->extBoundaryScale)
        return false;
    if (figureScale       != other->figureScale)
        return false;
    if (figureRotate      != other->figureRotate)
        return false;

    if (radialFigBoundary != other->radialFigBoundary)
        return false;
    if (extBoundary       != other->extBoundary)
        return false;
    if (hasCircleBoundary != other->hasCircleBoundary)
        return false;
    return true;
}



bool Figure::isExplicit()
{
    switch (figType)
    {
    case FIG_TYPE_EXPLICIT:
    case FIG_TYPE_EXPLICIT_INFER:
    case FIG_TYPE_EXPLICIT_ROSETTE:
    case FIG_TYPE_EXPLICIT_HOURGLASS:
    case FIG_TYPE_EXPLICIT_INTERSECT:
    case FIG_TYPE_EXPLICIT_GIRIH:
    case FIG_TYPE_EXPLICIT_STAR:
    case FIG_TYPE_EXPLICIT_FEATURE:
        return true;
    default:
        return false;
    }
}

bool Figure::isRadial()
{
    switch (figType)
    {
    case FIG_TYPE_RADIAL:
    case FIG_TYPE_ROSETTE:
    case FIG_TYPE_STAR:
    case FIG_TYPE_CONNECT_STAR:
    case FIG_TYPE_CONNECT_ROSETTE:
    case FIG_TYPE_EXTENDED_ROSETTE:
    case FIG_TYPE_EXTENDED_STAR:
        return true;
    default:
        return false;
    }
}


int Figure::getN()
{
    return n;
}

void Figure::setExtBoundarySides(int sides)
{
    extBoundarySides = sides;
    hasCircleBoundary = (sides < 3) ? true : false;
}

void Figure::setExtBoundaryScale(qreal scale)
{
    extBoundaryScale = scale;
}

void Figure::buildExtBoundary()
{
    QTransform qTrans;
    qTrans.scale(extBoundaryScale,extBoundaryScale);
    if (extBoundarySides >= 3)
    {
        Feature f2(extBoundarySides,0);
        extBoundary = f2.getPoints();
        extBoundary = qTrans.map(extBoundary);
        //qDebug() << "Ext boundary:" << extBoundary;
        hasCircleBoundary = false;
#if 0
        if (extBoundarySides == 4)
        {
            // test code
            QTransform t;
            t.rotate(30);

            QTransform t2;
            t2.rotate(90 + 30);

            extBoundary[0] = t.map(extBoundary[0]);
            extBoundary[1] = t2.map(extBoundary[1]);
            extBoundary[2] = t.map(extBoundary[2]);
            extBoundary[3] = t2.map(extBoundary[3]);
#if 0
            QLineF l0(extBoundary[0],extBoundary[1]);
            QLineF l1(extBoundary[1],extBoundary[2]);
            QLineF l2(extBoundary[2],extBoundary[3]);
            QLineF l3(extBoundary[3],extBoundary[0]);

            l0 = t.map(l0);
            l2 = t.map(l2);
            QTransform t2;
            t2.rotate(-30);
            l1 = t2.map(l1);
            l3 = t2.map(l3);
            extBoundary[0] = l0.p1();
            extBoundary[1] = l1.p1();
            extBoundary[2] = l2.p1();
            extBoundary[3] = l3.p1();
#endif
        }
#endif
    }
    else
    {
        //qDebug() << "circular boundary";
        hasCircleBoundary = true;
    }
}
#if 0
Rhombus::Rhombus(Vertex point, int radius) : Shape(point)
{
    if((radius>centroid.getX()/2) || (radius>centroid.getY()/2)) // Inteded to be a y?
    {
        cout << "Object must fit on screen." << endl;
        system("pause");
        exit(0);
    }

    // create vertices for a rhombus with horizontal and vertical diagonals
    vertices.push_back(Vertex(point.getX() - radius, point.getY()));
    vertices.push_back(Vertex(point.getX(), point.getY() - radius));
    vertices.push_back(Vertex(point.getX() + radius, point.getY()));
    vertices.push_back(Vertex(point.getX(), point.getY() + radius));
}

QPainterPath path;
QRectF rect(0, 0 , 100, 100);
path.moveTo(rect.center().x(), rect.top());
path.lineTo(rect.right(), rect.center().y());
path.lineTo(rect.center().x(), rect.bottom());
path.lineTo(rect.left(), rect.center().y());
path.lineTo(rect.center().x(), rect.top());
QGraphicsPathItem* itemR = ui->graphicsView->scene()->addPath(path);
itemR->setFlag(QGraphicsItem::ItemIsMovable);
#endif
// uses existing tmpIndices
void Figure::annotateEdges()
{
    if (!figureMap)
    {
        return;
    }

    int i=0;
    for (auto edge : figureMap->getEdges())
    {
        QPointF p = edge->getMidPoint();
        debugMap->insertDebugMark(p, QString::number(i++));
    }
    //debugMap->dumpMap(false);
}
