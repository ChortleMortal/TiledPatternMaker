#include <QDebug>

#include "mosaic/design_element.h"
#include "figures/explicit_figure.h"
#include "figures/figure.h"
#include "figures/rosette.h"
#include "geometry/map.h"
#include "geometry/transform.h"
#include "misc/utilities.h"
#include "tile/feature.h"

using std::make_shared;

int DesignElement::refs = 0;
int PlacedDesignElement::refs2 = 0;

////////////////////////////////////////////////////////////////////////////
//
// DesignElement.java
//
// A DesignElement is the core of the process of building a finished design.
// It's a Feature together with a Figure.  The Feature comes from the
// tile library and will be used to determine where to place copies of the
// Figure, which is designed by the user.

DesignElement::DesignElement(const FeaturePtr & feat, const FigurePtr & fig)
{
    feature = feat;
    figure  = fig;
    refs++;
}

DesignElement::DesignElement(const FeaturePtr & feat)
{
    feature = feat;
    createFigure();
    refs++;
}

DesignElement::DesignElement(const DesignElementPtr & dep)
{
    feature = dep->feature;
    figure  = dep->figure;
    refs++;
}

DesignElement::DesignElement(const DesignElement & other)
{
    feature = other.feature;
    figure  = other.figure;
    refs++;
}


DesignElement::DesignElement()
{
    refs++;
}

DesignElement::~DesignElement()
{
    refs--;
}

FeaturePtr DesignElement::getFeature() const
{
    return feature;
}

FigurePtr DesignElement::getFigure() const
{
    return figure;
}

void DesignElement::createFigure()
{
    if (feature->isRegular())
    {
        figure = make_shared<Rosette>(feature->numPoints(), 0.0, 3, 0);
        figure->setFigureRotate(feature->getRotation());
        figure->setFigureScale(feature->getScale());
    }
    else
    {
        figure = make_shared<ExplicitFigure>(make_shared<Map>("FIG_TYPE_EXPLICIT_FEATURE map"),FIG_TYPE_EXPLICIT_FEATURE, feature->numPoints());
    }
    figure->setExtBoundarySides(feature->numPoints());
    figure->setExtBoundaryScale(feature->getScale());
}

void DesignElement::setFigure(const FigurePtr & fig)
{
    //qDebug() << "oldfig =" << figure.get() << "newfig =" << fig.get();
    figure = fig;
}

void DesignElement::replaceFeature(const FeaturePtr & feat)
{
    //qDebug() << "oldfeat=" << feature.get() << "newfeat=" << feat.get();
    if (feat->isSimilar(feature))
    {
        feature = feat;
    }
    else
    {
        feature = feat;
        createFigure();
    }
}
bool DesignElement::validFigure()
{
   if (figure->getFigType() == FIG_TYPE_EXPLICIT_FEATURE)
   {
       return true;     // always valid
   }
   if (feature->isRegular())
   {
       if (figure->isRadial())
            return true;
        else
           return false;
   }
   else
   {
        if (figure->isExplicit())
        {
            return true;
        }
        else
        {
            return false;
        }
   }
}

QString DesignElement::toString()
{
    return QString("this=%1 feature=%2 figure=%3").arg(Utils::addr(this)).arg(Utils::addr(feature.get())).arg(Utils::addr(figure.get()));
}

void DesignElement::describe()
{
    FigurePtr  fig  = getFigure();
    FeaturePtr feat = getFeature();
    if (fig)
        qDebug().noquote()  << "Figure:" << fig.get()  << fig->getFigureDesc() << fig->getFigureMap()->summary();
    else
        qDebug().noquote()  << "Figure: 0";
    if (feat)
        qDebug().noquote()  << "Feature:" << feat.get()  << "sides:" << feat->numSides() << ((feat->isRegular()) ? "regular" : "irregular");
    else
        qDebug().noquote()  << "Feature: 0";
}

/////////////////////////////////////////////////////////
///
///
/// /////////////////////////////////////////////////////

PlacedDesignElement::PlacedDesignElement()
{
    refs2++;
}

PlacedDesignElement::PlacedDesignElement(const DesignElementPtr & del, QTransform T)
{
    feature = del->getFeature();
    figure  = del->getFigure();
    trans   = T;
    refs2++;
}

PlacedDesignElement::PlacedDesignElement(const FeaturePtr & featp, const FigurePtr & figp, QTransform T)
{
    feature = featp;
    figure  = figp;
    trans   = T;
    refs2++;
}

PlacedDesignElement::PlacedDesignElement(const PlacedDesignElement & other) : DesignElement(other)
{
    trans   = other.trans;
    refs2++;
}

PlacedDesignElement::~PlacedDesignElement()
{
    refs2--;
}

PlacedDesignElement& PlacedDesignElement::operator=(const PlacedDesignElement& other)
{
    feature = other.feature;
    figure  = other.figure;
    trans   = other.trans;
    return *this;
}

QString PlacedDesignElement::toString()
{
    return QString("feature=%1 figure=%2 T=%3").arg(Utils::addr(feature.get())).arg(Utils::addr(figure.get())).arg(Transform::toInfoString(trans));
}
