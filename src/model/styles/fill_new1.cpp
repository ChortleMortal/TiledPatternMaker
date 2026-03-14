#include <QDebug>

#include "gui/viewers/geo_graphics.h"
#include "sys/geometry/dcel.h"
#include "model/styles/fill_color_maker.h"
#include "model/styles/filled.h"

///////////////////////////////////////////////
///
///  New1 Coloring
///
///////////////////////////////////////////////

New1Coloring::New1Coloring(Filled * filled) : ColorMaker(filled)
{
    draw_inside_blacks    = true;
    draw_outside_whites   = true;
}

New1Coloring::New1Coloring(const New1Coloring & other) : ColorMaker(other.filled)
{
    draw_inside_blacks    = other.draw_inside_blacks;
    draw_outside_whites   = other.draw_outside_whites;
}

void New1Coloring::initFrom(eFillType old)
{
    if (initialised)
        return;

    switch (old)
    {
    case FILL_ORIGINAL:
        whiteColorSet = filled->original.whiteColorSet;
        blackColorSet = filled->original.blackColorSet;
        draw_inside_blacks  = filled->original.draw_inside_blacks;
        draw_outside_whites = filled->original.draw_outside_whites;
        break;
    case FILL_TWO_FACE:
        break;  // always ignore from self
    case FILL_MULTI_FACE:
        whiteColorSet.addColor(filled->new2.colorSet.getTPColor(0));
        blackColorSet.addColor(filled->new2.colorSet.getTPColor(1));
        break;
    case FILL_MULTI_FACE_MULTI_COLORS:
        whiteColorSet = *filled->new3.colorGroup.getColorSet(0);
        blackColorSet = *filled->new3.colorGroup.getColorSet(1);
        break;
    case DEPRECATED_FILL_FACE_DIRECT:   // DEPRECATED
        break;
    case FILL_FACE_DIRECT:
        whiteColorSet.addColor(filled->direct.palette.getTPColor(0));
        blackColorSet.addColor(filled->direct.palette.getTPColor(1));
        break;
    }

    initialised = true;
}

void New1Coloring::resetStyleRepresentation()
{
    facesToDo.clear();
    whiteFaces.clear();
    blackFaces.clear();
}

void New1Coloring::createStyleRepresentation(DCELPtr dcel)
{
    OriginalColoring::createFacesToDo(dcel,facesToDo);
#if 1
    assignColorsNew1();
#else
    OriginalColoring::assignColorsOriginal(facesToDo,whiteFaces,blackFaces);
#endif
}

// DAC Notes
// An edge can only be in two faces
// A vertex can be in multiple faces
void New1Coloring::assignColorsNew1()
{
    eFaceState newState = FACE_WHITE;     // seed
    for (auto & fp : std::as_const(facesToDo))
    {
        if (fp->state != FACE_UNDONE)
        {
            continue;
        }
        fp->state = newState;
        qreal size = fp->area;
        for (auto & fp2 : std::as_const(facesToDo))
        {
            if (fp2->state == FACE_UNDONE)
            {
                if (Loose::equals(size,fp2->area))
                {
                    fp2->state = newState;
                }
            }
        }
        newState = (newState == FACE_WHITE) ? FACE_BLACK : FACE_WHITE;
    }

    for (const auto & fi : std::as_const(facesToDo))
    {
        if( fi->state == FACE_WHITE )
        {
            whiteFaces.push_back(fi);
        }
        else if( fi->state == FACE_BLACK )
        {
            blackFaces.push_back(fi);
        }
        else
        {
            Q_ASSERT(false);
        }
    }
}

void New1Coloring::draw(GeoGraphics * gg)
{
    qDebug() << "New1Coloring::draw  black" << ((draw_inside_blacks) ? "show" : "hide") << blackColorSet.info() << "white" << ((draw_outside_whites) ? "show" : "hide") << whiteColorSet.info();

    if (draw_outside_whites)
    {
        for (int i=0; i < whiteFaces.size(); i++)
        {
            FacePtr & face = whiteFaces[i];
            QColor color   = whiteColorSet.getTPColor(i).color;
            QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

            const EdgePoly & ep = *face.get();
            gg->fillEdgePoly(ep, pen);
        }
    }

    if (draw_inside_blacks)
    {
        for (int i=0; i < blackFaces.size(); i++)
        {
            FacePtr & face = blackFaces[i];
            QColor color   = blackColorSet.getTPColor(i).color;
            QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

            const EdgePoly & ep = *face.get();
            gg->fillEdgePoly(ep, pen);
        }
    }
}

QString New1Coloring::sizeInfo()
{
    QString astring("TODO");
    return astring;
}

