#include <QDebug>

#include "gui/viewers/geo_graphics.h"
#include "sys/geometry/dcel.h"
#include "model/styles/fill_color_maker.h"
#include "model/styles/filled.h"

#define CONVERT_DEPRECATED_FILL

///////////////////////////////////////////////
///
/// DeprecatedDirectColoring
///
///////////////////////////////////////////////

DeprecatedDirectColoring::DeprecatedDirectColoring(Filled * filled) : ColorMaker(filled)
{
    converted = false;
}

void DeprecatedDirectColoring::resetStyleRepresentation()
{}

void DeprecatedDirectColoring::createStyleRepresentation(DCELPtr dcel)
{
    if (!converted)
    {
        initDCELFaces(dcel);
        assignPaletteColors(dcel);
#ifdef CONVERT_DEPRECATED_FILL
        filled->direct.palette = palette;
        filled->direct.createFromDeprecated(dcel);
        filled->setAlgorithm(FILL_FACE_DIRECT);
        //filled->createStyleRepresentation();
        filled->initAlgorithmFrom(FILL_FACE_DIRECT);
#endif
        converted = true;
    }
}

void DeprecatedDirectColoring::draw(GeoGraphics *gg)
{
#ifdef CONVERT_DEPRECATED_FILL

    Q_UNUSED(gg);
    qWarning() << "Trying to paint Deprecated Direct Coloring";

#else
    qDebug() << " DeprecatedDirectColoring::draw";

    DCELPtr dcel = filled->getPrototype()->getDCEL();
    if (!dcel) return;

    const FaceSet & faces = dcel->getFaceSet();
    if (faces.size() != numColorIndices())
    {
        qWarning() << "Filled faces=" << faces.size() << "indices" << numColorIndices();
    }

    for (const FacePtr & face : faces)
    {
        int index = face->iPalette;
        if (index >=0)
        {
            QColor color = getColorFromPalette(index);
            QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            const EdgePoly & ep = *face.get();
            gg->fillEdgePoly(ep, pen);
        }
    }

#if 0
    // debug code
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
#endif
}

void DeprecatedDirectColoring::initFrom(eFillType old)
{
    if (initialised)
        return;

    switch (old)
    {
    case FILL_ORIGINAL:
    case FILL_TWO_FACE:
    case FILL_MULTI_FACE:
    case FILL_MULTI_FACE_MULTI_COLORS:
        break;
    case DEPRECATED_FILL_FACE_DIRECT:   // DEPRECATED
        break;  // always ignore from self
    case FILL_FACE_DIRECT:
        break;
    }

    initialised = true;
}

QString DeprecatedDirectColoring::sizeInfo()
{
    QString astring("TODO");
    return astring;
}

// deprecated code - used by legacy mosaics
void DeprecatedDirectColoring::initDCELFaces(DCELPtr dcel)
{
    // this is bogus deprecated code which incorrectly assumes there
    // is only one outer. However included for leegacy designs.
    FacePtr outer;
    FaceSet & faces = dcel->getFaceSet();
    for (auto & face : faces)
    {
        if (face->outer)
        {
            outer = face;
            break;
        }
    }

    // remove the first outer from the DCEL
    faces.removeOne(outer);
}

// deprecated code - used by legacy mosaics
void DeprecatedDirectColoring::assignPaletteColors(DCELPtr dcel)
{
    FaceSet & faces = dcel->getFaceSet();
    int size        = faces.size();

    QVector<int> & indices = getPaletteIndices();
    int isize = indices.size();
    if (isize == 0)
    {
        // first time use - create the indices
        indices.resize(size);
        indices.fill(-1);
    }
    else if (isize != size)
    {
        // the number of faces has changed
        qWarning() << "resizing face color indices from" << isize << "to" << size;
        int num = size - isize;
        if (num > 0)
        {
            for (int i=0; i < num; i++)
            {
                indices.push_back(-1);
            }
        }
        else if (num < 0)
        {
            for (int i=0; i < qAbs(num) ; i++)
            {
                indices.removeLast();
            }
        }
    }

#if 1
    for (int i=0; i < size; i++)
    {
        // assign palette color to face
        FacePtr & face   = faces[i];
        face->iPalette = indices[i];
    }
#else
    // convert deprecated mosaics
    for (int i=0; i < size; i++)
    {
        int paletteIndex = indices[i];
        if (paletteIndex != -1)
        {
            FacePtr face   = faces[i];
            face->iPalette = paletteIndex;
            faceMap.insert(PointKey{face->center()},face);
        }
    }
#endif
}

void DeprecatedDirectColoring::removePaletteIndex(int index)
{
    // this needs to remove the index from the pacemap, and anything with a
    // higher index needs to be decremented

    QVector<int> & indices = getPaletteIndices();
    for (int i=0; i < indices.size(); i++)
    {
        int existing = indices[i];
        if (existing == index)
        {
            indices[i] = -1;
        }
        else if (existing > index)
        {
            indices[i] = (existing -1);
        }
    }
    setPaletteIndices(indices);

    DCELPtr dcel = filled->getPrototype()->getDCEL();
    assignPaletteColors(dcel);

}

int  DeprecatedDirectColoring::getColorIndex(int faceIndex)
{
    if (faceIndex < faceColorIndices.size())
    {
        return faceColorIndices[faceIndex];
    }
    else
    {
        return -1;
    }
}

void DeprecatedDirectColoring::setColorIndex(int faceIndex,int colorIndex)
{
    if (faceIndex < faceColorIndices.size())
    {
        faceColorIndices[faceIndex] = colorIndex;
    }
}




