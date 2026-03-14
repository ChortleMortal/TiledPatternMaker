#include <QDebug>

#include "gui/viewers/geo_graphics.h"
#include "sys/geometry/dcel.h"
#include "model/styles/fill_original.h"
#include "model/styles/filled.h"

///////////////////////////////////////////////
///
///  Original Coloring
///
///////////////////////////////////////////////

OriginalColoring::OriginalColoring(Filled * filled) : ColorMaker(filled)
{
    draw_inside_blacks    = true;
    draw_outside_whites   = true;
    whiteColorSet.addColor(TPColor (QColor::fromRgb(0x34554a),false));
    blackColorSet.addColor(TPColor (QColor::fromRgb(0xa35807),false));
}

OriginalColoring::OriginalColoring(const OriginalColoring & other) : ColorMaker(other.filled)
{
    whiteColorSet = other.whiteColorSet;
    blackColorSet = other.blackColorSet;
}

void OriginalColoring::initFrom(eFillType old)
{
    if (initialised)
        return;

    switch (old)
    {
    case FILL_ORIGINAL:
        break;  // always ignore from self
    case FILL_TWO_FACE:
        whiteColorSet = filled->new1.whiteColorSet;
        blackColorSet = filled->new1.blackColorSet;
        draw_outside_whites = filled->new1.draw_outside_whites;
        draw_inside_blacks  = filled->new1.draw_inside_blacks;
        break;
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

void OriginalColoring::resetStyleRepresentation()
{
    facesToDo.clear();
    whiteFaces.clear();
    blackFaces.clear();
}

void OriginalColoring::createStyleRepresentation(DCELPtr dcel)
{
    createFacesToDo(dcel,facesToDo);
    assignColorsOriginal(facesToDo,whiteFaces,blackFaces);
}

void OriginalColoring::draw(GeoGraphics * gg)
{
    qDebug() << "OriginalColoring::draw  black" << ((draw_inside_blacks) ? "show" : "hide") << blackColorSet.info() << "white" << ((draw_outside_whites) ? "show" : "hide") << whiteColorSet.info();

    if (draw_outside_whites)
    {
        for (int i=0; i < whiteFaces.size(); i++)
        {
            FacePtr & face  = whiteFaces[i];
            QColor color = whiteColorSet.getTPColor(i).color;
            QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

            const EdgePoly & ep = *face.get();
            gg->fillEdgePoly(ep, pen);
        }
    }

    if (draw_inside_blacks)
    {
        for (int i=0; i < blackFaces.size(); i++)
        {
            FacePtr & face  = blackFaces[i];
            QColor color    = blackColorSet.getTPColor(i).color;
            QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

            const EdgePoly & ep = *face.get();
            gg->fillEdgePoly(ep, pen);
        }
    }
}

QString OriginalColoring::sizeInfo()
{
    QString astring = QString(" blacks=%1 whites=%2").arg(blackFaces.size()).arg(whiteFaces.size());
    return astring;
}

void OriginalColoring::assignColorsOriginal(FaceSet & facesToDo, FaceSet & whiteFaces, FaceSet & blackFaces)
{
    // recurse through faces to assign colours
    // this now deals with non-contiguous figures

    int facesToProcess;
    int facesLeft;
    do
    {
        facesToProcess = facesToDo.size();

        assignColorsToFaces(facesToDo); // Propagate colours using a DFS (Depth First Search).
        addFaceResults(facesToDo,whiteFaces,blackFaces);

        facesLeft = facesToDo.size();

    } while ((facesLeft != 0) &&  (facesLeft < facesToProcess));

    qDebug() << "done: white =" << whiteFaces.size() << "black =" << blackFaces.size();
}

void OriginalColoring::createFacesToDo(DCELPtr dcel, FaceSet & facesToDo)
{
    facesToDo.clear();

    FaceSet & faces = dcel->getFaceSet();
    for (auto & face : faces)
    {
        if (!face->outer)
        {
            face->state = FACE_UNDONE;
            facesToDo.push_back(face);
        }
    }
}
// Propagate colours using a DFS (Depth First Search) - used by Original algorithm
void OriginalColoring::assignColorsToFaces(FaceSet &fset)
{
    if (fset.size() == 0)
    {
        qWarning() << "no faces to assign";
        return;
    }

    QStack<FacePtr> st;
    FacePtr aface = fset.at(0);
    aface->state = FACE_PROCESSING;
    st.push(aface);

    eColor color = C_WHITE;     // seed

    while (!st.empty())
    {
        aface = st.pop();

        //eColor color = C_WHITE;     // seed
        EdgePtr head = aface->incident_edge.lock();
        if (head)
        {
            EdgePtr edge = head;
            do
            {
                auto twin = edge->twin.lock();
                FacePtr nfi;
                if (twin)
                {
                    nfi = twin->incident_face.lock();
                }
                if (nfi)
                {
                    if (!nfi->outer)
                    {
                        switch( nfi->state )
                        {
                        case FACE_UNDONE:
                            nfi->state = FACE_PROCESSING;
                            st.push(nfi);
                            break;

                        case FACE_PROCESSING:
                        case FACE_DONE:
                        case FACE_REMOVE:
                            break;

                        case FACE_BLACK:
                            color = C_BLACK;
                            break;

                        case FACE_WHITE:
                            color = C_WHITE;
                            break;
                        }
                    }
                }
                edge = edge->next.lock();
            } while (edge != head);
        }

        if (color == C_BLACK)
        {
            aface->state = FACE_WHITE;
        }
        else
        {
            Q_ASSERT(color == C_WHITE);
            aface->state = FACE_BLACK;
        }
    }
}

void OriginalColoring::addFaceResults(FaceSet & fset, FaceSet & whiteFaces, FaceSet & blackFaces)
{
    FaceSet faces2do;
    for (auto & face : std::as_const(fset))
    {
        if( face->state == FACE_WHITE )
        {
            whiteFaces.push_back(face);
        }
        else if(face->state == FACE_BLACK )
        {
            blackFaces.push_back(face);
        }
        else
        {
            face->state = FACE_UNDONE;
            faces2do.push_back(face);
        }
    }
    fset = faces2do;
}

