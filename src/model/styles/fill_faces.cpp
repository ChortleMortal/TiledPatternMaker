#include <QDebug>

#include "gui/viewers/geo_graphics.h"
#include "sys/geometry/dcel.h"
#include "model/styles/fill_color_maker.h"
#include "model/styles/filled.h"

///////////////////////////////////////////////
///
/// DirectColoring
///
///////////////////////////////////////////////

DirectColoring::DirectColoring(Filled * filled) : ColorMaker(filled)
{}

void DirectColoring::initFrom(eFillType old)
{
    if (initialised)
        return;

    switch (old)
    {
    case FILL_ORIGINAL:
        palette  = filled->original.whiteColorSet;
        palette +=  filled->original.blackColorSet;
        break;
    case FILL_TWO_FACE:
        palette  = filled->new1.whiteColorSet;
        palette +=  filled->new1.blackColorSet;
        break;
    case FILL_MULTI_FACE:
    {
        palette.clear();
        UniqueQVector<TPColor> colors;
        for (auto & tpc : filled->new2.colorSet)
        {
            colors.push_back(tpc);
        }
        palette.setColors(colors);
    }   break;
    case FILL_MULTI_FACE_MULTI_COLORS:
    {
        palette.clear();
        UniqueQVector<TPColor> colors;
        for (auto & cset : filled->new3.colorGroup)
        {
            for (auto & tpc : cset)
            {
                colors.push_back(tpc);
            }
        }
        palette.setColors(colors);
    }   break;
    case DEPRECATED_FILL_FACE_DIRECT:   // DEPRECATED
        break;
    case FILL_FACE_DIRECT:
        break;  // always ignore from self
    }

    initialised = true;
}

void DirectColoring::resetStyleRepresentation()
{
    faceMap.clear();
}

void DirectColoring::createStyleRepresentation(DCELPtr dcel)
{
    faceMap.clear();

    const FaceSet & faceSet = dcel->getFaceSet();

    // uses FaceColorList - which could be empty
    for (auto & faceColor : faceColorList)
    {
        QPointF center = faceColor.first;
        FacePtr face   = faceSet.findByCenter(center);
        if (face)
        {
            face->iPalette = faceColor.second;
            faceMap.insert(PointKey{face->center()},face);
        }
    }
}

void DirectColoring::createFromDeprecated(DCELPtr dcel)
{
    faceMap.clear();

    const FaceSet & faces = dcel->getFaceSet();

    // uses FaceColorList - which could be empty
    for (auto & face : faces)
    {
        if (face->iPalette >= 0)
        {
            faceMap.insert(PointKey{face->center()},face);
        }
    }
}

void DirectColoring::draw(GeoGraphics *gg)
{
    qDebug() << "Direct2Coloring::draw";

    for (auto it = faceMap.begin(); it != faceMap.end(); it++)
    {
        FacePtr face = it.value();
        int index = face->iPalette;
        if (index >=0)
        {
            QColor color = palette.getQColor(index);
            QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            const EdgePoly & ep = *face.get();
            gg->fillEdgePoly(ep, pen);
        }
    }

#if 0
    // debug code
    DCELPtr dcel = filled->getPrototype()->getDCEL();
    if (!dcel) return;

    const FaceSet & faces = dcel->getFaceSet();
    for (const FacePtr & face : faces)
    {
        if (face->outer)
        {
            QPen pen(Qt::blue, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            const EdgePoly & ep = *face.get();
            gg->drawEdgePoly(ep, pen);
        }
    }
#endif
}

QString DirectColoring::sizeInfo()
{
    QString astring = QString("faceMap size=%1").arg(faceMap.size());
    return astring;
}





