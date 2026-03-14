#include <QDebug>

#include "gui/viewers/geo_graphics.h"
#include "sys/geometry/dcel.h"
#include "model/styles/fill_color_maker.h"
#include "model/styles/filled.h"

///////////////////////////////////////////////
///
///  New3Coloring
///
///////////////////////////////////////////////

New3Coloring::New3Coloring(Filled * filled) : ColorMaker(filled)
{}

void New3Coloring::initFrom(eFillType old)
{
    if (initialised)
        return;

    switch (old)
    {
    case FILL_ORIGINAL:
    {
        for (auto & tpc : filled->original.whiteColorSet)
        {
            ColorSet cs;
            cs.addColor(tpc);
            colorGroup.addColorSet(cs);
        }
        for (auto & tpc : filled->original.blackColorSet)
        {
            ColorSet cs;
            cs.addColor(tpc);
            colorGroup.addColorSet(cs);
        }
    }   break;
    case FILL_TWO_FACE:
    {
        for (auto & tpc : filled->new1.whiteColorSet)
        {
            ColorSet cs;
            cs.addColor(tpc);
            colorGroup.addColorSet(cs);
        }
        for (auto & tpc : filled->new1.blackColorSet)
        {
            ColorSet cs;
            cs.addColor(tpc);
            colorGroup.addColorSet(cs);
        }
    }   break;
    case FILL_MULTI_FACE:
        // from face sets to color groups
        for (auto & tpc : filled->new2.colorSet)
        {
            ColorSet cs;
            cs.addColor(tpc);
            colorGroup.addColorSet(cs);
        }
        break;
    case FILL_MULTI_FACE_MULTI_COLORS:
        break;  // always ignore from self

    case DEPRECATED_FILL_FACE_DIRECT:   // DEPRECATED
        break;

    case FILL_FACE_DIRECT:
        for (TPColor & tpc : filled->direct.palette)
        {
            ColorSet cs;
            cs.addColor(tpc);
            colorGroup.addColorSet(cs);
        }
        break;

    }

    initialised = true;
}

void New3Coloring::resetStyleRepresentation()
{
    faceGroup.clear();
}

void New3Coloring::createStyleRepresentation(DCELPtr dcel)
{
    New2Coloring::buildFaceGroup(dcel,facesToDo,faceGroup);
    adjustColorGroupSizeIfNeeded();
}

void New3Coloring::draw(GeoGraphics *gg)
{
    qDebug() << "New3Coloring::draw";
    Q_ASSERT(faceGroup.size() == colorGroup.size());

    // each face set has a set of colors
    for (int i=0; i < faceGroup.size(); i++)
    {
        FaceSetPtr & fset = faceGroup[i];
        if (fset->selected)
        {
            QPen pen(Qt::red, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            for (FacePtr & face : *fset)
            {
                EdgeSet & ep = *face.get();
                gg->fillEdgePoly(ep,pen);
            }
            continue;
        }

        ColorSet &  cset = colorGroup[i];

        if (cset.isHidden())
        {
            continue;
        }

        for (int j=0; j < fset->size(); j++)
        {
            FacePtr face = fset->at(j);
            TPColor tpc  = cset.getTPColor(j);
            if (tpc.hidden)
            {

                continue;
            }
            QPen pen(tpc.color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            EdgeSet & ep = *face.get();
            gg->fillEdgePoly(ep,pen);
        }
    }
}

QString New3Coloring::sizeInfo()
{
    QString astring("TODO");
    return astring;
}

void New3Coloring::adjustColorGroupSizeIfNeeded()
{
    qDebug() << "ColorMaker::assignColorGroups" << faceGroup.size() << faceGroup.totalSize();

    // the color set size must be the same as the face group size
    if (colorGroup.size() < faceGroup.size())
    {
        qInfo() <<  "less color groups than face groups: facegroup=" << faceGroup.size() << "colorgroup=" << colorGroup.size();
        int diff = faceGroup.size() - colorGroup.size();
        for (int i = 0; i < diff; i++)
        {
            ColorSet cset;
            TPColor tpc(Qt::yellow);
            cset.addColor(tpc);
            cset.hide(true);
            colorGroup.addColorSet(cset);
        }
    }
    else if (colorGroup.size() > faceGroup.size())
    {
        qInfo() <<  "more  color groups than face  groups: facegroup" << faceGroup.size() << "colorgroup=" << colorGroup.size();
        colorGroup.resize(faceGroup.size());
    }
}




