#include <QDebug>

#include "gui/viewers/geo_graphics.h"
#include "sys/geometry/loose.h"
#include "sys/geometry/dcel.h"
#include "sys/geometry/edge.h"
#include "model/styles/fill_color_maker.h"
#include "model/styles/filled.h"

///////////////////////////////////////////////
///
/// New2 Coloring
///
///////////////////////////////////////////////

New2Coloring::New2Coloring(Filled * filled) : ColorMaker(filled)
{}

void New2Coloring::initFrom(eFillType old)
{
    if (initialised)
        return;

    switch (old)
    {
    case FILL_ORIGINAL:
        colorSet  = filled->original.whiteColorSet;
        colorSet += filled->original.blackColorSet;
        break;
    case FILL_TWO_FACE:
        colorSet  = filled->new1.whiteColorSet;
        colorSet += filled->new1.blackColorSet;
        break;
    case FILL_MULTI_FACE:
        break;  // always ignore from self
    case FILL_MULTI_FACE_MULTI_COLORS:
        colorSet  = *filled->new3.colorGroup.getColorSet(0);
        colorSet += *filled->new3.colorGroup.getColorSet(1);
        break;
    case DEPRECATED_FILL_FACE_DIRECT:   // DEPRECATED
        break;
    case FILL_FACE_DIRECT:
        colorSet = filled->direct.palette;
        break;
    }

    initialised = true;
}

void New2Coloring::resetStyleRepresentation()
{
    faceGroup.clear();
}

void New2Coloring::createStyleRepresentation(DCELPtr dcel)
{
    // create a facegroup consisting of a face set for each face size
    buildFaceGroup(dcel,facesToDo,faceGroup);

    // create a color set with one color for each face set
    adjustColorGroupSizeIfNeeded();
}

void New2Coloring::draw(GeoGraphics *gg)
{
    qDebug() << "New2Coloring::draw";

    if (colorSet.size() != faceGroup.size())
    {
        qWarning() << "New2Coloring::draw - ERROR color set size =" << colorSet.size() << "face group size" << faceGroup.size();
        adjustColorGroupSizeIfNeeded();
    }

    // each face set has a single color in whiteColorSet
    for (int i=0; i < faceGroup.size(); i++)
    {
        FaceSetPtr & fset = faceGroup[i];
        TPColor tpcolor   = colorSet.getTPColor(i);

        if (fset->selected)
        {
            QPen pen(Qt::red, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            for (const FacePtr & face : std::as_const(*fset))
            {
                EdgeSet & ep = *face.get();
                gg->fillEdgePoly(ep,pen);
            }
        }
        else
        {
            if (!tpcolor.hidden)
            {
                QPen pen(tpcolor.color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                for (const FacePtr & face : std::as_const(*fset))
                {
                    EdgeSet & ep = *face.get();
                    gg->fillEdgePoly(ep,pen);
                }
            }
        }
    }
}

QString New2Coloring::sizeInfo()
{
    QString astring("TODO");
    return astring;
}

void New2Coloring::buildFaceGroup(DCELPtr dcel,FaceSet & facesToDo, FaceGroup & faceGroup)
{
    // create a facegroup consisting of a face set for each face size
    qDebug() << "ColorMaker::buildFaceGroup";

    faceGroup.clear();

    OriginalColoring::createFacesToDo(dcel,facesToDo);

    for (auto & face : std::as_const(facesToDo))
    {
        face->state = FACE_UNDONE;
    }

    // create face groups
    for (auto it = facesToDo.begin(); it != facesToDo.end(); it++)
    {
        FacePtr fp = *it;
        if (fp->state != FACE_UNDONE)
        {
            continue;
        }

        fp->state      = FACE_DONE;

        FaceSetPtr fsp = std::make_shared<FaceSet>();
        fsp->area      = fp->area;
        fsp->sides     = fp->getNumSides();
        fsp->push_back(fp);
        faceGroup.push_back(fsp);

        for (auto it2 = (it + 1); it2 != facesToDo.end(); it2++)
        {
            FacePtr fp2 = *it2;
            if (fp2->state == FACE_UNDONE)
            {
                if (Loose::equals(fsp->area,fp2->area) && (fsp->sides == fp2->getNumSides()))
                {
                    fp2->state = FACE_DONE;
                    fsp->push_back(fp2);
                }
            }
        }
    }

    std::sort(faceGroup.begin(), faceGroup.end(), FaceSet::sort);    // largest first
    qDebug() << "Num face sets s=" << faceGroup.size();
}

void New2Coloring::adjustColorGroupSizeIfNeeded()
{
    // create a color set with one color for each face set

    qDebug() << "ColorMaker::assoociateColorsWithFaceSets" << faceGroup.size() << faceGroup.totalSize();;

    // make the color set size the same as the face group size
    if (colorSet.size() < faceGroup.size())
    {
        // expand color set by adding (default) yellow
        qWarning() <<  "less colors than faces: faces=" << faceGroup.size() << "colors=" << colorSet.size();
        int diff = faceGroup.size() - colorSet.size();
        for (int i = 0; i < diff; i++)
        {
            TPColor tpc(Qt::yellow);
            tpc.hidden = true;
            colorSet.addColor(tpc);
        }
    }
    else if (colorSet.size() > faceGroup.size())
    {
        // shrink the size of the color set
        qWarning() <<  "more colors than faces: faces=" << faceGroup.size() << "colors=" << colorSet.size();
        colorSet.resize(faceGroup.size());
    }

    // so there is one color for each face set in the face group
    Q_ASSERT(colorSet.size() == faceGroup.size());
}

#if 1
void New2Coloring::removeOverlappingFaces()
{
    int start = facesToDo.size();
    qDebug() << "ColorMaker::removeOverlappingFaces START faces:" << start;

    for (auto & face : std::as_const(facesToDo))
    {
        face->state = FACE_UNDONE;
    }

    UniqueQVector<FacePtr> overlaps;

    for (auto it = facesToDo.begin(); it != facesToDo.end(); it++)
    {
        FacePtr f1 = *it;

        if (f1->state == FACE_UNDONE)
            f1->state = FACE_DONE;

        for (auto it2 = it+1; it2 != facesToDo.end(); it2++)
        {
            FacePtr f2 = *it2;

            if (f2->state != FACE_UNDONE)
                continue;

            //qDebug() << "overlap" << dcel->faceIndex(f1) << dcel->faceIndex(f2);

            if (f1->overlaps(f2))
            {
                Q_ASSERT(f1 != f2);
                // which one is the larger overlapper?
                if (f2->area >= f1->area)
                {
                    f2->state = FACE_REMOVE;
                    overlaps.push_back(f2);
                }
                else
                {
                    f1->state = FACE_REMOVE;
                    overlaps.push_back(f1);
                }
            }
        }
    }

    if (overlaps.size())
        qWarning()  << "removing" << overlaps.size() << "faces";

    for (auto & face : std::as_const(overlaps))
    {
        facesToDo.removeAll(face);
    }

    int end = facesToDo.size();
    qDebug() << "ColorMaker::removeOverlappingFaces - END faces:" << end;
    if (start != end)
    {
        qWarning() << "ColorMaker::removeOverlappingFaces start =" << start << "end =" << end;
    }
}
#endif

