#ifndef DESIGN_ELEMENT_H
#define DESIGN_ELEMENT_H

#include <QString>
#include <QTransform>

typedef std::shared_ptr<class Figure>          FigurePtr;
typedef std::shared_ptr<class Feature>         FeaturePtr;
typedef std::shared_ptr<class DesignElement>   DesignElementPtr;

////////////////////////////////////////////////////////////////////////////
//
// DesignElement.java
//
// A DesignElement is the core of the process of building a finished design.
// It's a Feature together with a Figure.  The Feature comes from the
// tile library and will be used to determine where to place copies of the
// Figure, which is designed by the user.

class DesignElement
{
public:

    DesignElement(const FeaturePtr & feat, const FigurePtr & fig );
    DesignElement(const FeaturePtr & feat );
    DesignElement(const DesignElementPtr & dep);
    DesignElement();
    DesignElement(const DesignElement & other);
    ~DesignElement();

    FeaturePtr  getFeature() const;
    void        replaceFeature(const FeaturePtr & afeature);
    FigurePtr   getFigure() const;
    void        setFigure(const FigurePtr & fig);

    bool        validFigure();
    void        createFigure();

    QString     toString();
    void        describe();

    static int refs;

protected:
    FeaturePtr	feature;
    FigurePtr	figure;
};

// added by DAC - not in taprats
class PlacedDesignElement : public DesignElement
{
public:
    PlacedDesignElement();
    PlacedDesignElement(const DesignElementPtr & del, QTransform T);
    PlacedDesignElement(const FeaturePtr & featp, const FigurePtr & figp, QTransform T);
    PlacedDesignElement(const PlacedDesignElement & other);

    ~PlacedDesignElement();

    PlacedDesignElement & operator=(const PlacedDesignElement & other);

    QTransform   getTransform() const { return trans; }

    QString toString();

    static int refs2;

protected:
    QTransform  trans;
};
#endif

