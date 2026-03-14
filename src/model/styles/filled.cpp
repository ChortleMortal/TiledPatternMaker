#include <QPainter>
#include <QDebug>
#include "model/styles/filled.h"
#include "sys/geometry/dcel.h"
#include "gui/viewers/geo_graphics.h"
#include "model/prototypes/prototype.h"

using std::shared_ptr;

////////////////////////////////////////////////////////////////////////////
//
// Filled.java
//
// A rendering style that converts the map to a collection of
// polygonal faces.  The faces are divided into two groups according to
// a two-colouring of the map (which is always possible for the
// kinds of Islamic designs we're building).
//
// The code to build the faces from the map is contained in
// geometry.Faces.

Filled::Filled(const ProtoPtr & proto, eFillType algorithm ) : Style(proto),original(this),new1(this),new2(this),new3(this),direct(this),deprecated(this)
{
    setAlgorithm(algorithm);
    initAlgorithmFrom(algorithm);   // sets initialised
}

Filled::Filled(const StylePtr & other) : Style(other),original(this),new1(this),new2(this),new3(this),direct(this),deprecated(this)
{
    FilledPtr filled = std::dynamic_pointer_cast<Filled>(other);
    if (filled)
    {
        setAlgorithm(filled->getAlgorithm());
        initAlgorithmFrom(filled->getAlgorithm());   // sets initialised
    }
    else
    {
        setAlgorithm(FILL_ORIGINAL);
        initAlgorithmFrom(FILL_ORIGINAL);   // sets initialised
    }
}

Filled::~Filled()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() >> "Filled destructor";
#endif
}

 void Filled::setAlgorithm(eFillType algorithm)
{
     _algorithm = algorithm;

    switch (algorithm)
    {
    case FILL_ORIGINAL:
        currentColoring = &original;
        break;
    case FILL_TWO_FACE:
        currentColoring = &new1;
        break;
    case FILL_MULTI_FACE:
        currentColoring = &new2;
        break;
    case FILL_MULTI_FACE_MULTI_COLORS:
        currentColoring = &new3;
        break;
    case DEPRECATED_FILL_FACE_DIRECT:   // DEPRECATED
        currentColoring = &deprecated;
        break;
    case FILL_FACE_DIRECT:
        currentColoring = &direct;
        break;
    }
}

void Filled::initAlgorithmFrom(eFillType old)
{
    currentColoring->initFrom(old);
}

// Style overrides.

void Filled::resetStyleRepresentation()
{
    currentColoring->resetStyleRepresentation();
    styled = false;
}

void Filled::createStyleRepresentation()
{
    if (!styled)
    {
        DCELPtr dcel = prototype->getDCEL();
        if (dcel)
        {
            currentColoring->createStyleRepresentation(dcel);
        }
        styled = true;
    }
}

void Filled::draw(GeoGraphics * gg)
{
    if (!isVisible() || !getPrototype()->getDCEL())
    {
        return;
    }

    currentColoring->draw(gg);
}
