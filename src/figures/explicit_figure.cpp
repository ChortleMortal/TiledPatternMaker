#include <QDebug>
#include "figures/explicit_figure.h"


typedef std::shared_ptr<class ExplicitFigure>   ExplicitPtr;

ExplicitFigure::ExplicitFigure(MapPtr map, eFigType figType, int sides)
    : Figure()
{
    figureMap = map;
    setFigType(figType);
    init(sides);
}

ExplicitFigure::ExplicitFigure(const Figure &fig, MapPtr map, eFigType figType, int sides)
    : Figure(fig)
{
    figureMap = map;
    setFigType(figType);
    init(sides);
}

ExplicitFigure::ExplicitFigure(const Figure & fig, eFigType figType, int sides)
    : Figure(fig)
{
    setFigType(figType);
    init(sides);
}

ExplicitFigure::~ExplicitFigure()
{}

void ExplicitFigure::init(int sides)
{
    this->n = sides;  // was sides = 10; // girih + intersect + rosette + star
    skip  = 3.0;         // girih
    d     = 2.0;         // hourglass + intersect + star
    s     = 1;           // hourglass + intersect + star
    q     = 0.0;         // rosette
    r_flexPt    = 0.5;   // rosette
    progressive = false; // intersect    
}


bool ExplicitFigure::equals(const FigurePtr other)
{
    ExplicitPtr otherp = std::dynamic_pointer_cast<ExplicitFigure>(other);
    if (!otherp)
        return  false;

    if (getFigType() != other->getFigType())
        return false;

    if (n != otherp->n)
        return false;

    if (d != otherp->d)
        return  false;

    if (s != otherp->s)
        return false;

    if (q != otherp->q)
        return  false;

    if (r_flexPt != otherp->r_flexPt)
        return  false;

    if (progressive != otherp->progressive)
        return  false;

    if (!Figure::equals(other))
        return false;

     return true;
}

MapPtr ExplicitFigure::getFigureMap()
{
    return figureMap;
}

void ExplicitFigure::buildMaps()
{
    //qWarning() << "ExplicitFigure::buildMaps - does nothing";
}

void ExplicitFigure::resetMaps()
{
    //qWarning() << "ExplicitFigure::resetMaps - does nothing";
}
